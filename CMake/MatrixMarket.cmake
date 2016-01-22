###########################################################################
#
# Copyright (c) Kitware, Inc.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0.txt
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################

set(MATRIX_MARKET_URL "ftp://math.nist.gov/pub/MatrixMarket2/Harwell-Boeing/")

# Power system networks
set(MATRICES_PSADMIT
  "pub/MatrixMarket2/Harwell-Boeing/psadmit/662_bus.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/psadmit/494_bus.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/psadmit/685_bus.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/psadmit/1138_bus.mtx.gz")

set(MATRICES_BCSSTRUC
# Static analyses in structural engineering
#http://math.nist.gov/MatrixMarket/data/Harwell-Boeing/bcsstruc2/bcsstruc2.html
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc2/bcsstk14.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc2/bcsstk15.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc2/bcsstk16.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc2/bcsstk17.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc2/bcsstk18.mtx.gz"
# Dynamic analyses in structural engineering
#http://math.nist.gov/MatrixMarket/data/Harwell-Boeing/bcsstruc3/bcsstruc3.html
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc3/bcsstm19.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc3/bcsstm20.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc3/bcsstm21.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc3/bcsstm22.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc3/bcsstm23.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc3/bcsstm24.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc3/bcsstm25.mtx.gz"
http://math.nist.gov/MatrixMarket/data/Harwell-Boeing/bcsstruc4/bcsstruc4.html
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc4/bcsstk26.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc4/bcsstk27.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc4/bcsstk28.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc4/bcsstm26.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/bcsstruc4/bcsstm27.mtx.gz")

#Linear equations in structural engineering
set(MATRICES_LANPRO
#http://math.nist.gov/MatrixMarket/data/Harwell-Boeing/lanpro/lanpro.html
  "pub/MatrixMarket2/Harwell-Boeing/lanpro/nos1.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/lanpro/nos2.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/lanpro/nos3.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/lanpro/nos4.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/lanpro/nos5.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/lanpro/nos6.mtx.gz"
  "pub/MatrixMarket2/Harwell-Boeing/lanpro/nos7.mtx.gz"
  )

# Partial differential equations
set(MATRICES_LAPLACE
  "pub/MatrixMarket2/Harwell-Boeing/laplace/gr_30_30.mtx.gz"
)

# Finite element analysis of cylindrical shells
set(MATRICES_SHELLS
#http://math.nist.gov/MatrixMarket/data/misc/cylshell/cylshell.html
  "pub/MatrixMarket2/misc/cylshell/s1rmq4m1.mtx.gz"
  "pub/MatrixMarket2/misc/cylshell/s2rmq4m1.mtx.gz"
  "pub/MatrixMarket2/misc/cylshell/s3rmq4m1.mtx.gz"
  "pub/MatrixMarket2/misc/cylshell/s1rmt3m1.mtx.gz"
  "pub/MatrixMarket2/misc/cylshell/s2rmt3m1.mtx.gz"
  "pub/MatrixMarket2/misc/cylshell/s3rmt3m1.mtx.gz"
  "pub/MatrixMarket2/misc/cylshell/s3dkq4m2.mtx.gz"
  "pub/MatrixMarket2/misc/cylshell/s3dkq3m2.mtx.gz"
  "pub/MatrixMarket2/misc/cylshell/s3dkq3m3.mtx.gz"
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
