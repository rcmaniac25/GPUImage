#if defined(Q_OS_BLACKBERRY)

#ifndef GPUIMAGEOUTPUT_H
#define GPUIMAGEOUTPUT_H

#include "GPUImageOpenGLESContext.h"

__BEGIN_DECLS

void runOnMainQueueWithoutDeadlocking(QRunnable* block);
void runSynchronouslyOnVideoProcessingQueue(QRunnable* block);
void report_memory(QString* tag);

__END_DECLS

class GPUImageMovieWriter;

/** GPUImage's base source object

 Images or frames of video are uploaded from source objects, which are subclasses of GPUImageOutput. These include:

 - GPUImageVideoCamera (for live video from an iOS camera)
 - GPUImageStillCamera (for taking photos with the camera)
 - GPUImagePicture (for still images)
 - GPUImageMovie (for movies)

 Source objects upload still image frames to OpenGL ES as textures, then hand those textures off to the next objects in the processing chain.
 */
class GPUImageOutput : public QObject
{
	Q_OBJECT

	Q_PROPERTY(bool shouldSmoothlyScaleOutput READ shouldSmoothlyScaleOutput WRITE setShouldSmoothlyScaleOutput FINAL)
	Q_PROPERTY(bool shouldIgnoreUpdatesToThisTarget READ shouldIgnoreUpdatesToThisTarget WRITE setShouldIgnoreUpdatesToThisTarget FINAL)
	Q_PROPERTY(GPUImageInput targetToIgnoreForUpdates READ targetToIgnoreForUpdates WRITE setTargetToIgnoreForUpdates FINAL)
	Q_PROPERTY(bool enabled READ enabled FINAL)

public:
	typedef void (*frameProcessingCompletionFunc)(GPUImageOutput*,QTime&);

	GPUImageOutput();
	virtual ~GPUImageOutput();

	//Properties
	bool shouldSmoothlyScaleOutput() const;
	void setShouldSmoothlyScaleOutput(bool smooth);
	bool shouldIgnoreUpdatesToThisTarget() const;
	void setShouldIgnoreUpdatesToThisTarget(bool ignore);
	GPUImageMovieWriter* audioEncodingTarget() const;
	void setAudioEncodingTarget(GPUImageMovieWriter* movieWriter);
	GPUImageInput& targetToIgnoreForUpdates() const;
	void setTargetToIgnoreForUpdates(GPUImageInput& ignore);
	frameProcessingCompletionFunc frameProcessingCompletionBlock() const;
	bool enabled() const;

	//Managing targets
	void setInputTextureForTarget(GPUImageInput& target, int inputTextureIndex);
	GLuint textureForOutput() const;
	void notifyTargetsAboutNewOutputTexture();

	//Returns an array of the current targets.
	QListIterator<GPUImageInput*> targets() const;

	/**
	 * @brief Adds a target to receive notifications when new frames are available.
	 *
	 * The target will be asked for its next available texture.
	 *
	 * @param newTarget Target to be added
	 *
	 * @seealso GPUImageInput.newFrameReadyAtTime
	 */
	void addTarget(GPUImageInput& newTarget);

	/**
	 * @brief Adds a target to receive notifications when new frames are available.
	 *
	 * @param newTarget Target to be added
	 *
	 * @seealso GPUImageInput.newFrameReadyAtTime
	 */
	void addTarget(GPUImageInput& newTarget, int textureLocation);

	/**
	 * @brief Removes a target. The target will no longer receive notifications when new frames are available.
	 *
	 * @param targetToRemove Target to be removed
	 */
	void removeTarget(GPUImageInput& targetToRemove);

	/**
	 * @brief Removes all targets.
	 */
	void removeAllTargets();

	//Manage the output texture
	void initializeOutputTexture();
	void deleteOutputTexture();
	void forceProcessingAtSize(const QSizeF& frameSize);
	void forceProcessingAtSizeRespectingAspectRatio(const QSizeF& frameSize);

	//Still image processing
	//TODO

	void prepareForImageCapture();

protected:
	QList<GPUImageInput*> _targets;
	QList<int> targetTextureIndices;

	GLuint outputTexture;
	QSizeF inputTextureSize, cachedMaximumOutputSize, forcedMaximumSize;

	bool overrideInputSize;

private:
	/*! @cond PRIVATE */
	bool _shouldSmoothlyScaleOutput;
	bool _shouldIgnoreUpdatesToThisTarget;
	GPUImageMovieWriter* _audioEncodingTarget;
	GPUImageInput* _targetToIgnoreForUpdates;
	frameProcessingCompletionFunc _frameProcessingCompletionBlock;
	bool _enabled;

