#pragma once
#include <glm/glm.hpp>

class Transform {
public:
	Transform();

	void SetPosition(glm::vec3 position);
	void SetRotation(float angle, glm::vec3 rotation);
	void SetScale(glm::vec3 scale);
	glm::mat4& GetWorldMatrix();
private:
	glm::vec3 position;
	glm::vec3 rotation;
	float angle;
	glm::vec3 scale;
	glm::mat4 transform;

	void Apply();
};