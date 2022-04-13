include(imstkAddTest)
imstk_add_test( Geometry )
target_link_libraries(GeometryTests PUBLIC MeshIO)