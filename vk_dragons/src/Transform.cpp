#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>

Transform::Transform() {
	scale = glm::vec3(1, 1, 1);
	rotation = glm::vec3(1, 0, 0);
	angle = 0.0f;
}

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

glm::mat4& Transform::GetWorldMatrix() {
	return transform;
}

void Transform::Apply() {
	transform = glm::mat4();
	transform = glm::translate(transform, position);
	transform = glm::rotate(transform, angle, rotation);
	transform = glm::scale(transform, scale);
}