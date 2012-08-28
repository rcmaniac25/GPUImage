#include "GPUImageTextureOutput.h"

GPUImageTextureOutput::GPUImageTextureOutput() : _delegate(NULL), _texture(0), _enabled(true)
{
}

GPUImageTextureOutput::~GPUImageTextureOutput()
{
}

GPUImageTextureOutputDelegate* GPUImageTextureOutput::delegate() const
{
	return _delegate;
}

void GPUImageTextureOutput::setDelegate(GPUImageTextureOutputDelegate* del)
{
	_delegate = del;
}

GLint GPUImageTextureOutput::texture() const
{
	return _texture;
}

bool GPUImageTextureOutput::enabled() const
{
	return _enabled;
}

void GPUImageTextureOutput::newFrameReadyAtTime(const QTime&, int)
{
	_delegate->newFrameReadyFromTextureOutput(this);
}

int GPUImageTextureOutput::nextAvailableTextureIndex()
{
	return 0;
}

void GPUImageTextureOutput::setInputTexture(GLuint newInputTexture, int)
{
	_texture = newInputTexture;
}

void GPUImageTextureOutput::setInputRotation(GPUImageRotationMode, int)
{
}

void GPUImageTextureOutput::setInputSize(const QSizeF&, int)
{
}

QSizeF GPUImageTextureOutput::maximumOutputSize()
{
	return QSizeF(0, 0);
}

void GPUImageTextureOutput::endProcessing()
{
}

bool GPUImageTextureOutput::shouldIgnoreUpdatesToThisTarget()
{
	return false;
}
