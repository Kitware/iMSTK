
set(EXAMPLES_DATA ${CMAKE_CURRENT_BINARY_DIR}/Data)
if(NOT EXISTS ${EXAMPLES_DATA})
    file(MAKE_DIRECTORY ${EXAMPLES_DATA})
endif()

function(MIDAS_DOWNLOAD MIDAS_URL FILE_SHA1 LOCAL_FILE)
    if(NOT EXISTS ${LOCAL_FILE})
        file(
            DOWNLOAD "${MIDAS_URL}"
            "${LOCAL_FILE}"
            SHOW_PROGRESS
            STATUS DOWNLOAD_STATUS
            TIMEOUT 15
            TLS_VERIFY ON
            EXPECTED_HASH SHA1=${FILE_SHA1}
        )
        list(GET DOWNLOAD_STATUS 0 STATUS0)
        if(NOT STATUS0 STREQUAL "0")
            list(GET DOWNLOAD_STATUS 1 STATUS1)
            message("${STATUS1}")
        endif()
    endif()
endfunction()

# add_subdirectory(common)

if(USE_VRPN_CLIENT OR USE_VRPN_SERVER)
  # add_subdirectory(vrpnPhantomTest)
endif()

if(SimMedTK_USE_OCULUS)
  add_subdirectory(renderCubeOculus)
  add_subdirectory(renderCubeToTextureOculus)
endif(SimMedTK_USE_OCULUS)

# add_subdirectory(AudioExample)
# add_subdirectory(renderCube)
# add_subdirectory(renderCubeToTexture)
add_subdirectory(vegaFem)
# add_subdirectory(MultipleObjects)
# add_subdirectory(CollisionDetectionSpatialHashing)
# add_subdirectory(CollisionDetectionBVH)
#add_subdirectory(demoExample)
# add_subdirectory(AVMNidus)
