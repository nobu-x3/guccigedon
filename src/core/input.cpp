#include "core/input.h"
#include <SDL.h>
#include <SDL_mouse.h>
#include <core/types.h>

namespace core {

	MousePosition InputSystem::mouse_position() {
		MousePosition pos{};
		SDL_GetMouseState(&pos.x, &pos.y);
		return pos;
	}

	MouseState InputSystem::mouse_state() {
		MouseState state{};
		int x, y;
		u32 buttons{SDL_GetMouseState(&x, &y)};
		if (buttons & SDL_BUTTON_LMASK) {
			state.LMB = true;
		}
		if (buttons & SDL_BUTTON_RMASK) {
			state.RMB = true;
		}
		return state;
	}

	void InputSystem::set_window_grab(SDL_Window* window, bool value) {
		SDL_SetWindowMouseGrab(window, value ? SDL_TRUE : SDL_FALSE);
	}

	void InputSystem::show_cursor(bool value) {
		SDL_ShowCursor(value ? SDL_ENABLE : SDL_DISABLE);
	}
} // namespace core