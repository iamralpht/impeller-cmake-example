#pragma once

#include <string>
#include <memory>

#include "impeller/renderer/command_buffer.h"
#include "impeller/renderer/context.h"

namespace example {

class ExampleBase {
 public:
  struct Info {
    const char* name;
    const char* description;
  };

  virtual ~ExampleBase();

  virtual Info GetInfo() = 0;
  virtual bool Setup(std::shared_ptr<impeller::Context> context) = 0;
  virtual bool Render(std::shared_ptr<impeller::Context> context,
                      const impeller::RenderTarget& render_target,
                      impeller::CommandBuffer& command_buffer) = 0;
};

}  // namespace example
