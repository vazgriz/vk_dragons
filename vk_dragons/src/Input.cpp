#include "Input.h"
#include <algorithm>

Input::Input(GLFWwindow* window, Camera& camera) : camera(camera) {
	this->window = window;

	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, &KeyCallback);
	glfwSetCursorPosCallback(window, &MouseCallback);
	glfwSetMouseButtonCallback(window, &MouseButtonCallback);

	forward = false;
	back = false;
	right = false;
	left = false;
	up = false;
	down = false;
	lookX = 0;
	lookY = 0;
}

void Input::HandleKey(int key, int scancode, int action, int mods) {
	Toggle(forward, GLFW_KEY_W, key, action);
	Toggle(back, GLFW_KEY_S, key, action);
	Toggle(right, GLFW_KEY_D, key, action);
	Toggle(left, GLFW_KEY_A, key, action);
	Toggle(up, GLFW_KEY_E, key, action);
	Toggle(down, GLFW_KEY_Q, key, action);

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
	input->HandleKey(key, scancode, action, mods);
}

void Input::HandleMouse(double xpos, double ypos) {
	float newMouseX = static_cast<float>(xpos);
	float newMouseY = static_cast<float>(ypos);

	float deltaX = newMouseX - mouseX;
	float deltaY = newMouseY - mouseY;

	lookX += deltaX;
	lookY += deltaY;
	lookY = std::min(std::max(lookY, -90.0f), 90.0f);

	mouseX = newMouseX;
	mouseY = newMouseY;
}

void Input::MouseCallback(GLFWwindow* window, double xpos, double ypos) {
	Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
	input->HandleMouse(xpos, ypos);
}

void Input::HandleMouseButton(int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
	input->HandleMouseButton(button, action, mods);
}

void Input::Update(double elapsed) {
	UpdatePos(elapsed);
	UpdateRot(elapsed);
}

void Input::UpdatePos(double elapsed) {
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

void Input::UpdateRot(double elapsed) {
	float radX = glm::radians(lookX);
	float radY = glm::radians(lookY);

	glm::quat rot = glm::quat(glm::vec3(0, -radX, 0)) * glm::quat(glm::vec3(-radY, 0, 0));
	camera.SetRotation(rot);
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
