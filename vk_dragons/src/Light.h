#pragma once
#include <glm/glm.hpp>
class Light {
public:
	Light();
	void SetPosition(glm::vec3 position);
	glm::mat4 GetProjection();
	glm::mat4 GetView();
private:
	glm::vec3 position;
	glm::mat4 projection;
	glm::mat4 view;

	void Update();
};