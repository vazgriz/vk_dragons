#include<GLFW/glfw3.h>

#define INITIAL_SIZE_WIDTH 800
#define INITIAL_SIZE_HEIGHT 600

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(INITIAL_SIZE_WIDTH, INITIAL_SIZE_HEIGHT, "Here Be Dragons", nullptr, nullptr);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}