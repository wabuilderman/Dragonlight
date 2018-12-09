#include "VertexData.hpp"

VkVertexInputBindingDescription VertexData::getBindingDescription() {
	VkVertexInputBindingDescription bindingDescription = {};

	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(VertexData);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> VertexData::getAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(VertexData, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(VertexData, color);

	return attributeDescriptions;
}