#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( Common
  H_FILES
    imstkAbstractDataArray.h
    imstkColor.h
    imstkColorFunction.h
    imstkDataArray.h
    imstkEventObject.h
    imstkFactory.h
    imstkLogger.h
    imstkLoggerG3.h
    imstkLoggerSynchronous.h
    imstkMacros.h
    imstkMath.h
    imstkModule.h
    imstkModuleDriver.h
    imstkNew.h
    imstkTypes.h
    imstkVecDataArray.h
    Parallel/imstkAtomicOperations.h
    Parallel/imstkParallelFor.h
    Parallel/imstkParallelReduce.h
    Parallel/imstkParallelUtils.h
    Parallel/imstkSpinLock.h
    Parallel/imstkThreadManager.h
    TaskGraph/imstkSequentialTaskGraphController.h
    TaskGraph/imstkTaskGraph.h
    TaskGraph/imstkTaskGraphController.h
    TaskGraph/imstkTaskGraphVizWriter.h
    TaskGraph/imstkTaskNode.h
    TaskGraph/imstkTbbTaskGraphController.h
    Utils/imstkTimer.h
  CPP_FILES
    imstkColor.cpp
    imstkLoggerG3.cpp
    imstkLoggerSynchronous.cpp
    imstkModule.cpp
    imstkModuleDriver.cpp
    Parallel/imstkThreadManager.cpp
    TaskGraph/imstkSequentialTaskGraphController.cpp
    TaskGraph/imstkTaskGraph.cpp
    TaskGraph/imstkTaskGraphController.cpp
    TaskGraph/imstkTaskGraphVizWriter.cpp
    TaskGraph/imstkTaskNode.cpp
    TaskGraph/imstkTbbTaskGraphController.cpp
    Utils/imstkTimer.cpp
  DEPENDS
    Eigen3::Eigen
    g3log::g3log
    TBB::tbb
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()
