%inline %{
template <typename Base, typename Derived>
Derived* type_cast(Base* base) {
    return dynamic_cast<Derived*>(base);
}
%}

%template(castToRigidBodyCH) type_cast<imstk::CollisionHandling, imstk::expiremental::RigidBodyCH>;
%template(castToVecDataArray3uc) type_cast<imstk::AbstractDataArray, imstk::VecDataArray<unsigned char, 3>>;

%template(castToPointSet) type_cast<imstk::Geometry, imstk::PointSet>;
%template(castToLineMesh) type_cast<imstk::PointSet, imstk::LineMesh>;
%template(castToSurfaceMesh) type_cast<imstk::PointSet, imstk::SurfaceMesh>;
%template(castToTetrahedralMesh) type_cast<imstk::PointSet, imstk::TetrahedralMesh>;
%template(castToHexahedral) type_cast<imstk::PointSet, imstk::HexahedralMesh>;
%template(castToImageData) type_cast<imstk::PointSet, imstk::ImageData>;

%pragma(csharp) modulecode=%{
  public static T CastTo<T>(object from, bool cMemoryOwn=false)
  {
      System.Reflection.MethodInfo CPtrGetter = from.GetType().GetMethod("getCPtr", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Static);
      return CPtrGetter == null ? default(T) : (T) System.Activator.CreateInstance
      (
          typeof(T),
          System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance,
          null,
          new object[] { ((System.Runtime.InteropServices.HandleRef) CPtrGetter.Invoke(null, new object[] { from })).Handle, cMemoryOwn },
          null
      );
  }
%}

