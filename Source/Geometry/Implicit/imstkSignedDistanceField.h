/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDataArray.h"
#include "imstkImageData.h"
#include "imstkImplicitGeometry.h"

namespace imstk
{
//class ImageData;
//template<typename T> class DataArray;

///
/// \class SignedDistanceField
///
/// \brief Structured field of signed distances implemented with ImageData
/// The SDF differs in that when you scale an image via its spacing the
/// distance samples are then wrong. Here you can isotropically scale as you
/// wish
///
class SignedDistanceField : public ImplicitGeometry
{
public:
    ///
    /// \brief Constructor
    /// \param ImageData to utilize
    /// \param geometry name
    ///
    SignedDistanceField(std::shared_ptr<ImageData> imageData);
    ~SignedDistanceField() override = default;

    IMSTK_TYPE_NAME(SignedDistanceField)

    ///
    /// \brief Returns signed distance to surface at pos, returns clamped/nearest if out of bounds
    ///
    double getFunctionValue(const Vec3d& pos) const;

    ///
    /// \brief Returns signed distance to surface at coordinate
    /// inlined for performance
    ///
    inline double getFunctionValueCoord(const Vec3i& coord) const
    {
        if (coord[0] < m_imageDataSdf->getDimensions()[0] && coord[0] > 0
            && coord[1] < m_imageDataSdf->getDimensions()[1] && coord[1] > 0
            && coord[2] < m_imageDataSdf->getDimensions()[2] && coord[2] > 0)
        {
            return (*m_scalars)[m_imageDataSdf->getScalarIndex(coord)] * m_scale;
        }
        else
        {
            return std::numeric_limits<double>::min();
        }
    }

    ///
    /// \brief Returns the bounds of the field
    ///
    const Vec6d& getBounds() const { return m_bounds; }

    ///
    /// \brief Set the isotropic scale that is used/multplied with samples
    ///
    void setScale(const double scale) { m_scale = scale; }

    ///
    /// \brief Get the isotropic scale
    ///
    double getScale() const { return m_scale; }

    ///
    /// \brief Get the SDF as a float image
    ///
    std::shared_ptr<ImageData> getImage() const { return m_imageDataSdf; }

    void computeBoundingBox(Vec3d& min, Vec3d& max, const double paddingPercent) override;

    ///
    /// \brief Polymorphic clone, hides the declaration in superclass
    /// return own type
    ///
    std::unique_ptr<SignedDistanceField> clone()
    {
        return std::unique_ptr<SignedDistanceField>(cloneImplementation());
    }

protected:
    std::shared_ptr<ImageData> m_imageDataSdf;

    Vec3d  m_invSpacing;
    Vec6d  m_bounds;
    Vec3d  m_shift;
    double m_scale;

    std::shared_ptr<DataArray<double>> m_scalars;

private:
    SignedDistanceField* cloneImplementation() const
    {
        SignedDistanceField* geom = new SignedDistanceField(*this);
        // Deal with deep copy members
        geom->m_imageDataSdf = m_imageDataSdf->clone();
        geom->m_scalars      = std::dynamic_pointer_cast<DataArray<double>>(geom->m_imageDataSdf->getScalars());
        return geom;
    }
};
} // namespace imstk