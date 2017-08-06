#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float fov, uint32_t width, uint32_t height) {
	this->fov = fov;
	this->width = width;
	this->height;
}

void Camera::SetPosition(glm::vec3 position) {
	this->position = position;
}

void Camera::SetRotation(glm::quat rotation) {
	this->rotation = rotation;
}

glm::vec3 Camera::GetPosition() {
	return position;
}

glm::quat Camera::GetRotation() {
	return rotation;
}

void Camera::SetSize(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;
}

void Camera::Update() {
	projection = glm::perspective(glm::radians(fov), width / static_cast<float>(height), 0.1f, 100.0f);
	projection[1][1] *= -1;	//flip y axis

	glm::vec3 forward = rotation * glm::vec3(0, 0, -1);
	glm::vec3 up = rotation * glm::vec3(0, 1, 0);
	view = glm::lookAt(position, position + forward, up);
	rotationOnlyView = glm::lookAt(glm::vec3(), forward, up);
}

glm::mat4 Camera::GetProjection() {
	return projection;
}

glm::mat4 Camera::GetView() {
	return view;
}

glm::mat4 Camera::GetRotationOnlyView() {
	return rotationOnlyView;
}