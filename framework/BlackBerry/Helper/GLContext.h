#ifndef GLCONTEXT_H
#define GLCONTEXT_H

#include <qdeclarative.h>
#include <QObject>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

class GLSetup : public QObject
{
	Q_OBJECT

public:
	GLSetup(EGLContext con=EGL_NO_CONTEXT, EGLDisplay disp=EGL_NO_DISPLAY):_context(con),_display(disp){}
	~GLSetup(){}

	inline EGLContext context() const{return _context;}
	inline void setContext(EGLContext context){_context=context;}
	inline EGLDisplay display() const{return _display;}
	inline void setDisplay(EGLDisplay display){_display=display;}

protected:
	EGLContext _context;
	EGLDisplay _display;

private:
	/*! @cond PRIVATE */
	Q_DISABLE_COPY(GLSetup)
	/*! @endcond */
};
QML_DECLARE_TYPE(GLSetup)

class GLContext : public QObject
{
	Q_OBJECT

public:
	GLContext(GLenum api);
	GLContext(GLSetup& setup, bool own=false);
	virtual ~GLContext();

	static bool setCurrentContext(const GLContext* context);

	bool renderbufferStorage(EGLSurface surface);
	Q_INVOKABLE bool presentRenderbuffer();

	GLenum api() const;
	const GLSetup& getGLSetup() const;
	bool valid() const;

	static GLContext* currentContext();

private:
	/*! @cond PRIVATE */
	GLSetup setup;
	bool ownSetup;
	EGLSurface surface;

	Q_DISABLE_COPY(GLContext)
	/*! @endcond */
};
QML_DECLARE_TYPE(GLContext)
#endif
