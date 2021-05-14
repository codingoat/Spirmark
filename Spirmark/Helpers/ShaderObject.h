#pragma once

#include <GL/glew.h>
#include <GL/gl.h>

#include <string>
#include <utility>

#include <Spirver.h>

class ShaderObject final
{
public:
	ShaderObject(GLenum pType);

	ShaderObject(GLenum pType, const std::string&, bool spirv = false);
	ShaderObject(Spirver::Stage pType, const std::vector<char>& spirv);
	ShaderObject(Spirver::Stage pType, const std::vector<GLuint>& spirv);

	ShaderObject(Spirver::GlslShader& shader);
	ShaderObject(Spirver::SpirvShader& shader);

	~ShaderObject();

	ShaderObject(const ShaderObject&)				= delete;
	ShaderObject& operator=(const ShaderObject&)	= delete;

	ShaderObject(ShaderObject&&);
	ShaderObject& operator=(ShaderObject&&);

	operator unsigned int() const { return m_id; }

	bool FromFile(GLenum _shaderType, const char* _filename);
	bool FromMemory(GLenum _shaderType, const std::string& _source);
	bool FromMemory(Spirver::Stage _shaderType, const std::vector<char>& spirv);
	bool FromMemory(Spirver::Stage _shaderType, const std::vector<GLuint>& spirv);

	static ShaderObject LoadGlslAsSpirv(const std::string& path, Spirver::Stage pType);
private:
	GLuint	CompileShaderFromMemory(const GLuint _shaderObject, const std::string& _source);

	GLuint	m_id;
};