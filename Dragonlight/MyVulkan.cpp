#include "MyVulkan.hpp"

void * MyVulkan::device = NULL;
void * MyVulkan::instance = NULL;
void * MyVulkan::callback = NULL;
void * MyVulkan::surface = NULL;
void * MyVulkan::physicalDevice = NULL;
void * MyVulkan::graphicsQueue = NULL;
void * MyVulkan::presentQueue = NULL;

CommandPool * MyVulkan::commandPool = NULL;
std::vector<VkSemaphore> MyVulkan::imageAvailableSemaphores = std::vector<VkSemaphore>();
std::vector<VkSemaphore> MyVulkan::renderFinishedSemaphores = std::vector<VkSemaphore>();
std::vector<VkFence> MyVulkan::inFlightFences = std::vector<VkFence>();
size_t MyVulkan::currentFrame = 0;
bool MyVulkan::framebufferResized = false;
GLFWwindow * MyVulkan::window = NULL;

Buffer * MyVulkan::vertexBuffer = NULL;
SwapChain * MyVulkan::swapChain = NULL;
Renderer * MyVulkan::renderer = NULL;

VkResult MyVulkan::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void MyVulkan::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

MyVulkan::MyVulkan(GLFWwindow * window) {
	initialize();
	this->window = window;

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, 
		[](GLFWwindow* window, int width, int height) {
			auto vulkan = reinterpret_cast<MyVulkan *>(glfwGetWindowUserPointer(window));
			vulkan->framebufferResized = true;
		}
	);

	createInstance();
	setupDebugCallback();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	
	commandPool = new CommandPool(*this);
	commandPool->create();

	createVertexBuffer();
	
	swapChain = new SwapChain(*this);
	renderer = new Renderer(*this);

	swapChain->createImageViews();
	renderer->createRenderPass();
	renderer->createGraphicsPipeline();
	swapChain->createFramebuffers();
	renderer->createCommandBuffers();

	createSyncObjects();
}

MyVulkan::~MyVulkan() {
	vkDeviceWaitIdle((VkDevice) device);
	
	delete renderer;
	delete swapChain;
	delete vertexBuffer;	
	
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore((VkDevice) device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore((VkDevice) device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence((VkDevice) device, inFlightFences[i], nullptr);
	}

	delete commandPool;

	vkDestroyDevice((VkDevice) device, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT((VkInstance) instance, (VkDebugUtilsMessengerEXT) callback, nullptr);
	}

	vkDestroySurfaceKHR((VkInstance)instance, (VkSurfaceKHR) surface, nullptr);
	vkDestroyInstance((VkInstance)instance, nullptr);
}

void MyVulkan::drawFrame() {
	vkWaitForFences((VkDevice) device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR((VkDevice)device, swapChain->swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		swapChain->recreate();
		//return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &renderer->commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences((VkDevice)device, 1, &inFlightFences[currentFrame]);

	if (vkQueueSubmit((VkQueue)graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain->swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR((VkQueue) presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		swapChain->recreate();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}