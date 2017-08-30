#pragma once
#include "ProgramUtilities.h"

class StagingBuffer {
public:
	StagingBuffer(Renderer& renderer, Buffer buffer);
	~StagingBuffer();

private:
	Renderer& renderer;
	Buffer buffer;
};