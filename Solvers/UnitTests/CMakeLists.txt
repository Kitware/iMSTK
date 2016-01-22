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

set(Module Solvers)

include(MatrixMarket)

add_executable(${Module}UnitTestRunner
  ${BANDIT_RUNNER}
  SolversSpec.cpp
)

target_compile_options(${Module}UnitTestRunner PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)
target_link_libraries(${Module}UnitTestRunner Core Solvers)
simple_test(${Module} --reporter=spec)

set(SOURCE_PATH ${PROJECT_SOURCE_DIR}/Testing/)
set(BINARY_PATH ${PROJECT_BINARY_DIR}/Testing/MatrixData)
configure_file(${PROJECT_SOURCE_DIR}/Testing/path.in ${CMAKE_CURRENT_BINARY_DIR}/${Module}Config.paths @ONLY)
