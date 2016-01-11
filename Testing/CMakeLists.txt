set(MATRIX_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Matrices)
if(NOT EXISTS ${MATRIX_DIRECTORY})
    file(MAKE_DIRECTORY ${MATRIX_DIRECTORY})
endif()

include(MatrixMarket)
extract_matrix("http://midas3.kitware.com/midas/download/item/318786/TestMatrices.tar.gz")

add_subdirectory(Models)
