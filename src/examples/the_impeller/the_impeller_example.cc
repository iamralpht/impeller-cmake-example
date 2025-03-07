#include "the_impeller_example.h"

#include <filesystem>
#include <iostream>

#include "impeller/renderer/command.h"
#include "impeller/renderer/pipeline_library.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/render_target.h"
#include "impeller/renderer/sampler_library.h"

#include "examples/assets.h"

#include "generated/shaders/impeller.frag.h"
#include "generated/shaders/impeller.vert.h"

namespace example {

ExampleBase::Info TheImpellerExample::GetInfo() {
  return {
      .name = "The Impeller",
      .description =
          "One draw call with a textured quad. The fragment shader renders a "
          "fancy SDF scene with a cubemap backdrop.",
  };
}

bool TheImpellerExample::Setup(impeller::Context& context) {
  const auto fixture_path =
      std::filesystem::current_path() /
      "third_party/impeller-cmake/third_party/flutter/impeller/fixtures/";

  const auto blue_noise_path =
      (fixture_path / "blue_noise.png").generic_string();

  blue_noise_texture_ = example::LoadTexture(blue_noise_path.c_str(),
                                             *context.GetResourceAllocator());
  if (!blue_noise_texture_) {
    std::cerr << "Failed to load blue noise texture." << std::endl;
    return false;
  }
  impeller::SamplerDescriptor noise_sampler_desc;
  noise_sampler_desc.width_address_mode = impeller::SamplerAddressMode::kRepeat;
  noise_sampler_desc.height_address_mode =
      impeller::SamplerAddressMode::kRepeat;
  blue_noise_sampler_ =
      context.GetSamplerLibrary()->GetSampler(noise_sampler_desc);

  cube_map_texture_ =
      example::LoadTextureCube({fixture_path / "table_mountain_px.png",
                                fixture_path / "table_mountain_nx.png",
                                fixture_path / "table_mountain_py.png",
                                fixture_path / "table_mountain_ny.png",
                                fixture_path / "table_mountain_pz.png",
                                fixture_path / "table_mountain_nz.png"},
                               *context.GetResourceAllocator());
  cube_map_sampler_ = context.GetSamplerLibrary()->GetSampler({});

  auto pipeline_desc =
      impeller::PipelineBuilder<VS, FS>::MakeDefaultPipelineDescriptor(context);
  pipeline_desc->SetSampleCount(impeller::SampleCount::kCount4);
  pipeline_ = context.GetPipelineLibrary()->GetPipeline(pipeline_desc).get();
  if (!pipeline_ || !pipeline_->IsValid()) {
    std::cerr << "Failed to initialize pipeline for showcase.";
    return false;
  }

  return true;
}

bool TheImpellerExample::Render(impeller::Context& context,
                                const impeller::RenderTarget& render_target,
                                impeller::CommandBuffer& command_buffer) {
  clock_.Tick();

  auto pass = command_buffer.CreateRenderPass(render_target);
  if (!pass) {
    return false;
  }

  impeller::Command cmd;
  cmd.label = "Impeller SDF showcase";
  cmd.pipeline = pipeline_;

  auto size = render_target.GetRenderTargetSize();

  impeller::VertexBufferBuilder<VS::PerVertexData> builder;
  builder.AddVertices({{impeller::Point()},
                       {impeller::Point(0, size.height)},
                       {impeller::Point(size.width, 0)},
                       {impeller::Point(size.width, 0)},
                       {impeller::Point(0, size.height)},
                       {impeller::Point(size.width, size.height)}});
  cmd.BindVertices(builder.CreateVertexBuffer(pass->GetTransientsBuffer()));

  VS::FrameInfo vs_uniform;
  vs_uniform.mvp = impeller::Matrix::MakeOrthographic(size);
  VS::BindFrameInfo(cmd,
                    pass->GetTransientsBuffer().EmplaceUniform((vs_uniform)));

  FS::FragInfo fs_uniform;
  fs_uniform.texture_size = impeller::Point(size);
  fs_uniform.time = clock_.GetTime();
  FS::BindFragInfo(cmd, pass->GetTransientsBuffer().EmplaceUniform(fs_uniform));
  FS::BindBlueNoise(cmd, blue_noise_texture_, blue_noise_sampler_);
  FS::BindCubeMap(cmd, cube_map_texture_, cube_map_sampler_);

  if (!pass->AddCommand(std::move(cmd))) {
    return false;
  }

  if (!pass->EncodeCommands()) {
    return false;
  }

  return true;
}

}  // namespace example
