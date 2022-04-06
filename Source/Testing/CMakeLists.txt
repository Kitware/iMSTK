#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

if( ${PROJECT_NAME}_BUILD_TESTING )
    include(imstkAddLibrary)

    imstk_add_library( Testing
      H_FILES
        imstkTestingUtils.h
      CPP_FILES
        imstkTestingMain.cpp
      DEPENDS
        Common
        GTest::gtest
        VTK::CommonSystem
      )

    if (${PROJECT_NAME}_BUILD_VISUAL_TESTING)
        # Links to a great deal of imstk to provide
        # visual testing capabilities
        imstk_add_library( VisualTesting
          H_FILES
            imstkVisualTestingUtils.h
          CPP_FILES
            imstkVisualTestingUtils.cpp
          DEPENDS
            Testing
            SimulationManager
            ViewerVTK
          )
    endif()
endif()