#include "aiks_example.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "imgui/imgui.h"
#include "impeller/aiks/canvas.h"
#include "impeller/aiks/image.h"
#include "impeller/entity/contents/color_source_contents.h"
#include "impeller/entity/contents/filters/inputs/filter_input.h"
#include "impeller/entity/contents/tiled_texture_contents.h"
#include "impeller/geometry/color.h"
#include "impeller/geometry/constants.h"
#include "impeller/geometry/matrix.h"
#include "impeller/geometry/path_builder.h"
#include "impeller/playground/widgets.h"
#include "impeller/renderer/command_buffer.h"
#include "impeller/renderer/snapshot.h"

namespace example {

ExampleBase::Info AiksExample::GetInfo() {
  return {
    .name = "Aiks Example",
    .description =
      "One of the Aiks unit tests, built and run out-of-tree."
  };
}

bool AiksExample::Setup(std::shared_ptr<impeller::Context> context) {
  // create an aiks context
  aiks_context_ = std::make_shared<impeller::AiksContext>(context);
  return true;
}

bool AiksExample::Render(std::shared_ptr<impeller::Context> context,
                         const impeller::RenderTarget& render_target,
                         impeller::CommandBuffer& command_buffer)
{
  clock_.Tick();

  static impeller::Color color = impeller::Color::Black().WithAlpha(0.5);
  static float scale = 3;
  static bool add_circle_clip = true;

  impeller::Canvas canvas;
  impeller::Paint paint;

  paint.color = impeller::Color::White();
  canvas.DrawPaint(paint);

  paint.color = color;

  impeller::Path path = impeller::PathBuilder{}
                .MoveTo({20, 20})
                .QuadraticCurveTo({60, 20}, {60, 60})
                .Close()
                .MoveTo({60, 20})
                .QuadraticCurveTo({60, 60}, {20, 60})
                .TakePath();

  canvas.Scale(impeller::Vector2(scale, scale));

  for (auto join : {impeller::Join::kBevel, impeller::Join::kRound, impeller::Join::kMiter}) {
    paint.stroke_join = join;
    for (auto cap : {impeller::Cap::kButt, impeller::Cap::kSquare, impeller::Cap::kRound}) {
      paint.stroke_cap = cap;
      canvas.DrawPath(path, paint);
      canvas.Translate({80, 0});
    }
    canvas.Translate({-240, 60});
  }

  impeller::RenderTarget render_target_copy(render_target);

  return aiks_context_->Render(canvas.EndRecordingAsPicture(), render_target_copy);
}

} // namespace example

/*
[&](AiksContext& renderer, RenderTarget& render_target)

static Color color = Color::Black().WithAlpha(0.5);
    static float scale = 3;
    static bool add_circle_clip = true;

    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color));
    ImGui::SliderFloat("Scale", &scale, 0, 6);
    ImGui::Checkbox("Circle clip", &add_circle_clip);
    ImGui::End();

    Canvas canvas;
    canvas.Scale(GetContentScale());
    Paint paint;

    paint.color = Color::White();
    canvas.DrawPaint(paint);

    paint.color = color;
    paint.style = Paint::Style::kStroke;
    paint.stroke_width = 10;

    Path path = PathBuilder{}
                    .MoveTo({20, 20})
                    .QuadraticCurveTo({60, 20}, {60, 60})
                    .Close()
                    .MoveTo({60, 20})
                    .QuadraticCurveTo({60, 60}, {20, 60})
                    .TakePath();

    canvas.Scale(Vector2(scale, scale));

    if (add_circle_clip) {
      auto [handle_a, handle_b] = IMPELLER_PLAYGROUND_LINE(
          Point(60, 300), Point(600, 300), 20, Color::Red(), Color::Red());

      auto screen_to_canvas = canvas.GetCurrentTransformation().Invert();
      Point point_a = screen_to_canvas * handle_a * GetContentScale();
      Point point_b = screen_to_canvas * handle_b * GetContentScale();

      Point middle = (point_a + point_b) / 2;
      auto radius = point_a.GetDistance(middle);
      canvas.ClipPath(PathBuilder{}.AddCircle(middle, radius).TakePath());
    }

    for (auto join : {Join::kBevel, Join::kRound, Join::kMiter}) {
      paint.stroke_join = join;
      for (auto cap : {Cap::kButt, Cap::kSquare, Cap::kRound}) {
        paint.stroke_cap = cap;
        canvas.DrawPath(path, paint);
        canvas.Translate({80, 0});
      }
      canvas.Translate({-240, 60});
    }

return renderer.Render(canvas.EndRecordingAsPicture(), render_target);
*/
