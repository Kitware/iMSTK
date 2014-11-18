find_path(QGLVIEWER_INCLUDE_DIR
  QGLViewer/qglviewer.h)
find_library(QGLVIEWER_LIBRARY
  NAMES
    QGLViewer2)

set(QGLVIEWER_INCLUDE_DIRS "${QGLVIEWER_INCLUDE_DIR}")
set(QGLVIEWER_LIBRARIES "${QGLVIEWER_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QGLViewer
  REQUIRED_VARS
    QGLVIEWER_INCLUDE_DIR
    QGLVIEWER_LIBRARY)

mark_as_advanced(
  QGLVIEWER_INCLUDE_DIR
  QGLVIEWER_LIBRARY)