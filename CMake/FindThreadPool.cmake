find_path(ThreadPool_INCLUDE_DIR
  NAMES
    ThreadPool.h
    )

set(ThreadPool_INCLUDE_DIRS "${ThreadPool_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ThreadPool
  REQUIRED_VARS
    ThreadPool_INCLUDE_DIR)

mark_as_advanced(
  ThreadPool_INCLUDE_DIR)
