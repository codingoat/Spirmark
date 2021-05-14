#include "ShaderObject.h"

#include <iostream>
#include <fstream>

ShaderObject::ShaderObject(GLenum pType)
{
	m_id = glCreateShader(pType);
}

ShaderObject::~ShaderObject()
{
	if (m_id != 0)
	{
		glDeleteShader(m_id);
		m_id = 0;
	}
}

ShaderObject::ShaderObject(ShaderObject &&rhs)
{
	m_id = rhs.m_id;
	rhs.m_id = 0;
}

ShaderObject & ShaderObject::operator=(ShaderObject &&rhs)
{
	if (&rhs == this)
		return *this;

	m_id = rhs.m_id;
	rhs.m_id = 0;

	return *this;
}

ShaderObject::ShaderObject(GLenum pType, const std::string &pFilenameOrSource, bool spirv)
{
	if (spirv)
	{
		m_id = glCreateShader(Spirver::StageToGlsl(pType));
		Spirver::SpirvShader spirv = Spirver::SpirvShader::FromFile(pFilenameOrSource, Spirver::StageToSpirver(pType));
		spirv.Compile(m_id);
	}
	else {
        m_id = glCreateShader(pType);

        if (!FromFile(pType, pFilenameOrSource.c_str())) {
            if (!std::ifstream(pFilenameOrSource).good())
                FromMemory(pType, pFilenameOrSource);
        }
    }
}

ShaderObject::ShaderObject(Spirver::Stage pType, const std::vector<char>& spirv)
{
	m_id = glCreateShader(Spirver::StageToGlsl(pType));
	Spirver::proc::spirvToShader(spirv, pType, m_id);
}

ShaderObject::ShaderObject(Spirver::Stage pType, const std::vector<GLuint>& spirv)
{
	m_id = glCreateShader(Spirver::StageToGlsl(pType));
	Spirver::proc::spirvToShader(spirv, pType, m_id);
}

ShaderObject::ShaderObject(Spirver::GlslShader& shader)
{
	m_id = glCreateShader(Spirver::StageToGlsl(shader.GetStage()));
	shader.Compile(m_id);
}

ShaderObject::ShaderObject(Spirver::SpirvShader& shader)
{
	m_id = glCreateShader(Spirver::StageToGlsl(shader.GetStage()));
	shader.Compile(m_id);
}

bool ShaderObject::FromFile(GLenum _shaderType, const char* _filename)
{
	// _fileName megnyitasa
	std::ifstream shaderStream(_filename);

	if (!shaderStream.is_open())
		return false;

	// shaderkod betoltese _fileName fajlbol
	std::string shaderCode = "";

	// file tartalmanak betoltese a shaderCode string-be
	std::string line = "";
	while (std::getline(shaderStream, line))
		shaderCode += line + "\n";

	shaderStream.close();

	// t�rj�nk vissza a ford�t�s eredm�ny�vel
	return CompileShaderFromMemory(m_id, shaderCode) > 0;
}


bool ShaderObject::FromMemory(GLenum _shaderType, const std::string& _source)
{
	CompileShaderFromMemory(m_id, _source);
	if (m_id != 0)
		return true;
	else
		return false;
}

bool ShaderObject::FromMemory(Spirver::Stage _shaderType, const std::vector<char>& spirv)
{
	return Spirver::proc::spirvToShader(spirv, _shaderType, m_id);
}

bool ShaderObject::FromMemory(Spirver::Stage _shaderType, const std::vector<GLuint>& spirv)
{
	return Spirver::proc::spirvToShader(spirv, _shaderType, m_id);
}


GLuint ShaderObject::CompileShaderFromMemory(const GLuint _shaderObject, const std::string& _source)
{
	// betoltott kod hozzarendelese a shader-hez
	const char* sourcePointer = _source.c_str();
	glShaderSource(_shaderObject, 1, &sourcePointer, nullptr);

	// shader leforditasa
	glCompileShader(_shaderObject);

	// ellenorizzuk, h minden rendben van-e
	GLint result = GL_FALSE;
	int infoLogLength;

	// forditas statuszanak lekerdezese
	glGetShaderiv(_shaderObject, GL_COMPILE_STATUS, &result);
	glGetShaderiv(_shaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (GL_FALSE == result)
	{
		GLchar* error = new char[infoLogLength];
		glGetShaderInfoLog(_shaderObject, infoLogLength, nullptr, error);

		std::cerr << "Hiba: " << error << std::endl;

		delete[] error;

		return 0;
	}

	return _shaderObject;
}

ShaderObject ShaderObject::LoadGlslAsSpirv(const std::string &path, Spirver::Stage pType)
{
    std::vector<GLuint> spirv;
    Spirver::proc::glslToSpirv(Spirver::proc::fileToString(path.c_str()), pType, spirv);
    return ShaderObject(pType, spirv);
}
