#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>

void Transform::SetPosition(glm::vec3 position) {
	this->position = position;
	Apply();
}

void Transform::SetRotation(float angle, glm::vec3 rotation) {
	this->angle = angle;
	this->rotation = rotation;
	Apply();
}

void Transform::SetScale(glm::vec3 scale) {
	this->scale = scale;
	Apply();
}

glm::mat4 Transform::GetWorldMatrix() {
	return transform;
}

void Transform::Apply() {
	transform = glm::mat4();
	glm::scale(transform, scale);
	glm::rotate(transform, angle, rotation);
	glm::translate(transform, position);
}