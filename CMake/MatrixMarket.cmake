
set(MATRIX_MARKET_URL "ftp://math.nist.gov/pub/MatrixMarket2/Harwell-Boeing")
set(MATRICES
  "psadmit/662_bus.mtx.gz"
  "psadmit/494_bus.mtx.gz"
  "psadmit/685_bus.mtx.gz"
  "psadmit/1138_bus.mtx.gz"
  "bcsstruc2/bcsstk14.mtx.gz"
  "bcsstruc2/bcsstk15.mtx.gz"
  "bcsstruc2/bcsstk16.mtx.gz"
  "bcsstruc2/bcsstk17.mtx.gz"
  "bcsstruc2/bcsstk18.mtx.gz"
  "bcsstruc3/bcsstm19.mtx.gz"
  "bcsstruc3/bcsstm20.mtx.gz"
  "bcsstruc3/bcsstm21.mtx.gz"
  "bcsstruc3/bcsstm22.mtx.gz"
  "bcsstruc3/bcsstm23.mtx.gz"
  "bcsstruc3/bcsstm24.mtx.gz"
  "bcsstruc3/bcsstm25.mtx.gz"
  "bcsstruc4/bcsstk26.mtx.gz"
  "bcsstruc4/bcsstk27.mtx.gz"
  "lanpro/nos1.mtx.gz"
  "lanpro/nos2.mtx.gz"
  "lanpro/nos3.mtx.gz"
  "lanpro/nos4.mtx.gz"
  "lanpro/nos5.mtx.gz"
  "lanpro/nos6.mtx.gz"
  "lanpro/nos7.mtx.gz"
  )

# Download and extract matrix
function(EXTRACT_MATRIX url)
  get_filename_component(file ${url} NAME_WE)
  get_filename_component(DOWNLOADED_FILENAME ${url} NAME)

  set(DATA_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/MatrixData)
  set(DOWNLOADED_FILENAME ${DATA_DIRECTORY}/${DOWNLOADED_FILENAME})
  download_data(${url} ${DOWNLOADED_FILENAME})

  if(ARGN)
    add_custom_command(
        TARGET ${ARGN}
        POST_BUILD
    #     BYPRODUCTS ${CREATED_FILENAME}
        COMMAND ${CMAKE_COMMAND} -E tar xf ${DOWNLOADED_FILENAME}
        WORKING_DIRECTORY ${DATA_DIRECTORY}
        COMMENT "Unpacking ${DOWNLOADED_FILENAME}"
    )
  else(ARGN)
    # Uncompress the file
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar zvxf ${DOWNLOADED_FILENAME}
        WORKING_DIRECTORY ${DATA_DIRECTORY}
        RESULT_VARIABLE rv
    )
  endif(ARGN)
endfunction()
