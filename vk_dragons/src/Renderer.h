#pragma once
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include <vector>
#include <memory>
#include "MemorySystem.h"

struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Renderer {
public:
	Renderer(GLFWwindow* window, uint32_t width, uint32_t height);
	~Renderer();

	void Render(const std::vector<VkCommandBuffer>& commandBuffers);
	void Resize(uint32_t width, uint32_t height);

	VkCommandBuffer GetSingleUseCommandBuffer();
	void SubmitCommandBuffer(VkCommandBuffer commandBuffer);

	std::unique_ptr<Memory> memory;

	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkExtent2D swapChainExtent;
	VkCommandPool commandPool;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	std::vector<VkImageView> swapChainImageViews;

private:
	GLFWwindow* window;
	uint32_t width;
	uint32_t height;

	VkInstance instance;
	VkQueue graphicsQueue;
	VkSurfaceKHR surface;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	void createInstance();
	bool checkValidationSupport(const std::vector<const char*>& layers);
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void createLogicalDevice();
	void createSurface();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void createImageViews();
	void createSemaphores();
	void createCommandPool();
	void recreateSwapChain();
	void cleanupSwapChain();
};

