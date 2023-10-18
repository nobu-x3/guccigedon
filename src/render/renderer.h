#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "vulkan_types.h"

struct SDL_Window;

namespace render {
	class VulkanRenderer {

	public:
		int _curr_frame{0};
		VkExtent2D _window_extent{800, 600};
		SDL_Window *p_window{nullptr};
		VkInstance _instance;
		VkDebugUtilsMessengerEXT _debug_msger;
		VkPhysicalDevice _physical_device;
		VkDevice _device;
		VkSurfaceKHR _surface;
		VkQueue _graphics_queue;
		uint32_t _graphics_queue_family;
        VkFormat _swapchain_image_format;
        VkSwapchainKHR _swapchain;
        std::vector<VkImage>_swapchain_images;
        std::vector<VkImageView> _swapchain_image_views;
        std::vector<VkFramebuffer> _framebuffers;
        VkCommandPool _command_pool;
        VkCommandBuffer _main_command_buffer;

	private:
		void init_instance();
		void init_swapchain();
        void init_commands();

	public:
		VulkanRenderer();
		~VulkanRenderer();
		void draw();
		void run();
	};
} // namespace render
