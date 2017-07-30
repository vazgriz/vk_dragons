#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include "MeshUtilities.h"

class Model {
public:
	void Init(const std::string& fileName);

private:
	mesh_t mesh;
	VkBuffer positionsBuffer;
	VkBuffer normalsBuffer;
	VkBuffer tangentsBuffer;
	VkBuffer binormalsBuffer;
	VkBuffer texcoordsBuffer;
	VkBuffer indicesBuffer;
};