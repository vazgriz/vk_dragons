#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include "Scene.h"

#define INITIAL_SIZE_WIDTH 800
#define INITIAL_SIZE_HEIGHT 600

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(INITIAL_SIZE_WIDTH, INITIAL_SIZE_HEIGHT, "Here Be Dragons", nullptr, nullptr);

	Scene scene(window, INITIAL_SIZE_WIDTH, INITIAL_SIZE_HEIGHT);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		scene.Update();
		scene.Render();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}