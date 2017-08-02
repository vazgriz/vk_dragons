#include "Input.h"
#include <iostream>

Input::Input(GLFWwindow* window, Camera& camera) : camera(camera) {
	this->window = window;

	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, &InputCallback);
}

void Input::HandleInput(int key, int scancode, int action, int mods) {

}

void Input::InputCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
	input->HandleInput(key, scancode, action, mods);
}

void Input::Update(double elapsed) {

}