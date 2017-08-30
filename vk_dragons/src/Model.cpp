#include "Model.h"

Model::Model(Renderer& renderer, const std::string& fileName) : renderer(renderer) {
	Init(fileName);
}

Model::~Model() {
	vkDestroyBuffer(renderer.device, positionsBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, normalsBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, tangentsBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, binormalsBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, texcoordsBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, indicesBuffer.buffer, nullptr);
}

void Model::Init(const std::string& fileName) {
	loadObj(fileName, mesh, Indexed);
	centerAndUnitMesh(mesh);
	computeTangentsAndBinormals(mesh);

	CreateBuffers();
	indexCount = static_cast<uint32_t>(mesh.indices.size());
}

void Model::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Camera& camera) {
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

	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform.GetWorldMatrix());

	glm::mat4 MV = camera.GetView() * transform.GetWorldMatrix();
	glm::mat4 normal = glm::transpose(glm::inverse(MV));
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), 3 * sizeof(glm::vec4), &normal);

	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void Model::DrawDepth(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
	VkBuffer buffers[] = {
		positionsBuffer.buffer,
	};
	VkDeviceSize offsets[] = {
		0
	};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform.GetWorldMatrix());

	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
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

void Model::UploadData(VkCommandBuffer commandBuffer, std::vector<std::unique_ptr<StagingBuffer>>& stagingBuffers) {
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, positionsBuffer.buffer, mesh.positions.data(), mesh.positions.size() * sizeof(glm::vec3))));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, normalsBuffer.buffer, mesh.normals.data(), mesh.normals.size() * sizeof(glm::vec3))));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, tangentsBuffer.buffer, mesh.tangents.data(), mesh.tangents.size() * sizeof(glm::vec3))));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, binormalsBuffer.buffer, mesh.binormals.data(), mesh.binormals.size() * sizeof(glm::vec3))));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, texcoordsBuffer.buffer, mesh.texcoords.data(), mesh.texcoords.size() * sizeof(glm::vec2))));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, indicesBuffer.buffer, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t))));
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

std::vector<VkVertexInputBindingDescription> Model::GetDepthBindingDescriptions() {
	return std::vector<VkVertexInputBindingDescription>({
		{ 0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX },	//position
	});
}

std::vector<VkVertexInputAttributeDescription> Model::GetDepthAttributeDescriptions() {
	return std::vector<VkVertexInputAttributeDescription>({
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },	//position
	});
}

Transform& Model::GetTransform() {
	return transform;
}