#include "Model.h"

Model::Model(Renderer& renderer, const std::string& fileName) : renderer(renderer) {
	Init(fileName);
}

Model::~Model() {
	for (auto& buffer : buffers) {
		vkDestroyBuffer(renderer.device, buffer.buffer, nullptr);
		renderer.memory->Free(buffer.alloc);
	}
}

void Model::Init(const std::string& fileName) {
	loadObj(fileName, mesh, Indexed);
	centerAndUnitMesh(mesh);
	computeTangentsAndBinormals(mesh);

	CreateBuffers();
	indexCount = static_cast<uint32_t>(mesh.indices.size());
}

void Model::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Camera& camera) {
	VkBuffer vertexBuffers[] = {
		buffers[0].buffer,	//position
		buffers[1].buffer,	//normal
		buffers[2].buffer,	//tangent
		buffers[3].buffer,	//bitangent
		buffers[4].buffer	//tex coords
	};
	VkDeviceSize offsets[] = {
		0, 0, 0, 0, 0
	};
	vkCmdBindVertexBuffers(commandBuffer, 0, 5, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, buffers[5].buffer, 0, VK_INDEX_TYPE_UINT32);	//buffers[5] == index buffer

	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform.GetWorldMatrix());

	glm::mat4 MV = camera.GetView() * transform.GetWorldMatrix();
	glm::mat4 normal = glm::transpose(glm::inverse(MV));
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), 3 * sizeof(glm::vec4), &normal);

	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void Model::DrawDepth(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
	VkBuffer vertexBuffers[] = {
		buffers[0].buffer,	//position
	};
	VkDeviceSize offsets[] = {
		0
	};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, buffers[5].buffer, 0, VK_INDEX_TYPE_UINT32);	//buffers[5] == index buffer
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform.GetWorldMatrix());

	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void Model::CreateBuffers() {
	buffers.resize(6);
	buffers[0] = CreateBuffer(renderer, mesh.positions.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	buffers[1] = CreateBuffer(renderer, mesh.normals.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	buffers[2] = CreateBuffer(renderer, mesh.tangents.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	buffers[3] = CreateBuffer(renderer, mesh.binormals.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	buffers[4] = CreateBuffer(renderer, mesh.texcoords.size() * sizeof(glm::vec2),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	buffers[5] = CreateBuffer(renderer, mesh.indices.size() * sizeof(uint32_t),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
}

void Model::UploadData(VkCommandBuffer commandBuffer, std::vector<std::unique_ptr<StagingBuffer>>& stagingBuffers) {
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, buffers[0].buffer, mesh.positions.data(), mesh.positions.size() * sizeof(glm::vec3))));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, buffers[1].buffer, mesh.normals.data(), mesh.normals.size() * sizeof(glm::vec3))));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, buffers[2].buffer, mesh.tangents.data(), mesh.tangents.size() * sizeof(glm::vec3))));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, buffers[3].buffer, mesh.binormals.data(), mesh.binormals.size() * sizeof(glm::vec3))));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, buffers[4].buffer, mesh.texcoords.data(), mesh.texcoords.size() * sizeof(glm::vec2))));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, buffers[5].buffer, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t))));
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