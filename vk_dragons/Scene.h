#pragma once
#include "Renderer.h"

class Scene {
public:
	Scene(GLFWwindow* window);

private:
	Renderer renderer;
};

