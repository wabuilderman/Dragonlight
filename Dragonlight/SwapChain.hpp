#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class MyVulkan;

class SwapChain {
private:
	MyVulkan & vulkan;

public:
	std::vector<VkImage> images;
	VkFormat imageFormat;
	VkSwapchainKHR swapChain;
	VkExtent2D extent;
	std::vector<VkImageView> imageViews;
	std::vector<VkFramebuffer> framebuffers;

	void recreate();
	void cleanup();

	SwapChain(MyVulkan& vulkan, VkSwapchainKHR * oldSwapchain = 0);
	~SwapChain();

	void createFramebuffers();
	void createImageViews();
};