#include <iostream>
#include <vector>
#include "render/vulkan/renderer.h"

int main() {
    render::vulkan::VulkanRenderer renderer {};
    renderer.run();
	return 0;
}
