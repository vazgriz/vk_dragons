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

void Model::Bind(VkCommandBuffer commandBuffer) {
	VkBuffer buffers[] = {
		positionsBuffer.buffer,
		normalsBuffer.buffer,
		tangentsBuffer.buffer,
		binormalsBuffer.buffer,
		texcoordsBuffer.buffer
	};
	VkDeviceSize offsets[] = {
		0, 0, 0, 0, 0
	};
	vkCmdBindVertexBuffers(commandBuffer, 0, 5, buffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
}

void Model::Draw(VkCommandBuffer commandBuffer) {
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
}

void Model::CreateBuffers() {
	positionsBuffer = CreateBuffer(renderer, mesh.positions.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	normalsBuffer = CreateBuffer(renderer, mesh.normals.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	tangentsBuffer = CreateBuffer(renderer, mesh.tangents.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	binormalsBuffer = CreateBuffer(renderer, mesh.binormals.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	texcoordsBuffer = CreateBuffer(renderer, mesh.texcoords.size() * sizeof(glm::vec2),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	indicesBuffer = CreateBuffer(renderer, mesh.indices.size() * sizeof(uint32_t),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
}

Buffer Model::CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer destBuffer, void* source, size_t size) {
	Buffer buffer = CreateHostBuffer(renderer, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
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

std::vector<VkVertexInputBindingDescription> Model::GetBindingDescriptions() {
	return std::vector<VkVertexInputBindingDescription>({
		{ 0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX },	//position
		{ 1, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX },	//normal
		{ 2, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX },	//tangent
		{ 3, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX },	//binormal
		{ 4, sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX },	//texcoord
	});
}

std::vector<VkVertexInputAttributeDescription> Model::GetAttributeDescriptions() {
	return std::vector<VkVertexInputAttributeDescription>({
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },	//position
		{ 1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0 },	//normal
		{ 2, 2, VK_FORMAT_R32G32B32_SFLOAT, 0 },	//tangent
		{ 3, 3, VK_FORMAT_R32G32B32_SFLOAT, 0 },	//binormal
		{ 4, 4, VK_FORMAT_R32G32_SFLOAT, 0 },		//texcoord
	});
}

Transform& Model::GetTransform() {
	return transform;
}