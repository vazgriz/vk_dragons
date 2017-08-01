#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float fov) {
	this->fov = fov;
}

void Camera::SetPosition(glm::vec3 position) {
	this->position = position;
}

void Camera::SetRotation(float x, float y, float z) {
	rotation = glm::quat(glm::vec3(x, y, z));
}

void Camera::Update() {
	projection = glm::perspective(fov, width / static_cast<float>(height), 0.1f, 100.0f);
	glm::vec3 forward = rotation * glm::vec3(0, 0, -1);
	glm::vec3 up = rotation * glm::vec3(0, 1, 0);
	view = glm::lookAt(position, position + forward, up);
}

glm::mat4 Camera::GetProjection() {
	return projection;
}

glm::mat4 Camera::GetView() {
	return view;
}