#ifndef SMRENDERABLE_H
#define SMRENDERABLE_H

#include "Core/RenderDetail.h" // for GeometrySource

#include <memory> // for shared_ptr

class RenderDelegate
{
public:

    enum RendererType
    {
        VTK,
        Other
    };

    typedef std::shared_ptr<RenderDelegate> Ptr;

    virtual void initDraw()
    { }
    virtual void draw() const
    { }
    virtual void modified()
    { }

    virtual bool isTargetTextured() const
    {
        return false;
    }

    template<typename T>
    void setSourceGeometry ( T* srcGeom )
    {
        this->sourceGeometry.setSource ( srcGeom );
    }

    template<typename T>
    T* getSourceGeometryAs() const
    {
        return this->sourceGeometry.sourceAs<T>();
    }
protected:
    GeometrySource sourceGeometry; // object to render when draw() is called
    std::string type;
};

#endif // SMRENDERABLE_H

