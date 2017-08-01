#pragma once
#include <string>
#include <vector>
#include "Renderer.h"

class Texture {
public:
	Texture(Renderer& renderer);

	void Init(const std::string& filename);

private:
	Renderer& renderer;
	std::vector<unsigned char> data;
};