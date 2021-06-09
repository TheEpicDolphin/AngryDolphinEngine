#pragma once

class IRenderContext {
public:
	virtual void Render() = 0;

	virtual void Destroy() = 0;

	virtual void FrameBufferSize(int* width, int* height) = 0;
};