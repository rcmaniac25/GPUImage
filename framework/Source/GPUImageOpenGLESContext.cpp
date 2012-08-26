#include "GPUImageOpenGLESContext.h"

#include "../Helper/singleton.h"

GPUImageOpenGLESContext::GPUImageOpenGLESContext() : _context(NULL), _contextQueue(), _currentShaderProgram(NULL), shaderProgramCache()
{
}

GPUImageOpenGLESContext::~GPUImageOpenGLESContext()
{
	//Iterate through the cache to cleanup GLPrograms
	QHash<QString, GLProgram*>::iterator i = shaderProgramCache.begin();
	while(i != shaderProgramCache.end())
	{
		//Get the GLProgram
		GLProgram* program = *i;

		//Remove the GLProgram from shaderProgramCache
		i = shaderProgramCache.erase(i);

		//Delete GLProgram
		delete program;
	}
}

GLContext* GPUImageOpenGLESContext::context()
{
	if(_context == NULL)
	{
		_context = new GLContext(EGL_OPENGL_ES2_BIT);
		Q_ASSERT_X(_context != NULL && _context->valid(), "GPUImageOpenGLESContext::context", "Unable to create an OpenGL ES 2.0 context. The GPUImage framework requires OpenGL ES 2.0 support to work.");
		GLContext::setCurrentContext(_context);

		// Set up a few global settings for the image processing pipeline
		glDisable(GL_DEPTH_TEST);
	}

	return _context;
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
	return &Singleton<GPUImageOpenGLESContext>::instance();
}

const QThreadPool& GPUImageOpenGLESContext::sharedOpenGLESQueue()
{
	return GPUImageOpenGLESContext::sharedImageProcessingOpenGLESContext()->contextQueue();
}

void GPUImageOpenGLESContext::useImageProcessingContext()
{
	GLContext* imageProcessingContext = GPUImageOpenGLESContext::sharedImageProcessingOpenGLESContext()->context();
	if(GLContext::currentContext() != imageProcessingContext)
	{
		GLContext::setCurrentContext(imageProcessingContext);
	}
}

void GPUImageOpenGLESContext::setActiveShaderProgram(GLProgram* shaderProgram)
{
	GPUImageOpenGLESContext* sharedContext = GPUImageOpenGLESContext::sharedImageProcessingOpenGLESContext();
	GLContext* imageProcessingContext = sharedContext->context();
	if(GLContext::currentContext() != imageProcessingContext)
	{
		GLContext::setCurrentContext(imageProcessingContext);
	}

	if(sharedContext->currentShaderProgram() != shaderProgram)
	{
		sharedContext->setCurrentShaderProgram(shaderProgram);
		shaderProgram->use();
	}
}

GLint GPUImageOpenGLESContext::maximumTextureSizeForThisDevice()
{
	GLint maxTextureSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	return maxTextureSize;
}

GLint GPUImageOpenGLESContext::maximumTextureUnitsForThisDevice()
{
	GLint maxTextureUnits;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
	return maxTextureUnits;
}

QSizeF GPUImageOpenGLESContext::sizeThatFitsWithinATextureForSize(const QSizeF& inputSize)
{
	GLint maxTextureSize = maximumTextureSizeForThisDevice();
	if((inputSize.width() < maxTextureSize) && (inputSize.height() < maxTextureSize))
	{
		return inputSize;
	}

	QSizeF adjustedSize;
	if(inputSize.width() > inputSize.height())
	{
		adjustedSize.setWidth((qreal)maxTextureSize);
		adjustedSize.setHeight(((qreal)maxTextureSize / inputSize.width()) * inputSize.height());
	}
	else
	{
		adjustedSize.setHeight((qreal)maxTextureSize);
		adjustedSize.setWidth(((qreal)maxTextureSize / inputSize.height()) * inputSize.width());
	}

	return adjustedSize;
}

void GPUImageOpenGLESContext::presentBufferForDisplay()
{
	_context->presentRenderbuffer();
}

GLProgram* GPUImageOpenGLESContext::programForVertexShaderString(const QString& vertexShaderString, const QString& fragmentShaderString)
{
	QString lookupKeyForShaderProgram = QString("V: %1 - F: %2").arg(vertexShaderString, fragmentShaderString);
	GLProgram* programFromCache = shaderProgramCache.value(lookupKeyForShaderProgram);

	if(programFromCache == NULL)
	{
		programFromCache = new GLProgram(vertexShaderString, fragmentShaderString, GLProgram::VertStringFragString);
		shaderProgramCache[lookupKeyForShaderProgram] = programFromCache;
	}

	return programFromCache;
}

//supportsFastTextureUpload isn't implemented because it seems to be a very Mac/iOS specific functionality
