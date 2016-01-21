set(MATRIX_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Matrices)
if(NOT EXISTS ${MATRIX_DIRECTORY})
    file(MAKE_DIRECTORY ${MATRIX_DIRECTORY})
endif()

include(MatrixMarket)
include(SimMedTKMacros)

# Extract matrices for solvers testing
extract_matrix("http://midas3.kitware.com/midas/download/item/318786/TestMatrices.tar.gz")

# Extract data used in examples and some unit tests
set(EXAMPLES_DATA ${CMAKE_CURRENT_BINARY_DIR}/ExamplesData)

if(NOT EXISTS ${EXAMPLES_DATA})
    file(MAKE_DIRECTORY ${EXAMPLES_DATA})
endif()

# Nidus
set(NIDUS_DATA "${EXAMPLES_DATA}/Nidus")
if(NOT EXISTS ${NIDUS_DATA})
    file(MAKE_DIRECTORY ${NIDUS_DATA})
endif()

set(FILE_URL http://midas3.kitware.com/midas/download/item/318709/nidus.tar)
set(LOCAL_FILE ${EXAMPLES_DATA}/nidus.tar)

download_data(${FILE_URL} ${LOCAL_FILE})

execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xf ${LOCAL_FILE}
    WORKING_DIRECTORY ${NIDUS_DATA}
)

# Box
set(BOX_DATA "${EXAMPLES_DATA}/Box")
if(NOT EXISTS ${BOX_DATA})
    file(MAKE_DIRECTORY ${BOX_DATA})
endif()

set(FILE_URL http://midas3.kitware.com/midas/download/item/318708/box.tar)
set(LOCAL_FILE ${EXAMPLES_DATA}/box.tar)

download_data(${FILE_URL} ${LOCAL_FILE})

execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xf ${LOCAL_FILE}
    WORKING_DIRECTORY ${BOX_DATA}
)

# Camera
set(CAMERA_DATA "${EXAMPLES_DATA}/Camera")
if(NOT EXISTS ${CAMERA_DATA})
    file(MAKE_DIRECTORY ${CAMERA_DATA})
endif()

set(FILE_URL http://midas3.kitware.com/midas/download/item/318840/CameraNavAppData.tar)
set(LOCAL_FILE ${EXAMPLES_DATA}/CameraNavAppData.tar)

download_data(${FILE_URL} ${LOCAL_FILE})

execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xf ${LOCAL_FILE}
    WORKING_DIRECTORY ${CAMERA_DATA}
)

# Shaders
set(SHADERS_DATA "${EXAMPLES_DATA}/Shaders")
if(NOT EXISTS ${SHADERS_DATA})
    file(MAKE_DIRECTORY ${SHADERS_DATA})
endif()

set(FILE_URL http://midas3.kitware.com/midas/download/item/318851/ShadersData.tar)
set(LOCAL_FILE ${EXAMPLES_DATA}/ShadersData.tar)

download_data(${FILE_URL} ${LOCAL_FILE})

execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xf ${LOCAL_FILE}
    WORKING_DIRECTORY ${SHADERS_DATA}
)

set(SOURCE_PATH ${PROJECT_SOURCE_DIR})
set(BINARY_PATH ${PROJECT_BINARY_DIR})
configure_file(path.in ${PROJECT_BINARY_DIR}/Config.paths @ONLY)

add_subdirectory(Models)
