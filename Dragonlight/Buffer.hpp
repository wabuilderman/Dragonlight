#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class MyVulkan;

class Buffer {
	private:
		// A pointer to the current vulkan environment
		MyVulkan & vulkan;

	public:
		// Determine whether or not the CPU can directly write to the memory
		const bool isWritable = false;

		// Vulkan Buffer Pointer
		VkBuffer buffer;
		// Vulkan Memory Pointer
		VkDeviceMemory memory;

		Buffer(MyVulkan & vulkan, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		~Buffer();

		void writeData(void * srcData, VkDeviceSize size);
		void writeBuffer(VkBuffer src, VkDeviceSize size);
};