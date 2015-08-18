set(Module IO)

configure_file(IOMeshSpec.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/IOMeshSpec.cpp @ONLY)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  ${CMAKE_CURRENT_BINARY_DIR}/IOMeshSpec.cpp
  )

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)

target_link_libraries(${Module}UnitTestRunner Core Mesh IO)

simple_test(${Module} --reporter=spec)

set(VOLUME_MESH_SAMPLE
"# Vega mesh file.
# 4 vertices, 1 elements

*VERTICES
4 3 0 0
1 -2.44627 -0.903874999999999 -1.711465
2 -2.008655 -0.762779999999999 -1.63081
3 -2.248035 -0.599385 -1.41836
4 -2.2993347930303 -1.06841207461711 -0.999830245926915

*ELEMENTS
TET
1 4 0
1 1 2 3 4

*MATERIAL defaultMaterial
ENU, 1000, 1000000000, 0.45

*SET defaultSet
1,

*MATERIAL BODY
ENU, 1000,
allElements, BODY10000000, 0.4

*REGION
allElements, BODY")

set(VTK_TEST_MESH
"<?xml version=\"1.0\"?>
<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\" compressor=\"vtkZLibDataCompressor\">
  <UnstructuredGrid>
    <FieldData>
      <DataArray type=\"UInt32\" Name=\"boundary_conditions\" NumberOfTuples=\"5\" format=\"ascii\" RangeMin=\"0\" RangeMax=\"4\">
        0 1 2 3 4
      </DataArray>
      <DataArray type=\"Float64\" Name=\"mass_density\" NumberOfTuples=\"1\" format=\"ascii\" RangeMin=\"1000\" RangeMax=\"1000\">
        1000
      </DataArray>
      <DataArray type=\"Float64\" Name=\"poisson_ratio\" NumberOfTuples=\"1\" format=\"ascii\" RangeMin=\"0.45\" RangeMax=\"0.45\">
        0.45
      </DataArray>
      <DataArray type=\"Float64\" Name=\"young_modulus\" NumberOfTuples=\"1\" format=\"ascii\" RangeMin=\"75000\" RangeMax=\"75000\">
        75000
      </DataArray>
    </FieldData>
    <Piece NumberOfPoints=\"9\" NumberOfCells=\"24\">
      <PointData>
      </PointData>
      <CellData>
      </CellData>
      <Points>
        <DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\" RangeMin=\"3.9720546452e-09\" RangeMax=\"0.86602540378\">
          -0.5 -0.5 -0.5 -0.5 -0.5 0.5
          -0.5 0.5 -0.5 -0.5 0.5 0.5
          0.5 -0.5 -0.5 0.5 -0.5 0.5
          0.5 0.5 -0.5 0.5 0.5 0.5
          3.5527136788e-09 -1.7763568394e-09 0
        </DataArray>
      </Points>
      <Cells>
        <DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\" RangeMin=\"0\" RangeMax=\"8\">
          0 1 2 0 4 1
          0 2 4 3 2 1
          1 5 3 5 1 4
          2 3 6 6 4 2
          7 3 5 7 6 3
          4 6 5 7 5 6
          2 1 8 3 1 4
          8 5 4 2 0 8
          1 4 0 8 3 5
          1 8 3 5 8 7
          2 8 6 3 8 4
          6 5 4 2 8 6
          6 3 8 7 2 1
          0 8 5 6 8 7
        </DataArray>
        <DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\" RangeMin=\"3\" RangeMax=\"84\">
          3 6 9 12 15 18
          21 24 27 30 33 36
          40 44 48 52 56 60
          64 68 72 76 80 84
        </DataArray>
        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\" RangeMin=\"5\" RangeMax=\"10\">
          5 5 5 5 5 5
          5 5 5 5 5 5
          10 10 10 10 10 10
          10 10 10 10 10 10
        </DataArray>
      </Cells>
    </Piece>
  </UnstructuredGrid>
</VTKFile>
"
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/SampleMesh.veg ${VOLUME_MESH_SAMPLE})
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/SampleMesh.vtu ${VTK_TEST_MESH})
