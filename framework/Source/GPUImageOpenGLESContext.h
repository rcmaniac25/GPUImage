#define GPUImageRotationSwapsWidthAndHeight(rotation) ((rotation) == kGPUImageRotateLeft || (rotation) == kGPUImageRotateRight || (rotation) == kGPUImageRotateRightFlipVertical)

typedef enum { kGPUImageNoRotation, kGPUImageRotateLeft, kGPUImageRotateRight, kGPUImageFlipVertical, kGPUImageFlipHorizonal, kGPUImageRotateRightFlipVertical, kGPUImageRotate180 } GPUImageRotationMode;

#if defined(Q_OS_BLACKBERRY)

#ifndef GPUIMAGEOPENGLESCONTEXT_H
#define GPUIMAGEOPENGLESCONTEXT_H

#include "GLProgram.h"

#include <QThreadPool>
#include <QSizeF>
#include <QHash>

class GPUImageOpenGLESContext : public QObject
{
	Q_OBJECT

public:
	EGLContext context();
	const QThreadPool& contextQueue() const;
	GLProgram* currentShaderProgram() const;
	void setCurrentShaderProgram(GLProgram* program);

	static GPUImageOpenGLESContext* sharedImageProcessingOpenGLESContext();
	static const QThreadPool& sharedOpenGLESQueue();
	static void useImageProcessingContext();
	static void setActiveShaderProgram(GLProgram* shaderProgram);
	static GLint maximumTextureSizeForThisDevice();
	static GLint maximumTextureUnitsForThisDevice();
	static QSizeF sizeThatFitsWithinATextureForSize(const QSizeF& inputSize);

	void presentBufferForDisplay();
	GLProgram* programForVertexShaderString(const QString& vertexShaderString, const QString& fragmentShaderString);

	// Manage fast texture upload
	static bool supportsFastTextureUpload();

private:
	/*! @cond PRIVATE */
	GPUImageOpenGLESContext();
	~GPUImageOpenGLESContext();

	EGLContext _context;
	QThreadPool _contextQueue;
	GLProgram* _currentShaderProgram;

	QHash<QString, GLProgram*> shaderProgramCache;

	Q_DISABLE_COPY(GPUImageOpenGLESContext)
	/*! @endcond */
};
#endif
#else
#import <Foundation/Foundation.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <QuartzCore/QuartzCore.h>
#import <CoreMedia/CoreMedia.h>
#import "GLProgram.h"

@interface GPUImageOpenGLESContext : NSObject

@property(readonly, retain, nonatomic) EAGLContext *context;
@property(readonly, nonatomic) dispatch_queue_t contextQueue;
@property(readwrite, retain, nonatomic) GLProgram *currentShaderProgram;

+ (GPUImageOpenGLESContext *)sharedImageProcessingOpenGLESContext;
+ (dispatch_queue_t)sharedOpenGLESQueue;
+ (void)useImageProcessingContext;
+ (void)setActiveShaderProgram:(GLProgram *)shaderProgram;
+ (GLint)maximumTextureSizeForThisDevice;
+ (GLint)maximumTextureUnitsForThisDevice;
+ (CGSize)sizeThatFitsWithinATextureForSize:(CGSize)inputSize;

- (void)presentBufferForDisplay;
- (GLProgram *)programForVertexShaderString:(NSString *)vertexShaderString fragmentShaderString:(NSString *)fragmentShaderString;

// Manage fast texture upload
+ (BOOL)supportsFastTextureUpload;

@end

@protocol GPUImageInput
- (void)newFrameReadyAtTime:(CMTime)frameTime atIndex:(NSInteger)textureIndex;
- (void)setInputTexture:(GLuint)newInputTexture atIndex:(NSInteger)textureIndex;
- (NSInteger)nextAvailableTextureIndex;
- (void)setInputSize:(CGSize)newSize atIndex:(NSInteger)textureIndex;
- (void)setInputRotation:(GPUImageRotationMode)newInputRotation atIndex:(NSInteger)textureIndex;
- (CGSize)maximumOutputSize;
- (void)endProcessing;
- (BOOL)shouldIgnoreUpdatesToThisTarget;
- (BOOL)enabled;
@end
#endif
