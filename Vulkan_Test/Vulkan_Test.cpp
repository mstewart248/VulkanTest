// Vulkan_Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Application.h"
#include <iostream>

Application* CreateWindow(int, int);
void MainLoop(GLFWwindow*, Application*);
void CleanUp(GLFWwindow*, Application*);
void SetupDebugMessenger(Application*);
VkResult CreateDebugUtilsMessengerEXT(VkInstance, const VkDebugUtilsMessengerCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*);
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);
void DestroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*);
static void FramebufferResizeCallback(GLFWwindow*, int, int);
VkDebugUtilsMessengerEXT m_DebugMessenger;
GLFWwindow* applicationWindowPointer;
int RunVulkanStartUp(Application*, bool);

int main()
{
	Application* main = CreateWindow(1280, 720);	
	int startRes = RunVulkanStartUp(main, true);	

	if (startRes == 0) {
		MainLoop(main->GetWindow(), main);
		CleanUp(main->GetWindow(), main);

		return 0;
	}
	else {
		return startRes;
	}
}

int RunVulkanStartUp(Application* main, bool vSync)
{
	if (main->InitVulkan() != VK_SUCCESS) {
		printf("Failed to create Vulkan Instance");
		return -1;
	}

	SetupDebugMessenger(main);

	if (!main->CreateSurface()) {
		printf("Failed to Create Window Surface!");
		return -1;
	}
	if (!main->PickPhysicalDevice()) {
		printf("Failed to Find suitable GPU!");
		return -1;
	}
	if (!main->CreateLogicalDevice()) {
		printf("Failed to Create Logical Device!");
		return -1;
	}

	if (!main->CreateSwapChain(1280, 720, vSync)) {
		printf("Failed to Create Swap Chain!");
		return -1;
	}

	if (!main->CreateImageViews()) {
		printf("Failed to Create Image Views!");
		return -1;
	}

	if (!main->CreateRenderPass()) {
		printf("Create Render Pass Failed!");
		return -1;
	}

	if (!main->CreateDescriptorSetLayout()) {
		printf("Failed to Create Descriptor Set Layout!");
		return -1;
	}

	if (!main->CreateGraphicsPipeline()) {
		printf("Failed to Create Graphics Pipeline!");
		return -1;
	}	

	if (!main->CreateCommandPool()) {
		printf("Failed to Create Command Pool!");
		return -1;
	}

	if (!main->CreateDepthImageResources()) {
		printf("Failed to Create Depth Buffer!");
		return -1;
	}

	if (!main->CreateFrameBuffers()) {
		printf("Failed to Create Framebuffer!");
		return -1;
	}

	if (!main->CreateTextureImage("Textures/Abby Road.jpg")) {
		printf("failed to Create Texture Image");
		return -1;
	}

	if (!main->CreateTextureImageViews()) {
		printf("Failed to Create Texture Image Views!");
		return -1;
	}

	if (!main->CreateTextureSampler()) {
		printf("Failed to Create Texture Sampler!");
		return -1;
	}

	if (!main->CreateVertexBuffer()) {
		printf("Failed to Create Vertex Buffer!");
		return -1;
	}

	if (!main->CreateIndexBuffer()) {
		printf("Failed to Create Index Buffer!");
		return -1;
	}

	if (!main->CreateUniformBuffers()) {
		printf("Failed to Create Uniform Buffers!");
		return -1;
	}

	if (!main->CreateDescriptorPool()) {
		printf("Failed to Create Descriptor Pool!");
		return -1;
	}

	if (!main->CreateDescriptorSets()) {
		printf("Failed to Create Descriptor Sets!");
		return -1;
	}

	if (!main->CreateCommandBuffers()) {
		printf("Failed to Allocate Command Buffers!");
		return -1;
	}

	if (!main->CreateSemaphoresAndFences()) {
		printf("Failed to Create Semaphores!");
		return -1;
	}

	return 0;
}


Application* CreateWindow(int width, int height)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(width, height, "Vulkan Window", nullptr, nullptr);
	Application* app = new Application(window);

	glfwSetWindowUserPointer(window, app);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
	
	return app;
}

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->FrameResized(width, height);
}

void MainLoop(GLFWwindow* window, Application* app)
{
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		app->VertexTest();
		app->DrawFrame();
	}

	vkDeviceWaitIdle(app->GetDevice());
}

void CleanUp(GLFWwindow* window, Application* app)
{
	DestroyDebugUtilsMessengerEXT(app->GetInstance(), m_DebugMessenger, nullptr);
	app->~Application();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void SetupDebugMessenger(Application* app) {
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;

	if (CreateDebugUtilsMessengerEXT(app->GetInstance(), &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}

}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}






