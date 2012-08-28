#if defined(Q_OS_BLACKBERRY)

#ifndef GPUIMAGETEXTUREOUTPUT_H
#define GPUIMAGETEXTUREOUTPUT_H

#include "GPUImageOpenGLESContext.h"

class GPUImageTextureOutputDelegate;

class GPUImageTextureOutput : public QObject, public GPUImageInput
{
	Q_OBJECT
	Q_INTERFACES(GPUImageInput)

	Q_PROPERTY(bool enabled READ enabled FINAL)

public:
	GPUImageTextureOutput();
	~GPUImageTextureOutput();

	GPUImageTextureOutputDelegate* delegate() const;
	void setDelegate(GPUImageTextureOutputDelegate* del);
	GLint texture() const;
	bool enabled() const;

	void newFrameReadyAtTime(const QTime& frameTime, int textureIndex);
	void setInputTexture(GLuint newInputTexture, int textureIndex);
	int nextAvailableTextureIndex();
	void setInputSize(const QSizeF& newSize, int textureIndex);
	void setInputRotation(GPUImageRotationMode newInputRotation, int textureIndex);
	QSizeF maximumOutputSize();
	void endProcessing();
	bool shouldIgnoreUpdatesToThisTarget();

private:
	/*! @cond PRIVATE */
	GPUImageTextureOutputDelegate* _delegate;
	GLint _texture;
	bool _enabled;

	Q_DISABLE_COPY(GPUImageTextureOutput)
	/*! @endcond */
};
QML_DECLARE_TYPE(GPUImageTextureOutput)

class GPUImageTextureOutputDelegate
{
public:
	GPUImageTextureOutputDelegate(){}
	virtual ~GPUImageTextureOutputDelegate(){}

    virtual void newFrameReadyFromTextureOutput(GPUImageTextureOutput* callbackTextureOutput) = 0;
};
Q_DECLARE_INTERFACE(GPUImageTextureOutputDelegate, "GPUImage.Outputs.GPUImageTextureOutputDelegate_BlackBerry/1.0")
#endif
#else
#import <Foundation/Foundation.h>
#import "GPUImageOpenGLESContext.h"

@protocol GPUImageTextureOutputDelegate;

@interface GPUImageTextureOutput : NSObject <GPUImageInput>

@property(readwrite, unsafe_unretained, nonatomic) id<GPUImageTextureOutputDelegate> delegate;
@property(readonly) GLint texture;
@property(nonatomic) BOOL enabled;

@end

@protocol GPUImageTextureOutputDelegate
- (void)newFrameReadyFromTextureOutput:(GPUImageTextureOutput *)callbackTextureOutput;
@end
#endif
