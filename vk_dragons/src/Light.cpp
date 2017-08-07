#include "Light.h"
#include <glm/gtc/matrix_transform.hpp>

Light::Light() {
	projection = glm::ortho(-0.75f, 0.75f, -0.75f, 0.75f, 2.0f, 6.0f);
	Update();
}

void Light::SetPosition(glm::vec3 position) {
	this->position = position;
	Update();
}

void Light::Update() {
	view = glm::lookAt(position, glm::vec3(), glm::vec3(0, 1, 0));
}

glm::mat4 Light::GetProjection() {
	return projection;
}

glm::mat4 Light::GetView() {
	return view;
}