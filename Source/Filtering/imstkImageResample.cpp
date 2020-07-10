/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	  http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkImageResample.h"
#include "imstkDataArray.h"
#include "imstkImageData.h"
#include "imstkLogger.h"
#include "imstkParallelFor.h"

namespace imstk
{
///
/// \brief Accepts structured coordinates (ie: pre int cast, [0, dim)) so it can do interpolation
///
template<typename T>
static T
trilinearSample(const Vec3d& structuredPt, T* imgPtr, const Vec3i& dim, const int numComps, const int comp)
{
	// minima of voxel, clamped to bounds
	const Vec3i s1 = structuredPt.cast<int>().cwiseMax(0).cwiseMin(dim - Vec3i(1, 1, 1));
	// maxima of voxel, clamped to bounds
	const Vec3i s2 = (structuredPt.cast<int>() + Vec3i(1, 1, 1)).cwiseMax(0).cwiseMin(dim - Vec3i(1, 1, 1));

	const size_t index000 = ImageData::getScalarIndex(s1.x(), s1.y(), s1.z(), dim, numComps) + comp;
	const size_t index100 = ImageData::getScalarIndex(s2.x(), s1.y(), s1.z(), dim, numComps) + comp;
	const size_t index110 = ImageData::getScalarIndex(s2.x(), s2.y(), s1.z(), dim, numComps) + comp;
	const size_t index010 = ImageData::getScalarIndex(s1.x(), s2.y(), s1.z(), dim, numComps) + comp;
	const size_t index001 = ImageData::getScalarIndex(s1.x(), s1.y(), s2.z(), dim, numComps) + comp;
	const size_t index101 = ImageData::getScalarIndex(s2.x(), s1.y(), s2.z(), dim, numComps) + comp;
	const size_t index111 = ImageData::getScalarIndex(s2.x(), s2.y(), s2.z(), dim, numComps) + comp;
	const size_t index011 = ImageData::getScalarIndex(s1.x(), s2.y(), s2.z(), dim, numComps) + comp;

	const double val000 = static_cast<double>(imgPtr[index000]);
	const double val100 = static_cast<double>(imgPtr[index100]);
	const double val110 = static_cast<double>(imgPtr[index110]);
	const double val010 = static_cast<double>(imgPtr[index010]);

	const double val001 = static_cast<double>(imgPtr[index001]);
	const double val101 = static_cast<double>(imgPtr[index101]);
	const double val111 = static_cast<double>(imgPtr[index111]);
	const double val011 = static_cast<double>(imgPtr[index011]);

	// Interpolants
	const Vec3d t = s2.cast<double>() - structuredPt;

	// Interpolate along x
	const double ax = val000 + (val100 - val000) * -t[0];
	const double bx = val010 + (val110 - val010) * -t[0];

	const double dx = val001 + (val101 - val001) * -t[0];
	const double ex = val011 + (val111 - val011) * -t[0];

	// Interpolate along y
	const double cy = ax + (bx - ax) * -t[1];
	const double fy = dx + (ex - dx) * -t[1];

	// Interpolate along z
	const double gz = fy + (cy - fy) * t[2];

	return static_cast<T>(gz);
}

template<typename T>
static void
resample(std::shared_ptr<ImageData> inputImage, std::shared_ptr<ImageData> outputImage,
	const Vec3i& inputDim, const Vec3i& outputDim)
{
	T* inputImgPtr = std::dynamic_pointer_cast<DataArray<T>>(inputImage->getScalars())->getPointer();
	T* outputImgPtr = std::dynamic_pointer_cast<DataArray<T>>(outputImage->getScalars())->getPointer();
	const int numComps = inputImage->getNumComponents();

	ParallelUtils::parallelFor(outputDim[2], [&](const int& z)
		{
			int i = z * outputDim[0] * outputDim[1] * numComps;
			for (int y = 0; y < outputDim[1]; y++)
			{
				for (int x = 0; x < outputDim[0]; x++, i += numComps)
				{
					//const Vec3d structuredPt = Vec3d(x, y, z).cwiseProduct(ratio);
					// Physical pos of img2
					const Vec3d outImgPos = Vec3d(x, y, z).cwiseProduct(outputImage->getSpacing()) + outputImage->getSpacing() * 0.5;
					const Vec3d inImgStructuredPt = outImgPos.cwiseQuotient(inputImage->getSpacing());
					for (int j = 0; j < numComps; j++)
					{
						outputImgPtr[i + j] = trilinearSample(inImgStructuredPt, inputImgPtr, inputDim, numComps, j);
					}
				}
			}
		});
}

ImageResample::ImageResample()
{
	setNumberOfInputPorts(1);
	setNumberOfOutputPorts(1);
	setOutput(std::make_shared<ImageData>());
}

std::shared_ptr<ImageData>
ImageResample::getOutputImage() const
{
	return std::dynamic_pointer_cast<ImageData>(getOutput(0));
}

void
ImageResample::setInputImage(std::shared_ptr<ImageData> inputData)
{
	setInput(inputData, 0);
}

void
ImageResample::requestUpdate()
{
	std::shared_ptr<ImageData> inputImage = std::dynamic_pointer_cast<ImageData>(getInput(0));
	if (inputImage == nullptr)
	{
		LOG(WARNING) << "No inputImage to resample";
		return;
	}

	const Vec3i& inputDim = inputImage->getDimensions();
	const Vec3i& outputDim = Dimensions;

	const Vec3d inputSpacing = inputImage->getSpacing();
	const Vec3d inputSize = inputDim.cast<double>().cwiseProduct(inputSpacing);
	const Vec3d outputSpacing = inputSize.cwiseQuotient(outputDim.cast<double>());

	std::shared_ptr<ImageData> outputImage = std::make_shared<ImageData>();
	outputImage->allocate(inputImage->getScalarType(), inputImage->getNumComponents(),
		outputDim, outputSpacing, inputImage->getOrigin());

	switch (outputImage->getScalarType())
	{
		TemplateMacro(resample<IMSTK_TT>(inputImage, outputImage, inputDim, outputDim));
	default:
		break;
	}
	setOutput(outputImage);
}
}