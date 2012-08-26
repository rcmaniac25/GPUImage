#if defined(Q_OS_BLACKBERRY)

#ifndef GLPROGRAM_H
#define GLPROGRAM_H

#include <QObject>

#include <qdeclarative.h>
#include <QList>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

class GLProgram : public QObject
{
	Q_OBJECT

	Q_PROPERTY(bool initialized READ initialized WRITE setInitialized FINAL)

public:
	enum Type
	{
		VertStringFragString,
		VertStringFragFile,
		VertFileFragFile
	};

	GLProgram(const QString& vShader, const QString& fShader, Type paramType = GLProgram::VertStringFragString);
	virtual ~GLProgram();

	bool initialized() const;
	void setInitialized(bool initialized);

	void addAttribute(const QString& attributeName);
	GLuint attributeIndex(const QString& attributeName);
	GLuint uniformIndex(const QString& uniformName);

	bool link();
	void use();
	void validate();

	const QString vertexShaderLog();
	const QString fragmentShaderLog();
	const QString programLog();

protected:
	QList<QString> attributes;
	QList<QString> uniforms;
	GLuint program, vertShader, fragShader;

private:
	/*! @cond PRIVATE */
	bool _initialized;

	Q_DISABLE_COPY(GLProgram)
	/*! @endcond */
};
QML_DECLARE_TYPE(GLProgram)
#endif
#else
//  This is Jeff LaMarche's GLProgram OpenGL shader wrapper class from his OpenGL ES 2.0 book.
//  A description of this can be found at his page on the topic:
//  http://iphonedevelopment.blogspot.com/2010/11/opengl-es-20-for-ios-chapter-4.html
//  I've extended this to be able to take programs as NSStrings in addition to files, for baked-in shaders

#import <Foundation/Foundation.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

@interface GLProgram : NSObject
{
    NSMutableArray  *attributes;
    NSMutableArray  *uniforms;
    GLuint          program,
	vertShader,
	fragShader;
}

@property(readwrite, nonatomic) BOOL initialized;

- (id)initWithVertexShaderString:(NSString *)vShaderString
            fragmentShaderString:(NSString *)fShaderString;
- (id)initWithVertexShaderString:(NSString *)vShaderString
          fragmentShaderFilename:(NSString *)fShaderFilename;
- (id)initWithVertexShaderFilename:(NSString *)vShaderFilename
            fragmentShaderFilename:(NSString *)fShaderFilename;
- (void)addAttribute:(NSString *)attributeName;
- (GLuint)attributeIndex:(NSString *)attributeName;
- (GLuint)uniformIndex:(NSString *)uniformName;
- (BOOL)link;
- (void)use;
- (NSString *)vertexShaderLog;
- (NSString *)fragmentShaderLog;
- (NSString *)programLog;
- (void)validate;
@end
#endif
