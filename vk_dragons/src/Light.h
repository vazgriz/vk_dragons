#pragma once
#include <glm/glm.hpp>
class Light {
public:
	Light();
	void SetPosition(glm::vec3 position);
	glm::mat4 GetProjection();
	glm::mat4 GetView();
	glm::vec4 GetIa();
	glm::vec4 GetId();
	glm::vec4 GetIs();
	float GetShininess();
private:
	glm::vec3 position;
	glm::mat4 projection;
	glm::mat4 view;

	glm::vec4 Ia;
	glm::vec4 Id;
	glm::vec4 Is;
	float shininess;

	void Update();
};