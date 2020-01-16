
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Application.h"



Application::Application(GLFWwindow* window)
{
	this->m_Window = window;

	if (this->DetectVulkan()) {
		if (!CheckValidationSupport()) {
			throw std::runtime_error("Validation layers are not available");
		}
	}
	else {
		printf("Vulkan Not Installed on Machine");
	}
}

Application::~Application()
{
	this->CleanupSwapChain();
	
	vkDestroySampler(this->m_Device, this->m_TextureSampler, nullptr);
	vkDestroyImageView(this->m_Device, this->m_TextureImageView, nullptr);

	vkDestroyImage(this->m_Device, this->m_TextureImage, nullptr);
	vkFreeMemory(this->m_Device, this->m_TextureImageMemory, nullptr);

	vkDestroyDescriptorSetLayout(this->m_Device, this->m_DescriptorSetLayout, nullptr);

	vkDestroyBuffer(this->m_Device, this->m_IndexBuffer, nullptr);
	vkFreeMemory(this->m_Device, this->m_IndexBufferMemory, nullptr);

	vkDestroyBuffer(this->m_Device, this->m_VertexBuffer, nullptr);
	vkFreeMemory(this->m_Device, this->m_VertexBufferMemory, nullptr);

	for (size_t i = 0; i < this->MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(this->m_Device, this->m_RenderFinishedSemaphore[i], nullptr);
		vkDestroySemaphore(this->m_Device, this->m_ImageAvailableSemaphore[i], nullptr);
		vkDestroyFence(this->m_Device, this->m_InFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(this->m_Device, this->m_CommandPool, nullptr);
	vkDestroyDevice(this->m_Device, nullptr);
	vkDestroySurfaceKHR(this->m_Instance, this->m_Surface, nullptr);
	vkDestroyInstance(this->m_Instance, nullptr);	
}

bool Application::CleanupSwapChain() {

	vkDestroyImageView(this->m_Device, this->m_DepthImageView, nullptr);
	vkDestroyImage(this->m_Device, this->m_DepthImage, nullptr);
	vkFreeMemory(this->m_Device, this->m_DepthImageMemory, nullptr);

	for (auto framebuffer : this->m_SwapChainFramebuffers) {
		vkDestroyFramebuffer(this->m_Device, framebuffer, nullptr);
	}

	vkFreeCommandBuffers(this->m_Device, this->m_CommandPool, static_cast<uint32_t>(this->m_CommandBuffers.size()), this->m_CommandBuffers.data());
	vkDestroyPipeline(this->m_Device, this->m_GraphicsPipeLine, nullptr);
	vkDestroyPipelineLayout(this->m_Device, this->m_PipelineLayout, nullptr);
	vkDestroyRenderPass(this->m_Device, this->m_RenderPass, nullptr);

	for (auto view : this->m_SwapChainImageViews) {
		vkDestroyImageView(this->m_Device, view, nullptr);
	}

	vkDestroySwapchainKHR(this->m_Device, this->m_SwapChain, nullptr);

	for (size_t i = 0; i < this->m_SwapChainImages.size(); i++) {
		vkDestroyBuffer(this->m_Device, this->m_UniformBuffers[i], nullptr);
		vkFreeMemory(this->m_Device, this->m_UniformBufferMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(this->m_Device, this->m_DescriptorPool, nullptr);

	return true;
}

bool Application::DetectVulkan() {
	uint32_t extensionCount = 0;

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	printf("Vulkan Extenisons: %d\n", extensionCount);
	printf("Extensions Available\n");
	printf("--------------------\n");

	for (const auto& ext : extensions) {
		printf("%s\n", (const char*)ext.extensionName);
	}

	printf("\n");

	return extensionCount > 0;
}

bool Application::CheckValidationSupport() {
	uint32_t layerCount = 0;

	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> layers(layerCount);

	vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
	printf("Vulkan Layers: %d\n", layerCount);
	printf("Layers Available\n");
	printf("----------------\n");

	for (const auto& layer : layers) {
		printf("%s\n", (const char*)layer.layerName);
	}

	printf("\n");

	for (const char* layerName : m_ValidationLayers) {
		bool layerFound = false;

		for (const auto& prop : layers) {
			if (strcmp(layerName, prop.layerName) == 0) {
				return true;
			}
		}
	}

	return false;
}

VkResult Application::InitVulkan() {
	const char** glfwExtensions;
	uint32_t glfwExtensionsCount = 0;
	VkApplicationInfo appInfo = this->CreateAppInfo();
	VkInstanceCreateInfo createInfo = this->CreateCreateInfo(appInfo);

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

	std::vector<const char*> reqExtensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

	reqExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	createInfo.enabledExtensionCount = static_cast<uint32_t>(reqExtensions.size());
	createInfo.ppEnabledExtensionNames = reqExtensions.data();
	createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
	createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
	
	return vkCreateInstance(&createInfo, nullptr, &m_Instance);
}

VkApplicationInfo Application::CreateAppInfo() {
	VkApplicationInfo appInfo = {};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Render";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.pEngineName = "NO ENGINE";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	return appInfo;
}

VkInstanceCreateInfo Application::CreateCreateInfo(VkApplicationInfo appInfo) {
	VkInstanceCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	return createInfo;
}

VkInstance Application::GetInstance() {
	return this->m_Instance;
}

bool Application::PickPhysicalDevice() {
	uint32_t deviceCount = 0;

	vkEnumeratePhysicalDevices(this->m_Instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		std::runtime_error("Failed to find a GPU with Vulkan Support");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);

	vkEnumeratePhysicalDevices(this->m_Instance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (IsDeviceSuitable(device)) {
			this->m_PhysicalDevice = device;
			break;
		}
	}

	if (this->m_PhysicalDevice == VK_NULL_HANDLE) {
		return false;
	}
	else {
		return true;
	}
}

bool Application::IsDeviceSuitable(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	bool swapChainAdequate = false;

	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	if (!(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader)) {
		return false;
	}

	bool supportedExtensions = CheckDeviceExtensionSupport(device);

	if (supportedExtensions) {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return FindDeviceQueFamilies(device).isComplete() && supportedExtensions && swapChainAdequate && deviceFeatures.samplerAnisotropy;

}

bool Application::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(this->m_DeviceExtensions.begin(), this->m_DeviceExtensions.end());
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

QueueFamilyIndices Application::FindDeviceQueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->m_Surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

bool Application::CreateLogicalDevice() {
	QueueFamilyIndices indices = FindDeviceQueFamilies(this->m_PhysicalDevice); 

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(this->m_DeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = this->m_DeviceExtensions.data();


	createInfo.enabledLayerCount = static_cast<uint32_t>(this->m_ValidationLayers.size());
	createInfo.ppEnabledLayerNames = this->m_ValidationLayers.data();
	

	if (vkCreateDevice(this->m_PhysicalDevice, &createInfo, nullptr, &this->m_Device) != VK_SUCCESS) {
		return false;
	}

	vkGetDeviceQueue(this->m_Device, indices.graphicsFamily.value(), 0, &this->m_GraphicsQueue);
	vkGetDeviceQueue(this->m_Device, indices.presentFamily.value(), 0, &this->m_PresentQue);

	return true;
}

bool Application::CreateSurface() {
	if (glfwCreateWindowSurface(this->m_Instance, this->m_Window, nullptr, &this->m_Surface) != VK_SUCCESS) {
		return false;
	}

	return true;
}

SwapChainSupportDetails Application::QuerySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;
	uint32_t formatCount;
	uint32_t presentModeCount;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->m_Surface, &details.capabilities);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->m_Surface, &formatCount, nullptr);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->m_Surface, &presentModeCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->m_Surface, &formatCount, details.formats.data());
	}

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->m_Surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

bool Application::CreateSwapChain(uint32_t width, uint32_t height, bool vsync) {
	SwapChainSupportDetails scDetails = QuerySwapChainSupport(this->m_PhysicalDevice);
	uint32_t imageCount = scDetails.capabilities.minImageCount + 1;

	this->m_WindowWidth = width;
	this->m_WindowHeight = height;	
	this->m_Vsync = vsync;

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(scDetails.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(scDetails.presentModes, vsync);
	VkExtent2D extent = ChooseSwapChainExtent(scDetails.capabilities);

	if (scDetails.capabilities.maxImageCount > 0 && imageCount > scDetails.capabilities.maxImageCount) {
		imageCount = scDetails.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = this->m_Surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindDeviceQueFamilies(this->m_PhysicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	
	createInfo.preTransform = scDetails.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(this->m_Device, &createInfo, nullptr, &this->m_SwapChain) != VK_SUCCESS) {
		return false;
	}

	vkGetSwapchainImagesKHR(this->m_Device, this->m_SwapChain, &imageCount, nullptr);
	this->m_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(this->m_Device, this->m_SwapChain, &imageCount, this->m_SwapChainImages.data());
	this->m_SwapChainImageFormat = surfaceFormat.format;
	this->m_SwapChainExtent = extent;


	return true;
}

VkPresentModeKHR Application::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes, bool vsync) {
	if (vsync) {
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& format : availableFormats) {
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	return availableFormats[0];
}

VkExtent2D Application::ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	} 
	else {
		int width, height;

		glfwGetFramebufferSize(this->m_Window, &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

bool Application::CreateImageViews() {
	this->m_SwapChainImageViews.resize(this->m_SwapChainImages.size());

	for (size_t i = 0; i < this->m_SwapChainImages.size(); i++) {
		this->m_SwapChainImageViews[i] = CreateImageView(this->m_SwapChainImages[i], this->m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	return true;
}

bool Application::CreateDescriptorSetLayout() {

	VkDescriptorSetLayoutBinding uboLayoutBinding = {};

	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};

	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(this->m_Device, &layoutInfo, nullptr, &this->m_DescriptorSetLayout) != VK_SUCCESS) {
		return false;
	}

	return true;
}

bool Application::CreateGraphicsPipeline() {
	auto vertShaderCode = ReadFile("shaders/vert.spv");
	auto fragShaderCode = ReadFile("shaders/frag.spv");
	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);
	VkPipelineShaderStageCreateInfo vi = {};
	VkPipelineShaderStageCreateInfo fi = {};
	VkPipelineVertexInputStateCreateInfo vInputInfo = {};
	VkPipelineInputAssemblyStateCreateInfo inputAsm = {};
	VkViewport viewport = {};
	VkRect2D scissor = {};
	VkPipelineViewportStateCreateInfo viewportState = {};
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	vi.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vi.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vi.module = vertShaderModule;
	vi.pName = "main";

	fi.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fi.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fi.module = fragShaderModule;
	fi.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vi, fi };
	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescription = Vertex::getAttributeDescriptions();



	vInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vInputInfo.vertexBindingDescriptionCount = 1;
	vInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

	inputAsm.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAsm.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAsm.primitiveRestartEnable = VK_FALSE;

	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)this->m_SwapChainExtent.width;
	viewport.height = (float)this->m_SwapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset = { 0, 0 };
	scissor.extent = this->m_SwapChainExtent;

	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; 
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f; 

	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; 
	multisampling.pSampleMask = nullptr; 
	multisampling.alphaToCoverageEnable = VK_FALSE; 
	multisampling.alphaToOneEnable = VK_FALSE; 

	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; 
	pipelineLayoutInfo.pSetLayouts = &this->m_DescriptorSetLayout; 
	pipelineLayoutInfo.pushConstantRangeCount = 0; 
	pipelineLayoutInfo.pPushConstantRanges = nullptr; 

	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};

	if (vkCreatePipelineLayout(this->m_Device, &pipelineLayoutInfo, nullptr, &this->m_PipelineLayout) != VK_SUCCESS) {
		return false;
	}

	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAsm;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; //&dynamicState; // Optional
	pipelineInfo.layout = this->m_PipelineLayout;
	pipelineInfo.renderPass = this->m_RenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(this->m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->m_GraphicsPipeLine) != VK_SUCCESS) {
		return false;
	}

	vkDestroyShaderModule(this->m_Device, fragShaderModule, nullptr);
	vkDestroyShaderModule(this->m_Device, vertShaderModule, nullptr);

	return true;
}

