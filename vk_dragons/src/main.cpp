#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include "Scene.h"

#define INITIAL_SIZE_WIDTH 800
#define INITIAL_SIZE_HEIGHT 600

bool resizedFlag = false;
uint32_t width;
uint32_t height;

void OnFramebufferResized(GLFWwindow* window, int _width, int _height) {
	if (_width == 0 || _height == 0) return;

	resizedFlag = true;
	width = static_cast<uint32_t>(_width);
	height = static_cast<uint32_t>(_height);
}

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(INITIAL_SIZE_WIDTH, INITIAL_SIZE_HEIGHT, "Here Be Dragons", nullptr, nullptr);
	int _width, _height;
	glfwGetFramebufferSize(window, &_width, &_height);
	width = static_cast<uint32_t>(_width);
	height = static_cast<uint32_t>(_height);

	Scene scene(window, width, height);
	glfwSetWindowUserPointer(window, &scene);

	glfwSetFramebufferSizeCallback(window, OnFramebufferResized);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		if (resizedFlag) {
			resizedFlag = false;
			scene.Resize(width, height);
		}

		scene.Update();
		scene.Render();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}