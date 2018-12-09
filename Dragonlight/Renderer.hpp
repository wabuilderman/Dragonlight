#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class MyVulkan;

class Renderer {
private:
	MyVulkan & vulkan;

public:
	std::vector<VkCommandBuffer> commandBuffers;
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	Renderer(MyVulkan& vulkan);
	~Renderer();

	void createRenderPass();
	void createGraphicsPipeline();
	void createCommandBuffers();
};
