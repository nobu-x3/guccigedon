set(GUCCIGEDON_TRANSLATION_UNITS src/main.cpp
    src/core/logger.cpp
    src/render/vulkan/renderer.cpp
    src/render/vulkan/builders.cpp
    src/render/vulkan/mesh.cpp
    src/render/vulkan/types.cpp
    src/render/vulkan/image.cpp
    src/render/vulkan/device.cpp
    src/render/vulkan/instance.cpp
    src/render/vulkan/surface.cpp
    src/render/vulkan/swapchain.cpp
    src/render/vulkan/scene.cpp
    src/render/vulkan/pipeline.cpp
    src/render/vulkan/descriptor_allocator.cpp
    src/render/vulkan/shader.cpp
    src/render/vulkan/descriptor_set_builder.cpp
    src/gameplay/camera.cpp
    src/gameplay/transform.cpp
    src/core/input.cpp
    src/gameplay/input_component.cpp
    src/gameplay/movement_component.cpp
    src/assets/textures/texture_importer.cpp
)

set(GUCCIGEDON_TO_TEST
    src/physics/sphere_collider.cpp
)