	friend class InitializeOutputTexture_Runnable;

	Q_DISABLE_COPY(GPUImageOutput)
	/*! @endcond */
};
QML_DECLARE_TYPE(GPUImageOutput)
#endif
#else
#import <UIKit/UIKit.h>

#import "GPUImageOpenGLESContext.h"

void runOnMainQueueWithoutDeadlocking(void (^block)(void));
void runSynchronouslyOnVideoProcessingQueue(void (^block)(void));
void report_memory(NSString *tag);

@class GPUImageMovieWriter;

/** GPUImage's base source object
 
 Images or frames of video are uploaded from source objects, which are subclasses of GPUImageOutput. These include:
 
 - GPUImageVideoCamera (for live video from an iOS camera) 
 - GPUImageStillCamera (for taking photos with the camera)
 - GPUImagePicture (for still images)
 - GPUImageMovie (for movies)
 
 Source objects upload still image frames to OpenGL ES as textures, then hand those textures off to the next objects in the processing chain.
 */
@interface GPUImageOutput : NSObject
{
    NSMutableArray *targets, *targetTextureIndices;
    
    GLuint outputTexture;
    CGSize inputTextureSize, cachedMaximumOutputSize, forcedMaximumSize;
    
    BOOL overrideInputSize;
}

@property(readwrite, nonatomic) BOOL shouldSmoothlyScaleOutput;
@property(readwrite, nonatomic) BOOL shouldIgnoreUpdatesToThisTarget;
@property(readwrite, nonatomic, retain) GPUImageMovieWriter *audioEncodingTarget;
@property(readwrite, nonatomic, unsafe_unretained) id<GPUImageInput> targetToIgnoreForUpdates;
@property(nonatomic, copy) void(^frameProcessingCompletionBlock)(GPUImageOutput*, CMTime);
@property(nonatomic) BOOL enabled;

/// @name Managing targets
- (void)setInputTextureForTarget:(id<GPUImageInput>)target atIndex:(NSInteger)inputTextureIndex;
- (GLuint)textureForOutput;
- (void)notifyTargetsAboutNewOutputTexture;

/** Returns an array of the current targets.
 */
- (NSArray*)targets;

/** Adds a target to receive notifications when new frames are available.
 
 The target will be asked for its next available texture.
 
 See [GPUImageInput newFrameReadyAtTime:]
 
 @param newTarget Target to be added
 */
- (void)addTarget:(id<GPUImageInput>)newTarget;

/** Adds a target to receive notifications when new frames are available.
 
 See [GPUImageInput newFrameReadyAtTime:]
 
 @param newTarget Target to be added
 */
- (void)addTarget:(id<GPUImageInput>)newTarget atTextureLocation:(NSInteger)textureLocation;

/** Removes a target. The target will no longer receive notifications when new frames are available.
 
 @param targetToRemove Target to be removed
 */
- (void)removeTarget:(id<GPUImageInput>)targetToRemove;

/** Removes all targets.
 */
- (void)removeAllTargets;

/// @name Manage the output texture

- (void)initializeOutputTexture;
- (void)deleteOutputTexture;
- (void)forceProcessingAtSize:(CGSize)frameSize;
- (void)forceProcessingAtSizeRespectingAspectRatio:(CGSize)frameSize;

/// @name Still image processing

/** Retreives the currently processed image as a UIImage.
 */
- (UIImage *)imageFromCurrentlyProcessedOutput;
- (CGImageRef)newCGImageFromCurrentlyProcessedOutput;

/** Convenience method to retreive the currently processed image with a different orientation.
 @param imageOrientation Orientation for image
 */
- (UIImage *)imageFromCurrentlyProcessedOutputWithOrientation:(UIImageOrientation)imageOrientation;
- (CGImageRef)newCGImageFromCurrentlyProcessedOutputWithOrientation:(UIImageOrientation)imageOrientation;

/** Convenience method to process an image with a filter.
 
 This method is useful for using filters on still images without building a full pipeline.
 
 @param imageToFilter Image to be filtered
 */
- (UIImage *)imageByFilteringImage:(UIImage *)imageToFilter;
- (CGImageRef)newCGImageByFilteringImage:(UIImage *)imageToFilter;
- (CGImageRef)newCGImageByFilteringCGImage:(CGImageRef)imageToFilter;
- (CGImageRef)newCGImageByFilteringCGImage:(CGImageRef)imageToFilter orientation:(UIImageOrientation)orientation;

- (void)prepareForImageCapture;

@end
#endif
