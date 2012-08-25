#if defined(Q_OS_BLACKBERRY)
#include "GLProgram.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>

typedef void (*GLInfoFunc)(GLuint program, GLenum pname, GLint* params);
typedef void (*GLLogFunc)(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);

static bool compileShader(GLuint* shader, GLenum type, const QString& shaderString)
{
	const GLchar* source = (GLchar*)shaderString.toUtf8().constData();
	if (!source)
	{
		qDebug() << "Failed to load vertex shader";
		return false;
	}

	*shader = glCreateShader(type);
	glShaderSource(*shader, 1, &source, NULL);
	glCompileShader(*shader);

	GLint status;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE)
	{
		GLint logLength;
		glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0)
		{
			GLchar* log = (GLchar*)malloc(logLength);
			glGetShaderInfoLog(*shader, logLength, &logLength, log);
			qDebug() << QString("Shader compile log:\n%1").arg(log);
			free(log);
		}
	}

	return status == GL_TRUE;
}

static QString logForOpenGLObject(GLuint obj, GLInfoFunc infoFunc, GLLogFunc logFunc)
{
    GLint logLen = 0, charsWritten = 0;

    infoFunc(obj, GL_INFO_LOG_LENGTH, &logLen);
    if (logLen < 1)
    {
    	return "";
    }

    char* logBytes = (char*)malloc(logLen);
    logFunc(obj, logLen, &charsWritten, logBytes);
    QString log = QString::fromAscii(logBytes, logLen);
    free(logBytes);

    return log;
}


static const QString readFile(const QString& file)
{
	QFile f(file);
	if(!f.exists())
	{
		return QString();
	}

	f.open(QIODevice::ReadOnly | QIODevice::Text);

	QTextStream in(&f);
	in.setCodec("UTF-8");

	return in.readAll();
}

GLProgram::GLProgram(const QString& vShader, const QString& fShader, GLProgram::Type type) : attributes(), uniforms(), program(0), vertShader(0), fragShader(0), _initialized(false)
{
	QString veShader = vShader;
	QString fsShader = fShader;
	if(type == GLProgram::VertFileFragFile)
	{
		const QString& shader = readFile(vShader);
		if(shader.isNull())
		{
			return;
		}
		veShader = shader;
	}
	if(type == GLProgram::VertFileFragFile || type == GLProgram::VertStringFragFile)
	{
		const QString& shader = readFile(fShader);
		if(shader.isNull())
		{
			return;
		}
		fsShader = shader;
	}

	program = glCreateProgram();

	if (!compileShader(&vertShader, GL_VERTEX_SHADER, veShader))
	{
		qDebug() << "Failed to compile vertex shader.";
		goto COMPILE_ERROR;
	}

	if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, fsShader))
	{
		qDebug() << "Failed to compile fragment shader.";
		goto COMPILE_ERROR;
	}

	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);

COMPILE_ERROR:
	glDeleteProgram(program);
	program = 0;
}

GLProgram::~GLProgram()
{
	if (vertShader != 0)
	{
		glDeleteShader(vertShader);
	}

	if (fragShader != 0)
	{
		glDeleteShader(fragShader);
	}

	if (program != 0)
	{
		glDeleteProgram(program);
	}
}

bool GLProgram::initialized() const
{
	return _initialized;
}

void GLProgram::setInitialized(bool initialized)
{
	//Fail to see why this should be here
	_initialized = initialized;
}

void GLProgram::addAttribute(const QString& attributeName)
{
	if(!attributes.contains(attributeName))
	{
		attributes << attributeName;
		glBindAttribLocation(program, attributes.indexOf(attributeName), attributeName.toUtf8().constData());
	}
}

GLuint GLProgram::attributeIndex(const QString& attributeName)
{
	return attributes.indexOf(attributeName);
}

GLuint GLProgram::uniformIndex(const QString& uniformName)
{
	return glGetUniformLocation(program, uniformName.toUtf8().constData());
}

bool GLProgram::link()
{
	glLinkProgram(program);
	glValidateProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		return false;
	}

	if (vertShader)
	{
		glDeleteShader(vertShader);
		vertShader = 0;
	}

	if (fragShader)
	{
		glDeleteShader(fragShader);
		fragShader = 0;
	}

	_initialized = true;
	return true;
}

void GLProgram::use()
{
	glUseProgram(program);
}

void GLProgram::validate()
{
	glValidateProgram(program);

	GLint logLen;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
	if (logLen > 0)
	{
		GLchar* log = (GLchar*)malloc(logLen);
		glGetProgramInfoLog(program, logLen, &logLen, log);
		qDebug() << QString("Program validate log:\n%1").arg(log);
		free(log);
	}
}

const QString GLProgram::vertexShaderLog()
{
	return logForOpenGLObject(vertShader, (GLInfoFunc)&glGetProgramiv, (GLLogFunc)&glGetProgramInfoLog);
}

const QString GLProgram::fragmentShaderLog()
{
	return logForOpenGLObject(fragShader, (GLInfoFunc)&glGetProgramiv, (GLLogFunc)&glGetProgramInfoLog);
}

const QString GLProgram::programLog()
{
	return logForOpenGLObject(program, (GLInfoFunc)&glGetProgramiv, (GLLogFunc)&glGetProgramInfoLog);
}
#endif
