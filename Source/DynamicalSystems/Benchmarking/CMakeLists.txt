###########################################################################
#
# This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
# iMSTK is distributed under the Apache License, Version 2.0.
# See accompanying NOTICE for details. 
#
###########################################################################


project(PbdBenchmark)

#-----------------------------------------------------------------------------
# Create executable
#-----------------------------------------------------------------------------
imstk_add_executable(${PROJECT_NAME} PbdBenchmark.cpp)

SET_TARGET_PROPERTIES (${PROJECT_NAME} PROPERTIES FOLDER Benchmarking)

#-----------------------------------------------------------------------------
# Link libraries to executable
#-----------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME}
	SimulationManager
	benchmark::benchmark)