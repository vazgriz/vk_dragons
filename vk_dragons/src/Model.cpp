#include "Model.h"

Model::Model(Renderer& renderer) : renderer(renderer) {

}

Model::~Model() {
	vkDestroyBuffer(renderer.device, positionsBuffer, nullptr);
	vkDestroyBuffer(renderer.device, normalsBuffer, nullptr);
	vkDestroyBuffer(renderer.device, tangentsBuffer, nullptr);
	vkDestroyBuffer(renderer.device, binormalsBuffer, nullptr);
	vkDestroyBuffer(renderer.device, texcoordsBuffer, nullptr);
}

void Model::Init(const std::string& fileName) {
	loadObj(fileName, mesh, Indexed);
	centerAndUnitMesh(mesh);
	computeTangentsAndBinormals(mesh);

	CreateBuffers();
}

void Model::CreateBuffers() {
	positionsBuffer = CreateBuffer(mesh.positions.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		*renderer.memory->deviceAllocator);

	normalsBuffer = CreateBuffer(mesh.normals.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		*renderer.memory->deviceAllocator);

	tangentsBuffer = CreateBuffer(mesh.tangents.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		*renderer.memory->deviceAllocator);

	binormalsBuffer = CreateBuffer(mesh.binormals.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		*renderer.memory->deviceAllocator);

	texcoordsBuffer = CreateBuffer(mesh.texcoords.size() * sizeof(glm::vec2),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		*renderer.memory->deviceAllocator);
}

VkBuffer Model::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, Allocator allocator) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer;

	if (vkCreateBuffer(renderer.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(renderer.device, buffer, &memRequirements);

	Allocation alloc = allocator.Alloc(size, memRequirements.alignment);

	vkBindBufferMemory(renderer.device, buffer, alloc.memory, alloc.offset);

	return buffer;
}