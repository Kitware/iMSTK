// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "ImageSave.h"

class ImageExample: public smCoreClass
{

    smSDK* simmedtkSDK;
public:

    ImageExample()
    {
        ///create the SDK
        simmedtkSDK = smSDK::createSDK();

        ///Init the texture manager before using it
        smTextureManager::init(smSDK::getErrorLog());

        ///Generate a binary image
        smChar data[128 * 128];
        smChar data1[255 * 255 * 3];

        for (int j = 0; j < 128; j++)
            for (int i = 0; i < 128; i++)
            {
                data[i + j * 128] = i % 2 * 255;
                data[i + j * 128] = j % 2 * 255;
            }

        //save image
        for (int j = 0; j < 255; j++)
            for (int i = 0; i < 255; i++)
            {
                data1[i * 3 + j * 255 * 3] = 255;
                data1[i * 3 + j * 255 * 3 + 1] = (i % 2 == 0 ? 255 : 120);
                data1[i * 3 + j * 255 * 3 + 2] = 255;
            }

        ///save the data in JPG format. file extension(jpg) specifies the file format
        smTextureManager::saveRGBImage(data1, 255, 255, "ImageExampletest.jpg");
        cout << "RGB Image is created" << endl;

        ///save the file in binary format. the extension will be always bmp.
        smTextureManager::saveBinaryImage(data, 128, 128, "ImageExampletest.bmp");
        cout << "Bitmap Image is created" << endl;

        ///run the sdk
        simmedtkSDK->run();
    }
};

int main(int argc, char** argv)
{
    ImageExample *img = new ImageExample();
    delete img;
    return 0;
}
