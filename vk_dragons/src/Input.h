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

	void HandleKey(int key, int scancode, int action, int mods);
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	void HandleMouse(double xpos, double ypos);
	static void MouseCallback(GLFWwindow* window, double xpos, double ypos);

	void HandleMouseButton(int button, int action, int mods);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	bool forward;
	bool back;
	bool right;
	bool left;
	bool up;
	bool down;
	bool looking;
	float mouseX;
	float mouseY;
	float lookX;
	float lookY;

	void UpdatePos(double elapsed);
	void UpdateRot(double elapsed);

	void Toggle(bool& state, int keycode, int key, int action);
};