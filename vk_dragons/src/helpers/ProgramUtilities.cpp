#include "ProgramUtilities.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <lodepng/lodepng.h>

std::string loadStringFromFile(const std::string & filename) {
	std::ifstream in;
	// Open a stream to the file.
	in.open(filename.c_str());
	if (!in) {
		std::cerr << filename + " is not a valid file." << std::endl;
		return "";
	}
	std::stringstream buffer;
	// Read the stream in a buffer.
	buffer << in.rdbuf();
	// Create a string based on the content of the buffer.
	std::string line = buffer.str();
	in.close();
	return line;
}

void flipImage(std::vector<unsigned char> & image, const int width, const int height){
	// Compute the number of components per pixel.
	int components = static_cast<int>(image.size() / (width * height));
	// The width in bytes.
	int widthInBytes = width * components;
	int halfHeight = height/2;

	// For each line of the first half, we swap it with the mirroring line, starting from the end of the image.
	for(int h=0; h < halfHeight; h++){
		std::swap_ranges(image.begin() + h * widthInBytes, image.begin() + (h+1) * widthInBytes , image.begin() + (height - h - 1) * widthInBytes);
	}
}

static std::vector<char> loadFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}