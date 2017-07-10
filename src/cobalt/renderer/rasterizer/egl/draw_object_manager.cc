// Copyright 2017 Google Inc. All Rights Reserved.
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

#include "cobalt/renderer/rasterizer/egl/draw_object_manager.h"

#include <algorithm>

#include "base/debug/trace_event.h"
#include "base/logging.h"
#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/renderer/backend/egl/utils.h"
#include "cobalt/renderer/rasterizer/egl/draw_callback.h"

namespace cobalt {
namespace renderer {
namespace rasterizer {
namespace egl {

DrawObjectManager::DrawObjectManager(
    const base::Closure& reset_external_rasterizer,
    const base::Closure& flush_external_offscreen_draws)
    : reset_external_rasterizer_(reset_external_rasterizer),
      flush_external_offscreen_draws_(flush_external_offscreen_draws),
      current_draw_id_(0) {}

uint32_t DrawObjectManager::AddOnscreenDraw(scoped_ptr<DrawObject> draw_object,
    BlendType blend_type, base::TypeId draw_type,
    const backend::RenderTarget* render_target,
    const math::RectF& draw_bounds) {
  onscreen_draws_.push_back(DrawInfo(
      draw_object.Pass(), draw_type, blend_type, render_target,
      draw_bounds, ++current_draw_id_));
  return current_draw_id_;
}

uint32_t DrawObjectManager::AddOffscreenDraw(scoped_ptr<DrawObject> draw_object,
    BlendType blend_type, base::TypeId draw_type,
    const backend::RenderTarget* render_target,
    const math::RectF& draw_bounds) {
  // Put all draws using kBlendExternal into their own draw list since they
  // use an external rasterizer which tracks its own state.
  auto* draw_list = &offscreen_draws_;
  if (blend_type == kBlendExternal) {
    draw_list = &external_offscreen_draws_;

    // Only the DrawCallback type should use kBlendExternal. All other draw
    // object types use the native rasterizer.
    DCHECK(base::polymorphic_downcast<DrawCallback*>(draw_object.get()));
  }

  draw_list->push_back(DrawInfo(
      draw_object.Pass(), draw_type, blend_type, render_target,
      draw_bounds, ++current_draw_id_));
  return current_draw_id_;
}

void DrawObjectManager::RemoveDraws(uint32_t last_valid_draw_id) {
  TRACE_EVENT0("cobalt::renderer", "RemoveDraws");
  RemoveDraws(&onscreen_draws_, last_valid_draw_id);
  RemoveDraws(&offscreen_draws_, last_valid_draw_id);
  RemoveDraws(&external_offscreen_draws_, last_valid_draw_id);
}

void DrawObjectManager::RemoveDraws(std::vector<DrawInfo>* draw_list,
    uint32_t last_valid_draw_id) {
  // Objects in the draw list should have ascending draw IDs at this point.
  auto iter = draw_list->end();
  for (; iter != draw_list->begin(); --iter) {
    if ((iter - 1)->draw_id <= last_valid_draw_id) {
      break;
    }
  }
  if (iter != draw_list->end()) {
    draw_list->erase(iter, draw_list->end());
  }
}

void DrawObjectManager::ExecuteOffscreenRasterize(GraphicsState* graphics_state,
    ShaderProgramManager* program_manager) {
  SortOffscreenDraws(&external_offscreen_draws_);

  // Process draws handled by an external rasterizer.
  {
    TRACE_EVENT0("cobalt::renderer", "OffscreenExternalRasterizer");
    for (auto draw = external_offscreen_draws_.begin();
         draw != external_offscreen_draws_.end(); ++draw) {
      draw->draw_object->ExecuteRasterize(graphics_state, program_manager);
    }
    if (!flush_external_offscreen_draws_.is_null()) {
      flush_external_offscreen_draws_.Run();
    }
  }

  SortOffscreenDraws(&offscreen_draws_);
  SortOnscreenDraws(&onscreen_draws_);

  // Update the vertex buffer for all draws.
  {
    TRACE_EVENT0("cobalt::renderer", "UpdateVertexBuffer");
    ExecuteUpdateVertexBuffer(graphics_state, program_manager);
  }

  // Process the native offscreen draws.
  {
    TRACE_EVENT0("cobalt::renderer", "OffscreenNativeRasterizer");
    Rasterize(offscreen_draws_, graphics_state, program_manager);
  }
}

void DrawObjectManager::ExecuteUpdateVertexBuffer(
    GraphicsState* graphics_state, ShaderProgramManager* program_manager) {
  for (auto draw = offscreen_draws_.begin(); draw != offscreen_draws_.end();
       ++draw) {
    draw->draw_object->ExecuteUpdateVertexBuffer(
        graphics_state, program_manager);
  }
  for (auto draw = onscreen_draws_.begin(); draw != onscreen_draws_.end();
       ++draw) {
    draw->draw_object->ExecuteUpdateVertexBuffer(
        graphics_state, program_manager);
  }
  graphics_state->UpdateVertexData();
}

void DrawObjectManager::ExecuteOnscreenRasterize(GraphicsState* graphics_state,
    ShaderProgramManager* program_manager) {
  Rasterize(onscreen_draws_, graphics_state, program_manager);
}

void DrawObjectManager::Rasterize(const std::vector<DrawInfo>& draw_list,
    GraphicsState* graphics_state, ShaderProgramManager* program_manager) {
  const backend::RenderTarget* current_target = nullptr;
  bool using_native_rasterizer = true;

  // Starting from an unknown state.
  graphics_state->SetDirty();

  for (auto draw = draw_list.begin(); draw != draw_list.end(); ++draw) {
    bool draw_uses_native_rasterizer = draw->blend_type != kBlendExternal;

    if (draw_uses_native_rasterizer) {
      if (!using_native_rasterizer) {
        // Transitioning from external to native rasterizer. Set the native
        // rasterizer's state as dirty.
        graphics_state->SetDirty();
      }

      if (draw->render_target != current_target) {
        current_target = draw->render_target;
        graphics_state->BindFramebuffer(current_target);
        graphics_state->Viewport(0, 0,
                                 current_target->GetSize().width(),
                                 current_target->GetSize().height());
      }

      if (draw->blend_type == kBlendNone) {
        graphics_state->DisableBlend();
      } else if (draw->blend_type == kBlendSrcAlpha) {
        graphics_state->EnableBlend();
      } else {
        NOTREACHED() << "Unsupported blend type";
      }
    } else {
      if (using_native_rasterizer) {
        // Transitioning from native to external rasterizer. Set the external
        // rasterizer's state as dirty.
        if (!reset_external_rasterizer_.is_null()) {
          reset_external_rasterizer_.Run();
        }
      }
    }

    draw->draw_object->ExecuteRasterize(graphics_state, program_manager);
    using_native_rasterizer = draw_uses_native_rasterizer;
  }
}

void DrawObjectManager::SortOffscreenDraws(std::vector<DrawInfo>* draw_list) {
  TRACE_EVENT0("cobalt::renderer", "SortOffscreenDraws");

  // Sort offscreen draws to minimize GPU state changes.
  for (auto iter = draw_list->begin(); iter != draw_list->end(); ++iter) {
    for (auto current_draw = iter; current_draw != draw_list->begin();
         std::swap(*current_draw, *(current_draw - 1)), current_draw--) {
      auto prev_draw = current_draw - 1;

      // Unlike onscreen draws, offscreen draws should be grouped by
      // render target.
      if (prev_draw->render_target > current_draw->render_target) {
        continue;
      } else if (prev_draw->render_target < current_draw->render_target) {
        break;
      }

      if (prev_draw->draw_bounds.Intersects(current_draw->draw_bounds)) {
        break;
      }

      if (prev_draw->draw_type > current_draw->draw_type) {
        continue;
      } else if (prev_draw->draw_type < current_draw->draw_type) {
        break;
      }

      if (prev_draw->blend_type <= current_draw->blend_type) {
        break;
      }
    }
  }
}

void DrawObjectManager::SortOnscreenDraws(std::vector<DrawInfo>* draw_list) {
  TRACE_EVENT0("cobalt::renderer", "SortOnscreenDraws");

  // Sort onscreen draws to minimize GPU state changes.
  for (auto iter = draw_list->begin(); iter != draw_list->end(); ++iter) {
    for (auto current_draw = iter; current_draw != draw_list->begin();
         std::swap(*current_draw, *(current_draw - 1)), current_draw--) {
      auto prev_draw = current_draw - 1;

      // Do not sort across different render targets since their contents may
      // be generated just before consumed by a subsequent draw.
      if (prev_draw->render_target != current_draw->render_target) {
        break;
      }

      if (prev_draw->draw_bounds.Intersects(current_draw->draw_bounds)) {
        break;
      }

      // Group native vs. non-native draws together.
      bool next_uses_same_rasterizer = current_draw + 1 != draw_list->end() &&
          ((current_draw + 1)->blend_type == kBlendExternal) ==
          (current_draw->blend_type == kBlendExternal);
      bool prev_uses_same_rasterizer =
          (prev_draw->blend_type == kBlendExternal) ==
          (current_draw->blend_type == kBlendExternal);
      if (!next_uses_same_rasterizer && !prev_uses_same_rasterizer) {
        continue;
      }
      if (next_uses_same_rasterizer && !prev_uses_same_rasterizer) {
        break;
      }

      if (prev_draw->draw_type > current_draw->draw_type) {
        continue;
      } else if (prev_draw->draw_type < current_draw->draw_type) {
        break;
      }

      if (prev_draw->blend_type <= current_draw->blend_type) {
        break;
      }
    }
  }
}

}  // namespace egl
}  // namespace rasterizer
}  // namespace renderer
}  // namespace cobalt
