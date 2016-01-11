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

add_subdirectory(FEMSimulator)
add_subdirectory(LaparoscopicCamera)
add_subdirectory(AVMNidus)
add_subdirectory(Shaders)
