#include "Texture.h"
#include <iostream>
#include "lodepng\lodepng.h"
#include "ProgramUtilities.h"

Texture::Texture(Renderer& renderer) : renderer(renderer) {

}

void Texture::Init(const std::string& filename) {
	unsigned int width, height;
	unsigned int error = lodepng::decode(data, width, height, filename);
	if (error != 0) {
		std::cerr << "Unable to load the texture at path " << filename << std::endl;
	}
	flipImage(data, width, height);
}