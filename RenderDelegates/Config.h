// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef SMRENDERDELEGATES_CONFIG_H
#define SMRENDERDELEGATES_CONFIG_H

#include "Core/Config.h"

#define SIMMEDTK_REGISTER_RENDER_DELEGATES() \
  SIMMEDTK_RUN_LOADER(register_rendering_viewer); \
  SIMMEDTK_RUN_LOADER(register_aabb_render_delegate); \
  SIMMEDTK_RUN_LOADER(register_mesh_render_delegate); \
  SIMMEDTK_RUN_LOADER(register_plane_render_delegate); \
  SIMMEDTK_RUN_LOADER(register_scene_object_deformable_render_delegate); \
  SIMMEDTK_RUN_LOADER(register_static_scene_object_render_delegate); \
  SIMMEDTK_RUN_LOADER(register_surface_tree_render_delegate)
//   SIMMEDTK_RUN_LOADER(register_stylus_render_delegate); \
  //SIMMEDTK_RUN_LOADER(register_fem_scene_render_delegate);
  //SIMMEDTK_RUN_LOADER(register_pbd_surface_render_delegate);
  //SIMMEDTK_RUN_LOADER(register_physx_volume_mesh_render_delegate);

#endif // SMRENDERDELEGATES_CONFIG_H
