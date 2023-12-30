#include <iostream>
#include <vector>
#include "core/input.h"
#include "core/sapfire_engine.h"

int main() {
	core::Engine engine{};
	engine.load_scene("assets/scenes/physics_test/physics_test.gltf");
	core::InputSystem::show_cursor(true);
	// core::InputSystem::set_window_grab(renderer.window(), false);
	engine.run();
	return 0;
}
