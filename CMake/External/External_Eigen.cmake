#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( Eigen
  REPOSITORY ${git_protocol}://github.com/RLovelett/eigen.git
  GIT_TAG e81ac4502ebbfde43a1e1761b36853d3ead47c33
  #DEPENDENCIES ""
  #VERBOSE
  )
