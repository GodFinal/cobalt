// Copyright 2014 The Cobalt Authors. All Rights Reserved.
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

#include "cobalt/render_tree/rect_node.h"

#include "cobalt/render_tree/brush_visitor.h"
#include "cobalt/render_tree/node_visitor.h"

namespace cobalt {
namespace render_tree {

RectNode::Builder::Builder(const math::RectF& rect) : rect(rect) {}

RectNode::Builder::Builder(const math::RectF& rect, scoped_ptr<Border> border)
    : rect(rect), border(border.Pass()) {}

RectNode::Builder::Builder(const math::RectF& rect, scoped_ptr<Border> border,
                           scoped_ptr<RoundedCorners> rounded_corners)
    : rect(rect),
      border(border.Pass()),
      rounded_corners(rounded_corners.Pass()) {}

RectNode::Builder::Builder(const math::RectF& rect,
                           scoped_ptr<Brush> background_brush)
    : rect(rect), background_brush(background_brush.Pass()) {}

RectNode::Builder::Builder(const math::RectF& rect,
                           scoped_ptr<RoundedCorners> rounded_corners)
    : rect(rect), rounded_corners(rounded_corners.Pass()) {}

RectNode::Builder::Builder(const math::RectF& rect,
                           scoped_ptr<Brush> background_brush,
                           scoped_ptr<RoundedCorners> rounded_corners)
    : rect(rect),
      background_brush(background_brush.Pass()),
      rounded_corners(rounded_corners.Pass()) {}

RectNode::Builder::Builder(const math::RectF& rect,
                           scoped_ptr<Brush> background_brush,
                           scoped_ptr<Border> border)
    : rect(rect),
      background_brush(background_brush.Pass()),
      border(border.Pass()) {}

RectNode::Builder::Builder(const math::RectF& rect,
                           scoped_ptr<Brush> background_brush,
                           scoped_ptr<Border> border,
                           scoped_ptr<RoundedCorners> rounded_corners)
    : rect(rect),
      background_brush(background_brush.Pass()),
      border(border.Pass()),
      rounded_corners(rounded_corners.Pass()) {}

RectNode::Builder::Builder(const Builder& other) : rect(other.rect) {
  if (other.background_brush) {
    background_brush = CloneBrush(other.background_brush.get());
  }

  if (other.border) {
    border.reset(new Border(*other.border));
  }

  if (other.rounded_corners) {
    rounded_corners.reset(new RoundedCorners(*other.rounded_corners));
  }
}

RectNode::Builder::Builder(Moved moved)
    : rect(moved->rect),
      background_brush(moved->background_brush.Pass()),
      border(moved->border.Pass()),
      rounded_corners(moved->rounded_corners.Pass()) {}

bool RectNode::Builder::operator==(const Builder& other) const {
  bool brush_equals = !background_brush && !other.background_brush;
  if (background_brush && other.background_brush) {
    EqualsBrushVisitor brush_equals_visitor(background_brush.get());
    other.background_brush->Accept(&brush_equals_visitor);
    brush_equals = brush_equals_visitor.result();
  }
  bool border_equals = (!border && !other.border) ||
                       (border && other.border && *border == *other.border);
  bool rounded_corners_equals = (!rounded_corners && !other.rounded_corners) ||
                                (rounded_corners && other.rounded_corners &&
                                 *rounded_corners == *other.rounded_corners);

  return rect == other.rect && brush_equals && border_equals &&
         rounded_corners_equals;
}

void RectNode::Accept(NodeVisitor* visitor) { visitor->Visit(this); }

math::RectF RectNode::GetBounds() const { return data_.rect; }

void RectNode::AssertValid() const {
  if (data_.rounded_corners) {
    DCHECK(data_.rounded_corners->IsNormalized(data_.rect));
  }
}

}  // namespace render_tree
}  // namespace cobalt
