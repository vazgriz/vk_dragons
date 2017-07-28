#pragma once
#include "Renderer.h"

class Scene {
public:
	Scene(GLFWwindow* window, uint32_t width, uint32_t height);

	void Update();
	void Render();
	void Resize(uint32_t width, uint32_t height);

private:
	Renderer renderer;
};

