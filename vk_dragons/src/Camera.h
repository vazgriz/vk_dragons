#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera {
public:
	Camera(float fov, uint32_t width, uint32_t height);
	void SetPosition(glm::vec3 position);
	void SetRotation(glm::quat rotation);
	glm::vec3 GetPosition();
	glm::quat GetRotation();
	void SetSize(uint32_t width, uint32_t height);
	void Update();
	glm::mat4 GetProjection();
	glm::mat4 GetView();
	glm::mat4 GetRotationOnlyView();
private:
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 rotationOnlyView;
	glm::vec3 position;
	glm::quat rotation;
	float fov;
	uint32_t width;
	uint32_t height;
};
