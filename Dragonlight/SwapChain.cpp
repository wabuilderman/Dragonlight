#include "SwapChain.hpp"
#include "MyVulkan.hpp"

SwapChain::SwapChain(MyVulkan& vulkan, VkSwapchainKHR * oldSwapchain) : vulkan(vulkan) {
	SwapChainSupportDetails swapChainSupport = vulkan.querySwapChainSupport((VkPhysicalDevice) MyVulkan::physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = vulkan.chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = vulkan.chooseSwapPresentMode(swapChainSupport.presentModes);
	extent = vulkan.chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = (VkSurfaceKHR) MyVulkan::surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = vulkan.findQueueFamilies((VkPhysicalDevice) MyVulkan::physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = oldSwapchain ? *oldSwapchain : VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR((VkDevice)MyVulkan::device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR((VkDevice)MyVulkan::device, swapChain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR((VkDevice)MyVulkan::device, swapChain, &imageCount, images.data());

	imageFormat = surfaceFormat.format;

	
}

void SwapChain::createFramebuffers() {
	framebuffers.resize(imageViews.size());

	for (size_t i = 0; i < imageViews.size(); i++) {
		VkImageView attachments[] = {
			imageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = vulkan.renderer->renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer((VkDevice)MyVulkan::device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void SwapChain::createImageViews() {
	imageViews.resize(images.size());
	for (size_t i = 0; i < images.size(); i++) {

		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = images[i];

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = imageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView((VkDevice)MyVulkan::device, &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void SwapChain::recreate() {
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(vulkan.window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle((VkDevice) MyVulkan::device);

	cleanup();
	VkSwapchainKHR oldSwapchain = swapChain;

	new(this) SwapChain(vulkan, &oldSwapchain);
	createImageViews();
	vulkan.renderer->createRenderPass();
	vulkan.renderer->createGraphicsPipeline();
	createFramebuffers();

	vkDestroySwapchainKHR((VkDevice)MyVulkan::device, oldSwapchain, nullptr);
}

void SwapChain::cleanup() {
	for (size_t i = 0; i < framebuffers.size(); i++) {
		vkDestroyFramebuffer((VkDevice)MyVulkan::device, framebuffers[i], nullptr);
	}

	vkFreeCommandBuffers(
		(VkDevice)MyVulkan::device,
		vulkan.commandPool->commandPool,
		static_cast<uint32_t>(vulkan.renderer->commandBuffers.size()),
		vulkan.renderer->commandBuffers.data()
	);

	vkDestroyPipeline((VkDevice)MyVulkan::device, vulkan.renderer->graphicsPipeline, nullptr);
	vkDestroyPipelineLayout((VkDevice)MyVulkan::device, vulkan.renderer->pipelineLayout, nullptr);
	vkDestroyRenderPass((VkDevice)MyVulkan::device, vulkan.renderer->renderPass, nullptr);

	for (size_t i = 0; i < imageViews.size(); i++) {
		vkDestroyImageView((VkDevice)MyVulkan::device, imageViews[i], nullptr);
	}
}


SwapChain::~SwapChain() {
	cleanup();
	vkDestroySwapchainKHR((VkDevice)MyVulkan::device, swapChain, nullptr);
}