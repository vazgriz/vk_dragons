#include "Light.h"
#include <glm/gtc/matrix_transform.hpp>

Light::Light() {
	projection = glm::ortho(-0.75f, 0.75f, -0.75f, 0.75f, -6.0f, 6.0f);
	projection[1][1] *= -1;
	Ia = glm::vec4(0.3f, 0.3f, 0.3f, 0.0f);
	Id = glm::vec4(0.8f, 0.8f, 0.8f, 0.0f);
	Is = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	shininess = 25.0f;
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

glm::vec4 Light::GetIa() {
	return Ia;
}

glm::vec4 Light::GetId() {
	return Id;
}

glm::vec4 Light::GetIs() {
	return Is;
}

float Light::GetShininess() {
	return shininess;
}