#include "mesh_example.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

#include "impeller/geometry/scalar.h"
#include "impeller/renderer/allocator.h"
#include "impeller/renderer/buffer_view.h"
#include "impeller/renderer/command.h"
#include "impeller/renderer/formats.h"
#include "impeller/renderer/pipeline_library.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/render_target.h"

#include "examples/assets.h"

#include "generated/importer/mesh_flatbuffers.h"
#include "generated/shaders/mesh_example.frag.h"
#include "generated/shaders/mesh_example.vert.h"
#include "impeller/renderer/vertex_buffer.h"
#include "impeller/tessellator/tessellator.h"

namespace example {

ExampleBase::Info MeshExample::GetInfo() {
  return {
      .name = "Mesh Example",
      .description =
          "A simple FBX with vertex colors and normals imported ahead of "
          "time. The importer tool is located under `src/examples/importer`.",
  };
}

bool MeshExample::Setup(impeller::Context& context) {
  //----------------------------------------------------------------------------
  /// Load/unpack the model.
  ///

  std::ifstream in;
  const char* filename = "assets/flutter_logo.model";
  in.open(filename, std::ios::binary | std::ios::in);
  in.seekg(0, std::ios::end);
  auto size = in.tellg();
  in.seekg(0, std::ios::beg);

  std::vector<char> data(size);
  in.read(data.data(), size);
  in.close();

  if (!in) {
    std::cerr << "Failed to read file: " << filename << std::endl;
    return false;
  }

  fb::MeshT mesh;
  fb::GetMesh(data.data())->UnPackTo(&mesh);

  //----------------------------------------------------------------------------
  /// Upload vertices/indices to the device.
  ///

  auto vertices_size = sizeof(fb::Vertex) * mesh.vertices.size();
  auto indices_size = sizeof(uint16_t) * mesh.indices.size();
  auto device_buffer = context.GetPermanentsAllocator()->CreateBuffer(
      impeller::StorageMode::kHostVisible, vertices_size + indices_size);

  if (!device_buffer) {
    std::cerr << "Failed to create device buffer." << std::endl;
    return false;
  }

  if (!device_buffer->CopyHostBuffer(
          reinterpret_cast<uint8_t*>(mesh.vertices.data()),
          impeller::Range{0, vertices_size}, 0)) {
    std::cerr << "Failed to upload vertices to device buffer." << std::endl;
    return false;
  }

  if (!device_buffer->CopyHostBuffer(
          reinterpret_cast<uint8_t*>(mesh.indices.data()),
          impeller::Range{0, indices_size}, vertices_size)) {
    std::cerr << "Failed to upload indices to device buffer." << std::endl;
    return false;
  }

  vertex_buffer_ = {
      .vertex_buffer =
          impeller::BufferView{.buffer = device_buffer,
                               .range = impeller::Range{0, vertices_size}},
      .index_buffer =
          impeller::BufferView{
              .buffer = device_buffer,
              .range = impeller::Range{vertices_size, indices_size}},
      .index_count = mesh.indices.size(),
      .index_type = impeller::IndexType::k16bit,
  };

  //----------------------------------------------------------------------------
  /// Build the pipeline.
  ///

  auto pipeline_desc =
      impeller::PipelineBuilder<VS, FS>::MakeDefaultPipelineDescriptor(context);
  pipeline_desc->SetSampleCount(impeller::SampleCount::kCount4);
  pipeline_desc->SetWindingOrder(impeller::WindingOrder::kCounterClockwise);
  pipeline_desc->SetCullMode(impeller::CullMode::kBackFace);
  pipeline_ =
      context.GetPipelineLibrary()->GetRenderPipeline(pipeline_desc).get();
  if (!pipeline_ || !pipeline_->IsValid()) {
    std::cerr << "Failed to initialize pipeline for mesh example.";
    return false;
  }

  return true;
}

bool MeshExample::Render(impeller::Context& context,
                         const impeller::RenderTarget& render_target,
                         impeller::CommandBuffer& command_buffer) {
  clock_.Tick();

  auto pass = command_buffer.CreateRenderPass(render_target);
  if (!pass) {
    return false;
  }

  impeller::Command cmd;
  cmd.label = "Mesh Example";
  cmd.pipeline = pipeline_;

  cmd.BindVertices(vertex_buffer_);

  auto time = clock_.GetTime();
  impeller::Vector3 euler_angles(0.19 * time, 0.7 * time, 0.43 * time);

  VS::VertInfo vs_uniform;
  vs_uniform.mvp =
      impeller::Matrix::MakePerspective(impeller::Degrees{60},
                                        pass->GetRenderTargetSize(), 0, 1000) *
      impeller::Matrix::MakeTranslation({0, 0, -50}) *
      impeller::Matrix::MakeRotationX(impeller::Radians(euler_angles.x)) *
      impeller::Matrix::MakeRotationY(impeller::Radians(euler_angles.y)) *
      impeller::Matrix::MakeRotationZ(impeller::Radians(euler_angles.z));
  VS::BindVertInfo(cmd, pass->GetTransientsBuffer().EmplaceUniform(vs_uniform));

  if (!pass->AddCommand(std::move(cmd))) {
    return false;
  }

  if (!pass->EncodeCommands(context.GetTransientsAllocator())) {
    return false;
  }

  return true;
}

}  // namespace example
