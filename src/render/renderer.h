#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "vulkan_types.h"
#include "../core/core.h"

struct SDL_Window;

namespace render {
	class VulkanRenderer {

	public:
		int _curr_frame{0};
		VkExtent2D _window_extent{800, 600};
		SDL_Window *_p_window{nullptr};
		VkInstance _instance;
		VkDebugUtilsMessengerEXT _debug_msger;
		VkPhysicalDevice _physical_device;
		VkDevice _device;
		VkSurfaceKHR _surface;
		VkQueue _graphics_queue;
		u32 _graphics_queue_family;
        VkFormat _swapchain_image_format;
        VkSwapchainKHR _swapchain;
        std::vector<VkImage>_swapchain_images;
        std::vector<VkImageView> _swapchain_image_views;
        std::vector<VkFramebuffer> _framebuffers;
        VkCommandPool _command_pool;
        VkCommandBuffer _main_command_buffer;
        VkSemaphore _present_semaphore, _render_semaphore;
        VkFence _render_fence;
        VkRenderPass _render_pass;

	private:
		void init_instance();
		void init_swapchain();
        void init_commands();
        void init_framebuffers();
        void init_default_renderpass();
        void init_sync_objects();

	public:
		VulkanRenderer();
		~VulkanRenderer();
		void draw();
		void run();
	};
} // namespace render
