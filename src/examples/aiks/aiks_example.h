#pragma once

#include "examples/example_base.h"

#include <memory>
#include "examples/clock.h"

#include "impeller/renderer/pipeline.h"
#include "impeller/renderer/pipeline_descriptor.h"
#include "impeller/renderer/sampler.h"
#include "impeller/renderer/vertex_buffer.h"
#include "impeller/aiks/aiks_context.h"

namespace example {

class AiksExample final : public ExampleBase {
 public:
  Info GetInfo() override;
  bool Setup(std::shared_ptr<impeller::Context> context) override;
  bool Render(std::shared_ptr<impeller::Context> context,
              const impeller::RenderTarget& render_target,
              impeller::CommandBuffer& command_buffer) override;
 private:
  example::Clock clock_;
  std::shared_ptr<impeller::AiksContext> aiks_context_;
};

} // namespace example
