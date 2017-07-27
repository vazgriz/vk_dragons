#pragma once
#include "Renderer.h"

class Scene {
public:
	Scene(GLFWwindow* window, uint32_t width, uint32_t height);

	void Update();
	void Render();

private:
	Renderer renderer;
};

