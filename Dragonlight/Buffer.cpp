#include "MyVulkan.hpp"

void Buffer::writeData(void * srcData, VkDeviceSize size) {
	if (!isWritable)
		return;

	void* data;
	vkMapMemory((VkDevice)MyVulkan::device, memory, 0, size, 0, &data);
	memcpy(data, srcData, (size_t) size);
	vkUnmapMemory((VkDevice)MyVulkan::device, memory);
}

void Buffer::writeBuffer(VkBuffer src, VkDeviceSize size) {
	// Create a command buffer to issue commands with
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = vulkan.commandPool->commandPool;
	allocInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers((VkDevice) MyVulkan::device, &allocInfo, &commandBuffer);

	// Begin inputting commands
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	// Issue a command to copy the buffers
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, src, buffer, 1, &copyRegion);

	// Stop inputting commands
	vkEndCommandBuffer(commandBuffer);

	// Execute the command buffer
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit((VkQueue) MyVulkan::graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle((VkQueue) MyVulkan::graphicsQueue);

	// Free the command buffer
	vkFreeCommandBuffers((VkDevice)MyVulkan::device, vulkan.commandPool->commandPool, 1, &commandBuffer);
}

Buffer::Buffer(MyVulkan & vulkan, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) :
	vulkan(
		vulkan
	), 
	isWritable(
		   properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		&& properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer((VkDevice)MyVulkan::device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements((VkDevice)MyVulkan::device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = vulkan.findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory((VkDevice)MyVulkan::device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory((VkDevice)MyVulkan::device, buffer, memory, 0);
}

Buffer::~Buffer() {
	vkDestroyBuffer((VkDevice)MyVulkan::device, buffer, nullptr);
	vkFreeMemory((VkDevice)MyVulkan::device, memory, nullptr);
}