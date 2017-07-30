#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include "MeshUtilities.h"
#include "Renderer.h"
#include "MemorySystem.h"
#include "Allocator.h"

class Model {
public:
	Model(Renderer& renderer);
	~Model();
	void Init(const std::string& fileName);

private:
	Renderer& renderer;
	mesh_t mesh;
	VkBuffer positionsBuffer;
	VkBuffer normalsBuffer;
	VkBuffer tangentsBuffer;
	VkBuffer binormalsBuffer;
	VkBuffer texcoordsBuffer;
	VkBuffer indicesBuffer;

	void CreateBuffers();
	VkBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, Allocator allocator);
};