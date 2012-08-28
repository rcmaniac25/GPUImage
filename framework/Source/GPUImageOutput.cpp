#include "GPUImageOutput.h"

#include <QCoreApplication>
#include <bb/cascades/OrientationSupport>

__BEGIN_DECLS

void runOnMainQueueWithoutDeadlocking(QRunnable* block)
{
	if(QCoreApplication::instance())
	{
		const QThread* thread = QCoreApplication::instance()->thread();
		if(QThread::currentThread() == thread)
		{
			block->run();
			if(block->autoDelete())
			{
				delete block;
			}
		}
		else
		{
			//TODO: dispatch_sync(dispatch_get_main_queue(), block);
		}
	}
	else
	{
		//TODO: What should we do? There is no "application"
	}
}

void runSynchronouslyOnVideoProcessingQueue(QRunnable* block)
{
	const QThreadPool& videoProcessingQueue = GPUImageOpenGLESContext::sharedOpenGLESQueue();
	//TODO
	/*
	if (dispatch_get_current_queue() == videoProcessingQueue)
	{
		block();
	}
	else
	{
		dispatch_sync(videoProcessingQueue, block);
	}
	 */
}

void report_memory(QString* tag)
{
	const QString& tags = tag ? *tag : "Default";
	//TODO
	/*
	struct task_basic_info info;

    mach_msg_type_number_t size = sizeof(info);

    kern_return_t kerr = task_info(mach_task_self(),

                                   TASK_BASIC_INFO,

                                   (task_info_t)&info,

                                   &size);
    if( kerr == KERN_SUCCESS ) {
        NSLog(@"%@ - Memory used: %u", tag, info.resident_size); //in bytes
    } else {
        NSLog(@"%@ - Error: %s", tag, mach_error_string(kerr));
    }
	 */
}

__END_DECLS

GPUImageOutput::GPUImageOutput() : _targets(), targetTextureIndices(), outputTexture(0), inputTextureSize(), cachedMaximumOutputSize(), forcedMaximumSize(), _shouldSmoothlyScaleOutput(false),
	_shouldIgnoreUpdatesToThisTarget(false), _audioEncodingTarget(NULL), _targetToIgnoreForUpdates(NULL), _frameProcessingCompletionBlock(NULL), _enabled(true)
{
	initializeOutputTexture();
}

GPUImageOutput::~GPUImageOutput()
{
	removeAllTargets();
	deleteOutputTexture();
}

bool GPUImageOutput::shouldSmoothlyScaleOutput() const
{
	return _shouldSmoothlyScaleOutput;
}

void GPUImageOutput::setShouldSmoothlyScaleOutput(bool smooth)
{
	_shouldSmoothlyScaleOutput = smooth;
}

bool GPUImageOutput::shouldIgnoreUpdatesToThisTarget() const
{
	return _shouldIgnoreUpdatesToThisTarget;
}

void GPUImageOutput::setShouldIgnoreUpdatesToThisTarget(bool ignore)
{
	_shouldIgnoreUpdatesToThisTarget = ignore;
}

GPUImageMovieWriter* GPUImageOutput::audioEncodingTarget() const
{
	return _audioEncodingTarget;
}

void GPUImageOutput::setAudioEncodingTarget(GPUImageMovieWriter* movieWriter)
{
	_audioEncodingTarget = movieWriter;

	//_audioEncodingTarget->setHasAudioTrack(true); //XXX
}

GPUImageInput& GPUImageOutput::targetToIgnoreForUpdates() const
{
	return *_targetToIgnoreForUpdates;
}

void GPUImageOutput::setTargetToIgnoreForUpdates(GPUImageInput& ignore)
{
	_targetToIgnoreForUpdates = &ignore;
}

GPUImageOutput::frameProcessingCompletionFunc GPUImageOutput::frameProcessingCompletionBlock() const
{
	return _frameProcessingCompletionBlock;
}

bool GPUImageOutput::enabled() const
{
	return _enabled;
}

void GPUImageOutput::setInputTextureForTarget(GPUImageInput& target, int inputTextureIndex)
{
	target.setInputTexture(textureForOutput(), inputTextureIndex);
}

GLuint GPUImageOutput::textureForOutput() const
{
	return outputTexture;
}

void GPUImageOutput::notifyTargetsAboutNewOutputTexture()
{
	foreach(GPUImageInput* currentTarget, _targets)
	{
		int indexOfObject = _targets.indexOf(currentTarget);
		int textureIndex = targetTextureIndices[indexOfObject];

		setInputTextureForTarget(*currentTarget, textureIndex);
	}
}

QListIterator<GPUImageInput*> GPUImageOutput::targets() const
{
	return QListIterator<GPUImageInput*>(_targets);
}

void GPUImageOutput::addTarget(GPUImageInput& newTarget)
{
	int nextAvailableTextureIndex = newTarget.nextAvailableTextureIndex();
	addTarget(newTarget, nextAvailableTextureIndex);
	if(newTarget.shouldIgnoreUpdatesToThisTarget())
	{
		_targetToIgnoreForUpdates = &newTarget;
	}
}

class AddTarget_Runnable : public QRunnable
{
	GPUImageOutput* output;
	GPUImageInput& newTarget;
	int textureLocation;

public:
	AddTarget_Runnable(GPUImageOutput* out, GPUImageInput& n, int tl) : output(out), newTarget(n), textureLocation(tl){}

	void run()
	{
		output->setInputTextureForTarget(newTarget, textureLocation);
		output->_targets.append(&newTarget);
		output->targetTextureIndices.append(textureLocation);
	}
};

