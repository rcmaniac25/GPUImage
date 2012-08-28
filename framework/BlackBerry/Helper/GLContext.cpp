#include "GLContext.h"

#include "../Helper/singleton.h"

#include <QHash>

static QHash<EGLContext, GLContext*>& getContexts()
{
	return Singleton<QHash<EGLContext, GLContext*> >::instance();
}

GLContext::GLContext(GLenum api) : setup(), ownSetup(true), surface(EGL_NO_SURFACE)
{
	if(api == EGL_OPENVG_BIT || api == EGL_OPENGL_ES_BIT || api == EGL_OPENGL_ES2_BIT)
	{
		EGLint configAtt[]= {	EGL_RED_SIZE,        8,
								EGL_GREEN_SIZE,      8,
								EGL_BLUE_SIZE,       8,
								EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
								EGL_RENDERABLE_TYPE, api,
								EGL_NONE};

		EGLint contextAtt[] = { EGL_CONTEXT_CLIENT_VERSION, 1, EGL_NONE };
		switch(api)
		{
			case EGL_OPENGL_ES2_BIT:
				contextAtt[1] = 2;
				break;
			case EGL_OPENVG_BIT:
				contextAtt[0] = EGL_NONE; //Make an "empty" list
				break;
		}

		EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if(display != EGL_NO_DISPLAY)
		{
			setup.setDisplay(display);
			if(eglInitialize(display, NULL, NULL) == EGL_TRUE && eglBindAPI(api == EGL_OPENVG_BIT ? EGL_OPENVG_API : EGL_OPENGL_ES_API) == EGL_TRUE)
			{
				EGLint numConf;
				EGLConfig config;
				if(eglChooseConfig(display, configAtt, &config, 1, &numConf) == EGL_TRUE)
				{
					setup.setContext(eglCreateContext(display, config, EGL_NO_CONTEXT, contextAtt));

					if(valid())
					{
						QHash<EGLContext, GLContext*>& cont = getContexts();
						cont[setup.context()] = this;
					}
				}
			}
		}
	}
}

GLContext::GLContext(GLSetup& set, bool own) : setup(), ownSetup(own), surface(EGL_NO_SURFACE)
{
	setup.setContext(set.context());
	setup.setDisplay(set.display());

	if(valid())
	{
		QHash<EGLContext, GLContext*>& cont = getContexts();
		cont[setup.context()] = this;
	}
}

GLContext::~GLContext()
{
	if(ownSetup)
	{
		if (setup.display() != EGL_NO_DISPLAY)
		{
			eglMakeCurrent(setup.display(), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			if (surface != EGL_NO_SURFACE)
			{
				eglDestroySurface(setup.display(), surface);
				surface = EGL_NO_SURFACE;
			}
			if (setup.context() != EGL_NO_CONTEXT)
			{
				eglDestroyContext(setup.display(), setup.context());
				setup.setContext(EGL_NO_CONTEXT);
			}
			eglTerminate(setup.display());
			setup.setDisplay(EGL_NO_DISPLAY);
		}
		eglReleaseThread();
	}
	else
	{
		surface = EGL_NO_SURFACE;
		setup.setContext(EGL_NO_CONTEXT);
		setup.setDisplay(EGL_NO_DISPLAY);
	}
}

bool GLContext::setCurrentContext(const GLContext* context)
{
	if(context)
	{
		return eglMakeCurrent(context->setup.display(), context->surface, context->surface, context->setup.context()) == EGL_TRUE;
	}
	return false;
}

bool GLContext::renderbufferStorage(EGLSurface surf)
{
	if(GLContext::currentContext() == this)
	{
		if(eglMakeCurrent(setup.display(), EGL_NO_SURFACE, EGL_NO_SURFACE, setup.context()) == EGL_TRUE)
		{
			surface = surf;
			return eglMakeCurrent(setup.display(), surface, surface, setup.context()) == EGL_TRUE;
		}
		return false;
	}
	else
	{
		surface = surf;
		return true; //Always return true, we just set it to this buffer
	}
}

bool GLContext::presentRenderbuffer()
{
	return eglSwapBuffers(setup.display(), surface) == EGL_TRUE;
}

GLenum GLContext::api() const
{
	EGLint val;
	if(eglQueryContext(setup.display(), setup.context(), EGL_CONTEXT_CLIENT_TYPE, &val) == EGL_TRUE && (val == EGL_OPENVG_API || val == EGL_OPENGL_ES_API))
	{
		if(eglQueryContext(setup.display(), setup.context(), EGL_CONTEXT_CLIENT_VERSION, &val) == EGL_TRUE)
		{
			return val == 1 ? EGL_OPENGL_ES_BIT : EGL_OPENGL_ES2_BIT;
		}
		else
		{
			return EGL_OPENVG_BIT;
		}
	}
	return 0;
}

const GLSetup& GLContext::getGLSetup() const
{
	return setup;
}

bool GLContext::valid() const
{
	return setup.display() != EGL_NO_DISPLAY && setup.context() != EGL_NO_CONTEXT;
}

GLContext* GLContext::currentContext()
{
	const QHash<EGLContext, GLContext*>& contexts = getContexts();
	return contexts.value(eglGetCurrentContext());
}
