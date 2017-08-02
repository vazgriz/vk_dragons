#pragma once
#include <GLFW/glfw3.h>
#include "Camera.h"
#include <functional>

class Input {
public:
	Input(GLFWwindow* window, Camera& camera);

	void Update(double elapsed);

private:
	GLFWwindow* window;
	Camera& camera;

	void HandleInput(int key, int scancode, int action, int mods);
	static void InputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};