void GPUImageOutput::addTarget(GPUImageInput& newTarget, int textureLocation)
{
	if(_targets.contains(&newTarget))
	{
		return;
	}

	cachedMaximumOutputSize *= 0;
	runSynchronouslyOnVideoProcessingQueue(new AddTarget_Runnable(this, newTarget, textureLocation));
}

class RemoveTarget_Runnable : public QRunnable
{
	GPUImageOutput* output;
	GPUImageInput& targetToRemove;
	int indexOfObject;
	int textureIndexOfTarget;

public:
	RemoveTarget_Runnable(GPUImageOutput* out, GPUImageInput& r, int i, int ti) : output(out), targetToRemove(r), indexOfObject(i), textureIndexOfTarget(ti){}

	void run()
	{
		targetToRemove.setInputSize(QSizeF(0,0), textureIndexOfTarget);
		targetToRemove.setInputTexture(0, textureIndexOfTarget);

		output->targetTextureIndices.removeAt(indexOfObject);
		output->_targets.removeOne(&targetToRemove);
		targetToRemove.endProcessing();
	}
};

void GPUImageOutput::removeTarget(GPUImageInput& targetToRemove)
{
	if(!_targets.contains(&targetToRemove))
	{
		return;
	}

	if (_targetToIgnoreForUpdates == &targetToRemove)
	{
		_targetToIgnoreForUpdates = NULL;
	}

	cachedMaximumOutputSize *= 0;

	int indexOfObject = _targets.indexOf(&targetToRemove);
	int textureIndexOfTarget = targetTextureIndices[indexOfObject];

	runSynchronouslyOnVideoProcessingQueue(new RemoveTarget_Runnable(this, targetToRemove, indexOfObject, textureIndexOfTarget));
}

class RemoveAllTargets_Runnable : public QRunnable
{
	GPUImageOutput* output;

public:
	RemoveAllTargets_Runnable(GPUImageOutput* out) : output(out){}

	void run()
	{
		foreach(GPUImageInput* targetToRemove, output->_targets)
		{
			int indexOfObject = output->_targets.indexOf(targetToRemove);
			int textureIndexOfTarget = output->targetTextureIndices[indexOfObject];

			targetToRemove->setInputSize(QSizeF(0,0), textureIndexOfTarget);
			targetToRemove->setInputTexture(0, textureIndexOfTarget);
			targetToRemove->setInputRotation(kGPUImageNoRotation, textureIndexOfTarget);
		}
		output->_targets.clear();
		output->targetTextureIndices.clear();
	}
};

void GPUImageOutput::removeAllTargets()
{
	cachedMaximumOutputSize *= 0;
	runSynchronouslyOnVideoProcessingQueue(new RemoveAllTargets_Runnable(this));
}

class InitializeOutputTexture_Runnable : public QRunnable
{
	GPUImageOutput* output;

public:
	InitializeOutputTexture_Runnable(GPUImageOutput* out) : output(out){}

	void run()
	{
		GPUImageOpenGLESContext::useImageProcessingContext();

		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &(output->outputTexture));
		glBindTexture(GL_TEXTURE_2D, output->outputTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// This is necessary for non-power-of-two textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};

void GPUImageOutput::initializeOutputTexture()
{
	runSynchronouslyOnVideoProcessingQueue(new InitializeOutputTexture_Runnable(this));
}

class DeleteOutputTexture_Runnable : public QRunnable
{
	GPUImageOutput* output;

public:
	DeleteOutputTexture_Runnable(GPUImageOutput* out) : output(out){}

	void run()
	{
		GPUImageOpenGLESContext::useImageProcessingContext();

		if(output->outputTexture != 0)
		{
			glDeleteTextures(1, &(output->outputTexture));
			output->outputTexture = 0;
		}
	}
};

void GPUImageOutput::deleteOutputTexture()
{
	runSynchronouslyOnVideoProcessingQueue(new DeleteOutputTexture_Runnable(this));
}

void GPUImageOutput::forceProcessingAtSize(const QSizeF&)
{
}

void GPUImageOutput::forceProcessingAtSizeRespectingAspectRatio(const QSizeF&)
{
}

PixelBufferData* GPUImageOutput::imageFromCurrentlyProcessedOutput()
{
	DisplayDirection::Type deviceOrientation = OrientationSupport::instance().displayDirection();
	return imageFromCurrentlyProcessedOutputWithOrientation(deviceOrientation);
}

QImage* GPUImageOutput::newQImageFromCurrentlyProcessedOutput()
{
	DisplayDirection::Type deviceOrientation = OrientationSupport::instance().displayDirection();
	return newQImageFromCurrentlyProcessedOutputWithOrientation(deviceOrientation);
}

PixelBufferData* GPUImageOutput::imageFromCurrentlyProcessedOutputWithOrientation(DisplayDirection::Type)
{
	return NULL;
}

PixelBufferData* GPUImageOutput::imageByFilteringImage(PixelBufferData*)
{
	return NULL;
}

QImage* GPUImageOutput::newQImageFromCurrentlyProcessedOutputWithOrientation(DisplayDirection::Type)
{
	return NULL;
}

QImage* GPUImageOutput::newQImageByFilteringImage(PixelBufferData*)
{
	return NULL;
}

QImage* GPUImageOutput::newQImageByFilteringQImage(QImage*)
{
	return NULL;
}

QImage* GPUImageOutput::newQImageByFilteringQImage(QImage*, DisplayDirection::Type)
{
	return NULL;
}

void GPUImageOutput::prepareForImageCapture()
{
}
