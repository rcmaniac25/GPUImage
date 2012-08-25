#if defined(Q_OS_BLACKBERRY)

#ifndef GPUIMAGEINPUT_H
#define GPUIMAGEINPUT_H

#include "GPUImageOpenGLESContext.h"

#include <QTime>

class GPUImageInput
{
public:
	GPUImageInput(){}
	virtual ~GPUImageInput(){}

    virtual void newFrameReadyAtTime(const QTime& frameTime, int textureIndex) = 0; //QTime isn't really the best choice. Originally CMTime
    virtual void setInputTexture(GLuint newInputTexture, int textureIndex) = 0;
    virtual int nextAvailableTextureIndex() = 0;
    virtual void setInputSize(const QSizeF& newSize, int textureIndex) = 0;
    virtual void setInputRotation(GPUImageRotationMode newInputRotation, int textureIndex) = 0;
    virtual QSizeF maximumOutputSize() = 0;
    virtual void endProcessing() = 0;
    virtual bool shouldIgnoreUpdatesToThisTarget() = 0;
    virtual bool enabled() = 0;
};
Q_DECLARE_INTERFACE(GPUImageInput, "GPUImage.GPUImageInput_BlackBerry/1.0")

#endif
#endif
