#include "renderer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>
#include "../../vendor/vk-bootstrap/src/VkBootstrap.h"
#include "../core/logger.h"
#include "vulkan_builders.h"

namespace render {

	VulkanRenderer::VulkanRenderer() {
		SDL_Init(SDL_INIT_VIDEO);
		p_window = SDL_CreateWindow("Guccigedon", SDL_WINDOWPOS_CENTERED,
									SDL_WINDOWPOS_CENTERED,
									_window_extent.width, _window_extent.height,
									(SDL_WindowFlags)(SDL_WINDOW_VULKAN));
		if (!p_window) {
			core::Logger::Fatal("Failed to create a window. %s",
								SDL_GetError());
			exit(-1);
		}
		init_instance();
		init_swapchain();
	}

	VulkanRenderer::~VulkanRenderer() { SDL_DestroyWindow(p_window); }

	void VulkanRenderer::draw() {}

	void VulkanRenderer::run() {
		SDL_Event e;
		bool quit = false;
		while (!quit) {
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT)
					quit = true;
			}
			draw();
		}
	}

	void VulkanRenderer::init_instance() {
		// all this vkb stuff is nice and all but eventually we will make
		// something more robust manually. There's a ton of unsafe stuff here.
		vkb::InstanceBuilder builder;
		vkb::Instance vkb_inst = builder.set_app_name("Guccigedon")
									 .request_validation_layers(true)
									 .use_default_debug_messenger()
									 .require_api_version(1, 1, 0)
									 .build()
									 .value();
		_instance = vkb_inst.instance;
		_debug_msger = vkb_inst.debug_messenger;
		SDL_Vulkan_CreateSurface(p_window, _instance, &_surface);
		vkb::PhysicalDeviceSelector selector{vkb_inst};
		vkb::PhysicalDevice vkb_phys_dev = selector.set_minimum_version(1, 1)
											   .set_surface(_surface)
											   .select()
											   .value();
		vkb::DeviceBuilder dev_builder{vkb_phys_dev};
		vkb::Device vkb_dev = dev_builder.build().value();
		_device = vkb_dev.device;
		_physical_device = vkb_phys_dev.physical_device;
		_graphics_queue = vkb_dev.get_queue(vkb::QueueType::graphics).value();
		_graphics_queue_family =
			vkb_dev.get_queue_index(vkb::QueueType::graphics).value();
	}

	void VulkanRenderer::init_swapchain() {
		vkb::SwapchainBuilder swap_buider{_physical_device, _device, _surface};
		vkb::Swapchain vkb_swap =
			swap_buider.use_default_format_selection()
				.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
				.set_desired_extent(_window_extent.width, _window_extent.height)
				.build()
				.value();
		_swapchain = vkb_swap.swapchain;
		_swapchain_images = vkb_swap.get_images().value();
		_swapchain_image_views = vkb_swap.get_image_views().value();
		_swapchain_image_format = vkb_swap.image_format;
	}

	void VulkanRenderer::init_commands() {
        VkCommandPoolCreateInfo command_pool_ci = vkbuild::command_pool_ci(_graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        VK_CHECK(vkCreateCommandPool(_device, &command_pool_ci, nullptr, &_command_pool));
        VkCommandBufferAllocateInfo alloc_info = vkbuild::command_buffer_ai(_command_pool, 1);
        VK_CHECK(vkAllocateCommandBuffers(_device, &alloc_info, &_main_command_buffer));
    }
} // namespace render
