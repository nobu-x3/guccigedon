#include <iostream>
#include <vector>
#include "core/logger.h"
#include "render/vulkan/renderer.h"

int main() {
    render::VulkanRenderer renderer {};
    renderer.run();
	return 0;
}
