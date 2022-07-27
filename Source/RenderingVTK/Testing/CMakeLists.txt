###########################################################################
#
# This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
# iMSTK is distributed under the Apache License, Version 2.0.
# See accompanying NOTICE for details. 
#
###########################################################################


include(imstkAddTest)
imstk_add_test( RenderingVTK )
# Note: Links to ViewerVTK but only tests RenderingVTK
target_link_libraries(RenderingVTKTests PUBLIC ViewerVTK)