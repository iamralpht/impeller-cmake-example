set(IMGUI_IMPELLER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/impeller-cmake/third_party/flutter/impeller/playground/imgui)
add_gles_shader_library(
    NAME imgui
    SHADERS
        ${IMGUI_IMPELLER_DIR}/imgui_raster.vert
        ${IMGUI_IMPELLER_DIR}/imgui_raster.frag
    OUTPUT_DIR ${GENERATED_DIR}/impeller/playground/imgui)

set(IMGUI_SOURCE_FILES
    "third_party/imgui/imgui.cpp"
    "third_party/imgui/imgui_draw.cpp"
    "third_party/imgui/imgui_widgets.cpp"
    "third_party/imgui/imgui_tables.cpp"
    "third_party/imgui/imgui_demo.cpp"
    "third_party/imgui/backends/imgui_impl_glfw.cpp"
    "${IMGUI_IMPELLER_DIR}/imgui_impl_impeller.cc"
    "${GENERATED_DIR}/impeller/playground/imgui/gles/imgui_shaders_gles.cc"
    "${GENERATED_DIR}/impeller/playground/imgui/imgui_raster.vert.cc"
    "${GENERATED_DIR}/impeller/playground/imgui/imgui_raster.frag.cc")

add_library(imgui STATIC ${IMGUI_SOURCE_FILES})

target_include_directories(imgui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # For "third_party/*"
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party # For "imgui/*"
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui # For "imgui.h"
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/impeller-cmake/third_party/flutter # For "impeller/*"
        ${GENERATED_DIR})
target_link_libraries(imgui PUBLIC glfw impeller_renderer)
