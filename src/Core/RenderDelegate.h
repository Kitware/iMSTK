#ifndef SMRENDERABLE_H
#define SMRENDERABLE_H

#include "RenderDetail.h" // for smGeometrySource

#include <memory> // for shared_ptr

class smRenderDelegate
{
public:
    typedef std::shared_ptr<smRenderDelegate> Ptr;

    virtual void initDraw() const
      { }
    virtual void draw() const
      { }

    virtual bool isTargetTextured() const
      { return false; }

    template<typename T>
    void setSourceGeometry(T* srcGeom)
      { this->sourceGeometry.setSource(srcGeom); }

    template<typename T>
    T* getSourceGeometryAs() const
      { return this->sourceGeometry.sourceAs<T>(); }

protected:
    smGeometrySource sourceGeometry; // object to render when draw() is called
};

#endif // SMRENDERABLE_H
