#include <iostream>
#include <vector>
#include "render/vulkan/renderer.h"
#include "core/input.h"

int main() {
   render::vulkan::VulkanRenderer renderer {};
	core::InputSystem::show_cursor(true);
	//core::InputSystem::set_window_grab(renderer.window(), false);
    renderer.run();
	return 0;
}
