#include "GPUImageOpenGLESContext.h"

GPUImageOpenGLESContext::GPUImageOpenGLESContext() : _context(EGL_NO_CONTEXT), _contextQueue(), _currentShaderProgram(NULL), shaderProgramCache()
{
	//TODO
}

GPUImageOpenGLESContext::~GPUImageOpenGLESContext()
{
	//TODO
}

EGLContext GPUImageOpenGLESContext::context()
{
	//TODO
	return EGL_NO_CONTEXT;
}

const QThreadPool& GPUImageOpenGLESContext::contextQueue() const
{
	return _contextQueue;
}

GLProgram* GPUImageOpenGLESContext::currentShaderProgram() const
{
	return _currentShaderProgram;
}

void GPUImageOpenGLESContext::setCurrentShaderProgram(GLProgram* program)
{
	_currentShaderProgram = program;
}

GPUImageOpenGLESContext* GPUImageOpenGLESContext::sharedImageProcessingOpenGLESContext()
{
	//TODO
	return NULL;
}

const QThreadPool& GPUImageOpenGLESContext::sharedOpenGLESQueue()
{
	return GPUImageOpenGLESContext::sharedImageProcessingOpenGLESContext()->contextQueue();
}

void GPUImageOpenGLESContext::useImageProcessingContext()
{
	//TODO
}

void GPUImageOpenGLESContext::setActiveShaderProgram(GLProgram* shaderProgram)
{
	//TODO
}

GLint GPUImageOpenGLESContext::maximumTextureSizeForThisDevice()
{
	//TODO
	return 0;
}

GLint GPUImageOpenGLESContext::maximumTextureUnitsForThisDevice()
{
	//TODO
	return 0;
}

QSizeF GPUImageOpenGLESContext::sizeThatFitsWithinATextureForSize(const QSizeF& inputSize)
{
	//TODO
	return QSizeF();
}

void GPUImageOpenGLESContext::presentBufferForDisplay()
{
	//TODO
}

GLProgram* GPUImageOpenGLESContext::programForVertexShaderString(const QString& vertexShaderString, const QString& fragmentShaderString)
{
	//TODO
	return NULL;
}

bool GPUImageOpenGLESContext::supportsFastTextureUpload()
{
	//TODO
	return false;
}
