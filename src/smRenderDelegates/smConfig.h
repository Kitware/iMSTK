#ifndef SMRENDERDELEGATES_CONFIG_H
#define SMRENDERDELEGATES_CONFIG_H

#include "smCore/smConfig.h"

#define SIMMEDTK_REGISTER_RENDER_DELEGATES() \
  SIMMEDTK_RUN_LOADER(register_rendering_viewer); \
  SIMMEDTK_RUN_LOADER(register_aabb_render_delegate); \
  SIMMEDTK_RUN_LOADER(register_lattice_render_delegate); \
  SIMMEDTK_RUN_LOADER(register_mesh_render_delegate); \
  SIMMEDTK_RUN_LOADER(register_plane_render_delegate); \
  SIMMEDTK_RUN_LOADER(register_scene_object_deformable_render_delegate); \
  SIMMEDTK_RUN_LOADER(register_static_scene_object_render_delegate); \
  SIMMEDTK_RUN_LOADER(register_stylus_render_delegate); \
  SIMMEDTK_RUN_LOADER(register_surface_tree_render_delegate)
  //SIMMEDTK_RUN_LOADER(register_fem_scene_render_delegate); \
  //SIMMEDTK_RUN_LOADER(register_pbd_surface_render_delegate); \
  //SIMMEDTK_RUN_LOADER(register_physx_volume_mesh_render_delegate); \

#endif // SMRENDERDELEGATES_CONFIG_H
