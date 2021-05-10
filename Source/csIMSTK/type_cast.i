%inline %{
template <typename Base, typename Derived>
Derived* type_cast(Base* base) {
    return dynamic_cast<Derived*>(base);
}
%}

%template(castToRigidBodyCH) type_cast<imstk::CollisionHandling, imstk::expiremental::RigidBodyCH>;
%template(castToVecDataArray3uc) type_cast<imstk::AbstractDataArray, imstk::VecDataArray<unsigned char, 3>>;

%template(castToLineMesh) type_cast<imstk::PointSet, imstk::LineMesh>;
%template(castToSurfaceMesh) type_cast<imstk::PointSet, imstk::SurfaceMesh>;
%template(castToTetrahedralMesh) type_cast<imstk::PointSet, imstk::TetrahedralMesh>;
%template(castToHexahedral) type_cast<imstk::PointSet, imstk::HexahedralMesh>;
%template(castToImageData) type_cast<imstk::PointSet, imstk::ImageData>;
