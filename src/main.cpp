#include <iostream>
#include <vector>
#include "core/logger.h"
#include "render/renderer.h"

int main() {
    render::VulkanRenderer renderer {};
    renderer.run();
	return 0;
}
