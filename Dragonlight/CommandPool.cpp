#include "CommandPool.hpp"
#include "MyVulkan.hpp"

void CommandPool::create() {
	QueueFamilyIndices queueFamilyIndices = vulkan.findQueueFamilies((VkPhysicalDevice) MyVulkan::physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // Optional

	if (vkCreateCommandPool((VkDevice) MyVulkan::device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

CommandPool::~CommandPool() {
	vkDestroyCommandPool((VkDevice) MyVulkan::device, commandPool, nullptr);
}