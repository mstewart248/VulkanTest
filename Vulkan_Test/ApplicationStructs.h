#pragma once 

#include <optional>
#include <array>

struct QueueFamilyIndices {

	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {

		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 destColor;

	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindDescrip = {};

		bindDescrip.binding = 0;
		bindDescrip.stride = sizeof(Vertex);
		bindDescrip.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindDescrip;
	}

	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> attributeDescrip = {};

		attributeDescrip[0].binding = 0;
		attributeDescrip[0].location = 0;
		attributeDescrip[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescrip[0].offset = offsetof(Vertex, pos);
		
		attributeDescrip[1].binding = 0;
		attributeDescrip[1].location = 1;
		attributeDescrip[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescrip[1].offset = offsetof(Vertex, color);

		attributeDescrip[2].binding = 0;
		attributeDescrip[2].location = 2;
		attributeDescrip[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescrip[2].offset = offsetof(Vertex, texCoord);

		attributeDescrip[3].binding = 0;
		attributeDescrip[3].location = 3;
		attributeDescrip[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescrip[3].offset = offsetof(Vertex, destColor);

		return attributeDescrip;
	}
};

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;

};