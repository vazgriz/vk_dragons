#include "Input.h"

Input::Input(GLFWwindow* window, Camera& camera) : camera(camera) {
	this->window = window;

	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, &InputCallback);

	forward = false;
	back = false;
	right = false;
	left = false;
	up = false;
	down = false;
}

void Input::HandleInput(int key, int scancode, int action, int mods) {
	Toggle(forward, GLFW_KEY_W, key, action);
	Toggle(back, GLFW_KEY_S, key, action);
	Toggle(right, GLFW_KEY_D, key, action);
	Toggle(left, GLFW_KEY_A, key, action);
	Toggle(up, GLFW_KEY_E, key, action);
	Toggle(down, GLFW_KEY_Q, key, action);
}

void Input::InputCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
	input->HandleInput(key, scancode, action, mods);
}

void Input::Update(double elapsed) {
	float x = 0;
	float y = 0;
	float z = 0;

	if (forward) z += 1;
	if (back) z -= 1;
	if (right) x += 1;
	if (left) x -= 1;
	if (up) y += 1;
	if (down) y -= 1;

	if (x != 0 || y != 0 || z != 0) {
		glm::vec3 dir(x, y, z);
		glm::normalize(dir);
		x = dir.x;
		y = dir.y;
		z = dir.z;
	}

	glm::quat rot = camera.GetRotation();
	glm::vec3 pos = camera.GetPosition();

	glm::vec3 camForward = rot * glm::vec3(0, 0, -1);
	glm::vec3 camRight = rot * glm::vec3(1, 0, 0);
	glm::vec3 camUp = rot * glm::vec3(0, 1, 0);

	pos += static_cast<float>(elapsed) * x * camRight;
	pos += static_cast<float>(elapsed) * y * camUp;
	pos += static_cast<float>(elapsed) * z * camForward;

	camera.SetPosition(pos);
}

void Input::Toggle(bool& state, int keycode, int key, int action) {
	if (key == keycode) {
		if (action == GLFW_PRESS) {
			state = true;
		}
		else if (action == GLFW_RELEASE) {
			state = false;
		}
	}
}