bool Application::CreateFrameBuffers() {
	this->m_SwapChainFramebuffers.resize(this->m_SwapChainImageViews.size());

	for (size_t i = 0; i < this->m_SwapChainImageViews.size(); i++) {
		std::array<VkImageView, 2> attachments{
			this->m_SwapChainImageViews[i],
			this->m_DepthImageView
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = this->m_RenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = this->m_SwapChainExtent.width; 
		framebufferInfo.height = this->m_SwapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(this->m_Device, &framebufferInfo, nullptr, &this->m_SwapChainFramebuffers[i]) != VK_SUCCESS) {
			return false;
		}
	}

	return true;
}

bool Application::CreateCommandPool() {
	QueueFamilyIndices indices = FindDeviceQueFamilies(this->m_PhysicalDevice);
	VkCommandPoolCreateInfo poolInfo = {};

	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = indices.graphicsFamily.value();
	poolInfo.flags = 0; // Optional

	if (vkCreateCommandPool(this->m_Device, &poolInfo, nullptr, &this->m_CommandPool) != VK_SUCCESS) {
		return false;
	}

	return true;
}

bool Application::CreateDescriptorSets() {
	std::vector<VkDescriptorSetLayout> layouts(this->m_SwapChainImages.size(), this->m_DescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};

	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = this->m_DescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(this->m_SwapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();
	this->m_DescriptionSets.resize(this->m_SwapChainImages.size());

	if (vkAllocateDescriptorSets(this->m_Device, &allocInfo, this->m_DescriptionSets.data()) != VK_SUCCESS) {
		return false;
	}

	for (size_t i = 0; i < this->m_SwapChainImages.size(); i++) {
		VkDescriptorBufferInfo bufferInfo = {};

		bufferInfo.buffer = this->m_UniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo = {};

		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = this->m_TextureImageView;
		imageInfo.sampler = this->m_TextureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = this->m_DescriptionSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = this->m_DescriptionSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;


		vkUpdateDescriptorSets(this->m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	return true;
}

bool Application::CreateRenderPass() {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = this->m_SwapChainImageFormat; // swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(this->m_Device, &renderPassInfo, nullptr, &this->m_RenderPass) != VK_SUCCESS) {
		return false;
	}

	return true;
}

bool Application::CreateTextureImage(const char* fileName) {
	int texWidth;
	int texHeight;
	int texChannels;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	stbi_uc* pixels = stbi_load(fileName, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	void* data;

	if (!pixels) {
		return false;
	}

	CreateBuffers(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);


	vkMapMemory(this->m_Device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(this->m_Device, stagingBufferMemory);
	stbi_image_free(pixels);
	CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->m_TextureImage, this->m_TextureImageMemory);

	TransitionImageLayout(this->m_TextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage(stagingBuffer, this->m_TextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	TransitionImageLayout(this->m_TextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(this->m_Device, stagingBuffer, nullptr);
	vkFreeMemory(this->m_Device, stagingBufferMemory, nullptr);

	return true;
}

void Application::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(this->m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(this->m_Device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(this->m_Device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(this->m_Device, image, imageMemory, 0);
}

bool Application::CreateTextureImageViews() {
	this->m_TextureImageView = CreateImageView(this->m_TextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);


	return true;
}

VkImageView Application::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
	VkImageViewCreateInfo viewInfo = {};

	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;

	if (vkCreateImageView(this->m_Device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to Create Texture Image View!");
	}

	return imageView;
}

bool Application::CreateTextureSampler() {
	VkSamplerCreateInfo samplerInfo = {};

	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(this->m_Device, &samplerInfo, nullptr, &this->m_TextureSampler) != VK_SUCCESS) {
		return false;
	}

	return true;
}

bool Application::CreateVertexBuffer() {
	VkDeviceSize bufferSize = sizeof(this->m_Vertices[0]) * this->m_Vertices.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	void* data;

	CreateBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	vkMapMemory(this->m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, this->m_Vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(this->m_Device, stagingBufferMemory);

	if (this->m_VertexBufferMemory != NULL) {
		vkDeviceWaitIdle(this->m_Device);
		vkFreeCommandBuffers(this->m_Device, this->m_CommandPool, static_cast<uint32_t>(this->m_CommandBuffers.size()), this->m_CommandBuffers.data());
		vkDestroyBuffer(this->m_Device, this->m_VertexBuffer, nullptr);
		vkFreeMemory(this->m_Device, this->m_VertexBufferMemory, nullptr);
	}

	CreateBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->m_VertexBuffer, this->m_VertexBufferMemory);

	CopyBuffer(stagingBuffer, this->m_VertexBuffer, bufferSize);
	vkDestroyBuffer(this->m_Device, stagingBuffer, nullptr);
	vkFreeMemory(this->m_Device, stagingBufferMemory, nullptr);

	return true;
}

bool Application::CreateIndexBuffer() {
	VkDeviceSize bufferSize = sizeof(this->m_Indices[0]) * this->m_Indices.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	CreateBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(this->m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, this->m_Indices.data(), (size_t)bufferSize);
	vkUnmapMemory(this->m_Device, stagingBufferMemory);

	CreateBuffers(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->m_IndexBuffer, this->m_IndexBufferMemory);
	CopyBuffer(stagingBuffer, this->m_IndexBuffer, bufferSize);

	vkDestroyBuffer(this->m_Device, stagingBuffer, nullptr);
	vkFreeMemory(this->m_Device, stagingBufferMemory, nullptr);

	return true;
}

bool Application::CreateUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	this->m_UniformBuffers.resize(this->m_SwapChainImages.size());
	this->m_UniformBufferMemory.resize(this->m_SwapChainImages.size());

	for (size_t i = 0; i < this->m_SwapChainImages.size(); i++) {
		CreateBuffers(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, this->m_UniformBuffers[i], this->m_UniformBufferMemory[i]);
	}

	return true;
}

bool Application::CreateDescriptorPool() {
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(this->m_SwapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(this->m_SwapChainImages.size());


	VkDescriptorPoolCreateInfo poolInfo = {};

	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(this->m_SwapChainImages.size());

	if (vkCreateDescriptorPool(this->m_Device, &poolInfo, nullptr, &this->m_DescriptorPool) != VK_SUCCESS) {
		return false;
	}

	return true;
}

bool Application::CreateBuffers(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	VkBufferCreateInfo bufferInfo = {};

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(this->m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		return false;
	}

	VkMemoryRequirements memRequirements;

	vkGetBufferMemoryRequirements(this->m_Device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};

	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(this->m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		return false;
	}

	vkBindBufferMemory(this->m_Device, buffer, bufferMemory, 0);

	return true;
}

void Application::CopyBuffer(VkBuffer srcBuffer, VkBuffer destBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	VkBufferCopy copyRegion = {};

	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, destBuffer, 1, &copyRegion);

	EndSingleTimeCommands(commandBuffer);
}

VkCommandBuffer Application::BeginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = this->m_CommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(this->m_Device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Application::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(this->m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(this->m_GraphicsQueue);

	vkFreeCommandBuffers(this->m_Device, this->m_CommandPool, 1, &commandBuffer);
}

void Application::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	VkImageMemoryBarrier barrier = {};

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (HasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0; //TODO
	barrier.dstAccessMask = 0; //TODO

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else {
		throw std::invalid_argument("Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	EndSingleTimeCommands(commandBuffer);
}

void Application::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferImageCopy region = {};

	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};


	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


	EndSingleTimeCommands(commandBuffer);
}

uint32_t Application::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;

	vkGetPhysicalDeviceMemoryProperties(this->m_PhysicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

bool Application::CreateCommandBuffers() {
	VkCommandBufferAllocateInfo allocInfo = {};
	std::array<VkClearValue, 2> clearValues = {};

	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	this->m_CommandBuffers.resize(this->m_SwapChainFramebuffers.size());
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = this->m_CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)this->m_CommandBuffers.size();

	if (vkAllocateCommandBuffers(this->m_Device, &allocInfo, this->m_CommandBuffers.data()) != VK_SUCCESS) {
		return false;
	}

	for (size_t i = 0; i < this->m_CommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(this->m_CommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = this->m_RenderPass;
		renderPassInfo.framebuffer = this->m_SwapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = this->m_SwapChainExtent;


		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(this->m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(this->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->m_GraphicsPipeLine);
		
		VkBuffer vertexBuffers[] = { this->m_VertexBuffer };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(this->m_CommandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(this->m_CommandBuffers[i], this->m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(this->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->m_PipelineLayout, 0, 1, &this->m_DescriptionSets[i], 0, nullptr);
		vkCmdDrawIndexed(this->m_CommandBuffers[i], static_cast<uint32_t>(this->m_Indices.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(this->m_CommandBuffers[i]);

		if (vkEndCommandBuffer(this->m_CommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Faild to record command buffer!");
		}
	}

	return true;
}

bool Application::CreateSemaphoresAndFences() {
	VkSemaphoreCreateInfo semaphoreInfo = {};
	VkFenceCreateInfo fenceInfo = {};

	this->m_ImageAvailableSemaphore.resize(this->MAX_FRAMES_IN_FLIGHT);
	this->m_RenderFinishedSemaphore.resize(this->MAX_FRAMES_IN_FLIGHT);		
	this->m_InFlightFences.resize(this->MAX_FRAMES_IN_FLIGHT);

	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < this->MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(this->m_Device, &semaphoreInfo, nullptr, &this->m_ImageAvailableSemaphore[i]) != VK_SUCCESS) {
			return false;
		}
		if (vkCreateSemaphore(this->m_Device, &semaphoreInfo, nullptr, &this->m_RenderFinishedSemaphore[i]) != VK_SUCCESS) {
			return false;
		}
		if (vkCreateFence(this->m_Device, &fenceInfo, nullptr, &this->m_InFlightFences[i]) != VK_SUCCESS) {
			return false;
		}
	}

	

	return true;
}

bool Application::RecreateSwapChain() {
	int width = 0;
	int height = 0;

	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(this->m_Window, &width, &height);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(this->m_Device);

	this->CleanupSwapChain();

	this->CreateSwapChain(this->m_WindowWidth, this->m_WindowHeight, this->m_Vsync);
	this->CreateImageViews();
	this->CreateRenderPass();
	this->CreateGraphicsPipeline();
	this->CreateDepthImageResources();
	this->CreateFrameBuffers();
	this->CreateUniformBuffers();
	this->CreateDescriptorPool();
	this->CreateDescriptorSets();
	this->CreateCommandBuffers();

	return true;
}

bool Application::DrawFrame() {
	uint32_t imageIndex;		

	vkWaitForFences(this->m_Device, 1, &this->m_InFlightFences[this->m_CurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	
	VkResult res = vkAcquireNextImageKHR(this->m_Device, this->m_SwapChain, std::numeric_limits<uint64_t>::max(), this->m_ImageAvailableSemaphore[this->m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		this->RecreateSwapChain();
		return true;
	}
	else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
		return false;
	}

	UpdateUniformBuffer(imageIndex);
	//UpdateVertexBuffer();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { this->m_ImageAvailableSemaphore[this->m_CurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->m_CommandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { this->m_RenderFinishedSemaphore[this->m_CurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(this->m_Device, 1, &this->m_InFlightFences[this->m_CurrentFrame]);

	if (vkQueueSubmit(this->m_GraphicsQueue, 1, &submitInfo, this->m_InFlightFences[this->m_CurrentFrame]) != VK_SUCCESS) {
		return false;
	}
	   	  
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { this->m_SwapChain };

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	res = vkQueuePresentKHR(this->m_PresentQue, &presentInfo);
	
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || this->m_FramebufferResized) {
		this->m_FramebufferResized = false;
		this->RecreateSwapChain();
	}
	else if (res != VK_SUCCESS) {
		return false;
	}

	this->m_CurrentFrame = (this->m_CurrentFrame + 1) % this->MAX_FRAMES_IN_FLIGHT;

	return true;
}

void Application::UpdateUniformBuffer(uint32_t currentImage) {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo = {};

	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), this->m_SwapChainExtent.width / (float)this->m_SwapChainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(this->m_Device, this->m_UniformBufferMemory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(this->m_Device, this->m_UniformBufferMemory[currentImage]);
}

void Application::UpdateVertexBuffer() {
	VertexTest();

	void* data;
	vkMapMemory(this->m_Device, this->m_VertexBufferMemory, 0, sizeof(this->m_Vertices), 0, &data);
	memcpy(data, &this->m_Vertices, sizeof(this->m_Vertices));
	vkUnmapMemory(this->m_Device, this->m_VertexBufferMemory);
}

VkDevice Application::GetDevice() {
	return this->m_Device;
}

std::vector<char> Application::ReadFile(const std::string& fileName) {
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to load file: " + fileName);
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

VkShaderModule Application::CreateShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo ci = {};
	VkShaderModule sm;

	ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ci.codeSize = code.size();
	ci.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(this->m_Device, &ci, nullptr, &sm) != VK_SUCCESS) {
		throw std::runtime_error("Failed to Create Shader Module!");
	}

	return sm;
}

bool Application::CreateDepthImageResources() {
	VkFormat depthFormat = FindDepthFormat();

	CreateImage(this->m_SwapChainExtent.width, this->m_SwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->m_DepthImage, this->m_DepthImageMemory);
	this->m_DepthImageView = CreateImageView(this->m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	TransitionImageLayout(this->m_DepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);


	return true;
}

VkFormat Application::FindSupportedFormat(const std::vector<VkFormat>& canidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	
	for (VkFormat format : canidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(this->m_PhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("Failed to find supported format!");
}

VkFormat Application::FindDepthFormat() {
	return FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool Application::HasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


void Application::FrameResized(int width, int height) {
	this->m_FramebufferResized = true;
	this->m_WindowWidth = width;
	this->m_WindowHeight = height;
}

GLFWwindow* Application::GetWindow() {
	return this->m_Window;
}


void Application::VertexTest() {

	static float destR = 0;
	static float destG = 0;
	static float destB = 0;
	bool rMatched = false;
	bool gMatched = false;
	bool bMatched = false;
	static uint64_t timeNow = 0;


	if (timeNow == 0) {
		timeNow = time(NULL);
		std::srand(timeNow);
	}
	

	for (auto& vert : this->m_Vertices) {

	 if (vert.destColor[0] == 0 && vert.destColor[1] == 0 && vert.destColor[2] == 0) {
			bool pointFound = false;

			for (auto sVerts : this->m_Vertices) {
				if (sVerts.pos == vert.pos && sVerts.destColor != vert.destColor) {
			
					if (sVerts.destColor[0] != 0 && sVerts.destColor[1] != 0 && sVerts.destColor[2] != 0) {
						vert.destColor = sVerts.destColor;
						pointFound = true;
						break;
					}
					
				}
			}

			if (!pointFound) {
				vert.destColor[0] = (float)RoundFloat(((float)std::rand() / (RAND_MAX))) / 100;
				vert.destColor[1] = (float)RoundFloat(((float)std::rand() / (RAND_MAX))) / 100;
				vert.destColor[2] = (float)RoundFloat(((float)std::rand() / (RAND_MAX))) / 100;
			}
		}

		int matchCount = 0;

		for (uint32_t i = 0; i < 3; i++) {
			if (vert.color[i] < vert.destColor[i]) {
				float delta = vert.destColor[i] - vert.color[i];
				
				if (delta > .009) {
					vert.color[i] = vert.color[i] + .01;
				}
				else {
					vert.color[i] = vert.destColor[i];		
					matchCount++;
				}
			}
			else if (vert.color[i] > vert.destColor[i]) {
				float delta = vert.color[i] - vert.destColor[i];

				if (delta > .009) {
					vert.color[i] = vert.color[i] - .01;
				}
				else {
					vert.color[i] = vert.destColor[i];
					matchCount++;
				}
			}
			else {
				matchCount++;
			}
		}

		if (matchCount == 3) {
			for (uint32_t i = 0; i < 3; i++) {
				vert.destColor[i] = 0.0;
			}
		}

	}

	CreateVertexBuffer();
	CreateCommandBuffers();
}

int Application::RoundFloat(float input) {
	int value = (int)(input * 100 + .5);
	return value;
}

void Application::MatrixTest() {
	glm::mat4 matrix;
	glm::vec4 vector;
	auto test = matrix * vector;
}