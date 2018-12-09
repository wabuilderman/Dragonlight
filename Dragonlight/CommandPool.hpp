#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
class MyVulkan;

class CommandPool {
private:
	MyVulkan & vulkan;
public:
	VkCommandPool commandPool;

	CommandPool(MyVulkan& vulkan) : vulkan(vulkan) {}
	~CommandPool();

	void create();
};