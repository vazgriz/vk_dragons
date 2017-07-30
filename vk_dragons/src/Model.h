#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include "MeshUtilities.h"
#include "Renderer.h"
#include "MemorySystem.h"
#include "Allocator.h"

struct Buffer {
	VkBuffer buffer;
	size_t size;
	size_t offset;
};

class Model {
public:
	Model(Renderer& renderer);
	~Model();
	void Init(const std::string& fileName);
	void UploadData(VkCommandBuffer commandBuffer);
	void DestroyStaging();

private:
	Renderer& renderer;
	mesh_t mesh;
	Buffer positionsBuffer;
	Buffer normalsBuffer;
	Buffer tangentsBuffer;
	Buffer binormalsBuffer;
	Buffer texcoordsBuffer;
	Buffer indicesBuffer;

	Buffer positionsStagingBuffer;
	Buffer normalsStagingBuffer;
	Buffer tangentsStagingBuffer;
	Buffer binormalsStagingBuffer;
	Buffer texcoordsStagingBuffer;
	Buffer indicesStagingBuffer;

	void CreateBuffers();
	Buffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, Allocator allocator);
	Buffer CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer destBuffer, void* source, size_t size);
};