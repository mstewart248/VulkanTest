#pragma once
#define GLFW_INCLUDE_VULKAN


#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <stdio.h>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>
#include <time.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>


#include "ApplicationStructs.h"

class Application
{
public:
	Application(GLFWwindow*);
	~Application();

	VkResult InitVulkan();
	VkInstance GetInstance();
	bool PickPhysicalDevice();
	bool CreateLogicalDevice();
	bool CreateSurface();
	bool CreateSwapChain(uint32_t, uint32_t, bool);
	bool CreateImageViews();
	bool CreateDescriptorSetLayout();
	bool CreateGraphicsPipeline();
	bool CreateRenderPass();
	bool CreateFrameBuffers();
	bool CreateCommandPool();
	bool CreateDepthImageResources();
	bool CreateTextureImage(const char*);
	bool CreateTextureImageViews();
	bool CreateTextureSampler();
	bool CreateVertexBuffer();
	bool CreateIndexBuffer();
	bool CreateUniformBuffers();
	bool CreateDescriptorPool();
	bool CreateDescriptorSets();
	bool CreateBuffers(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);
	bool CreateCommandBuffers();
	bool CreateSemaphoresAndFences();
	bool DrawFrame();
	VkDevice GetDevice();
	bool RecreateSwapChain();
	void MatrixTest();
	void VertexTest();
	void FrameResized(int, int);
	GLFWwindow* GetWindow();

private:
	bool DetectVulkan();
	bool CheckValidationSupport();
	VkApplicationInfo CreateAppInfo();
	VkInstanceCreateInfo CreateCreateInfo(VkApplicationInfo);
	bool IsDeviceSuitable(VkPhysicalDevice);
	QueueFamilyIndices FindDeviceQueFamilies(VkPhysicalDevice);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>, bool);
	VkExtent2D ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR&);
	std::vector<char> ReadFile(const std::string&);
	VkShaderModule CreateShaderModule(const std::vector<char>&);
	uint32_t FindMemoryType(uint32_t, VkMemoryPropertyFlags);
	bool CleanupSwapChain();
	int RoundFloat(float);
	void CopyBuffer(VkBuffer, VkBuffer, VkDeviceSize);
	void UpdateUniformBuffer(uint32_t);
	void CreateImage(uint32_t, uint32_t, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImage&, VkDeviceMemory&);
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer);
	void TransitionImageLayout(VkImage, VkFormat, VkImageLayout, VkImageLayout);
	void CopyBufferToImage(VkBuffer, VkImage, uint32_t, uint32_t);
	void UpdateVertexBuffer();
	VkImageView CreateImageView(VkImage, VkFormat, VkImageAspectFlags);
	VkFormat FindSupportedFormat(const std::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags);
	VkFormat FindDepthFormat();
	bool HasStencilComponent(VkFormat);

	GLFWwindow* m_Window;
	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_Device;
	VkSurfaceKHR m_Surface;
	VkQueue m_PresentQue;
	VkQueue m_GraphicsQueue;
	uint32_t m_WindowWidth;
	uint32_t m_WindowHeight;
	VkSwapchainKHR m_SwapChain;
	std::vector<VkImage> m_SwapChainImages;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;
	std::vector<VkImageView> m_SwapChainImageViews;
	VkRenderPass m_RenderPass;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkDescriptorPool m_DescriptorPool;
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeLine;
	VkCommandPool m_CommandPool;
	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;
	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexBufferMemory;		
	VkImage m_TextureImage;
	VkDeviceMemory m_TextureImageMemory;
	VkImageView m_TextureImageView;
	VkImage m_DepthImage;
	VkDeviceMemory m_DepthImageMemory;
	VkImageView m_DepthImageView;
	VkSampler m_TextureSampler;

	std::vector<VkDescriptorSet> m_DescriptionSets;
	std::vector<VkBuffer> m_UniformBuffers;
	std::vector<VkDeviceMemory> m_UniformBufferMemory;
	std::vector<VkCommandBuffer> m_CommandBuffers;
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;
	std::vector<VkSemaphore> m_ImageAvailableSemaphore;
	std::vector<VkSemaphore> m_RenderFinishedSemaphore;
	std::vector<VkFence> m_InFlightFences;

	size_t m_CurrentFrame = 0;
	const int MAX_FRAMES_IN_FLIGHT = 2;
	bool m_FramebufferResized = false;
	bool m_Vsync = true;

	const std::vector<const char*> m_ValidationLayers {
		"VK_LAYER_KHRONOS_validation"
	};
	const std::vector<const char*> m_DeviceExtensions{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	std::vector<Vertex> m_Vertices = {
		{{-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
		{{1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} , {0.0f, 0.0f, 0.0f}},
		{{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} , {0.0f, 0.0f, 0.0f}},
		{{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} , {0.0f, 0.0f, 0.0f}},

		{{-1.0f, -1.0f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
		{{1.0f, -1.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} , {0.0f, 0.0f, 0.0f}},
		{{1.0f, 1.0f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} , {0.0f, 0.0f, 0.0f}},
		{{-1.0f, 1.0f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} , {0.0f, 0.0f, 0.0f}}
	};

	const std::vector<uint16_t> m_Indices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};
	
	
};

