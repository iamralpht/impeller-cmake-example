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
  auto time = clock_.GetTime();

  impeller::Canvas canvas;
  impeller::Paint paint;

  paint.color = impeller::Color::White();
  canvas.DrawPaint(paint);

  canvas.Save();
  canvas.Translate({400, 400});

  // Limit drawing to face circle with a clip.
  canvas.ClipPath(impeller::PathBuilder{}.AddCircle(impeller::Point(), 200).TakePath());
  canvas.Save();

  // Cut away eyes/mouth using difference clips.
  canvas.ClipPath(impeller::PathBuilder{}.AddCircle({-100, -50}, 30).TakePath(),
                  impeller::Entity::ClipOperation::kDifference);
  canvas.ClipPath(impeller::PathBuilder{}.AddCircle({100, -50}, 30).TakePath(),
                  impeller::Entity::ClipOperation::kDifference);
  canvas.ClipPath(impeller::PathBuilder{}
                      .AddQuadraticCurve({-100, 50}, {0, 150}, {100, 50})
                      .TakePath(),
                  impeller::Entity::ClipOperation::kDifference);

  // Draw a huge yellow rectangle to prove the clipping works.
  paint.color = impeller::Color::Yellow();
  canvas.DrawRect(impeller::Rect::MakeXYWH(-1000, -1000, 2000, 2000), paint);

  // Remove the difference clips and draw hair that partially covers the eyes.
  canvas.Restore();
  paint.color = impeller::Color::Maroon();
  paint.mask_blur_descriptor = { impeller::FilterContents::BlurStyle::kNormal, impeller::Sigma(12.5f * (std::sin(time * 2.0f) + 1.5f)) };
  canvas.DrawPath(impeller::PathBuilder{}
                      .MoveTo({200, -200})
                      .HorizontalLineTo(-200)
                      .VerticalLineTo(-40)
                      .CubicCurveTo({0, -40}, {0, -80}, {200, -80})
                      .TakePath(),
                  paint);
  
  paint.mask_blur_descriptor.reset();
  canvas.Restore();

  static impeller::Color color = impeller::Color::Black().WithAlpha(0.5);
  static float scale = 3;
  static bool add_circle_clip = true;

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
