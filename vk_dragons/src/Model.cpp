#include "Model.h"

Model::Model(Renderer& renderer) : renderer(renderer) {

}

Model::~Model() {
	vkDestroyBuffer(renderer.device, positionsBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, normalsBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, tangentsBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, binormalsBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, texcoordsBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, indicesBuffer.buffer, nullptr);
}

void Model::DestroyStaging() {
	vkDestroyBuffer(renderer.device, positionsStagingBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, normalsStagingBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, tangentsStagingBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, binormalsStagingBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, texcoordsStagingBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, indicesStagingBuffer.buffer, nullptr);
}

void Model::Init(const std::string& fileName) {
	loadObj(fileName, mesh, Indexed);
	centerAndUnitMesh(mesh);
	computeTangentsAndBinormals(mesh);

	CreateBuffers();
}

void Model::CreateBuffers() {
	positionsBuffer = CreateBuffer(renderer.device, mesh.positions.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		*renderer.memory->deviceAllocator);

	normalsBuffer = CreateBuffer(renderer.device, mesh.normals.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		*renderer.memory->deviceAllocator);

	tangentsBuffer = CreateBuffer(renderer.device, mesh.tangents.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		*renderer.memory->deviceAllocator);

	binormalsBuffer = CreateBuffer(renderer.device, mesh.binormals.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		*renderer.memory->deviceAllocator);

	texcoordsBuffer = CreateBuffer(renderer.device, mesh.texcoords.size() * sizeof(glm::vec2),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		*renderer.memory->deviceAllocator);

	indicesBuffer = CreateBuffer(renderer.device, mesh.indices.size() * sizeof(uint32_t),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		*renderer.memory->deviceAllocator);
}

Buffer Model::CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer destBuffer, void* source, size_t size) {
	Buffer buffer = CreateBuffer(renderer.device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, *renderer.memory->hostAllocator);
	char* dest = static_cast<char*>(renderer.memory->hostMapping) + buffer.offset;
	memcpy(dest, source, size);

	VkBufferCopy copy = {};
	copy.size = size;
	copy.dstOffset = 0;	//these two offset values refer to the offset within each buffer, not the buffer's offset in memory
	copy.srcOffset = 0;

	vkCmdCopyBuffer(commandBuffer, buffer.buffer, destBuffer, 1, &copy);

	return buffer;
}

void Model::UploadData(VkCommandBuffer commandBuffer) {
	positionsStagingBuffer = CopyBuffer(commandBuffer, positionsBuffer.buffer, mesh.positions.data(), mesh.positions.size() * sizeof(glm::vec3));
	normalsStagingBuffer = CopyBuffer(commandBuffer, normalsBuffer.buffer, mesh.normals.data(), mesh.normals.size() * sizeof(glm::vec3));
	tangentsStagingBuffer = CopyBuffer(commandBuffer, tangentsBuffer.buffer, mesh.tangents.data(), mesh.tangents.size() * sizeof(glm::vec3));
	binormalsStagingBuffer = CopyBuffer(commandBuffer, binormalsBuffer.buffer, mesh.binormals.data(), mesh.binormals.size() * sizeof(glm::vec3));
	texcoordsStagingBuffer = CopyBuffer(commandBuffer, texcoordsBuffer.buffer, mesh.texcoords.data(), mesh.texcoords.size() * sizeof(glm::vec2));
	indicesStagingBuffer = CopyBuffer(commandBuffer, indicesBuffer.buffer, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));
}