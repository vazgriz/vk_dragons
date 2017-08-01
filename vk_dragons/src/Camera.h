#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera {
public:
	Camera(float fov);
	void SetPosition(glm::vec3 position);
	void SetRotation(float x, float y, float z);
	void Update();
	glm::mat4 GetProjection();
	glm::mat4 GetView();
private:
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec3 position;
	glm::quat rotation;
	float fov;
	uint32_t width;
	uint32_t height;
};
