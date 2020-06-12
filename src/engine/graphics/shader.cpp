#include "shader.h"
#include "../utils/xgl.h"
#include <vector>

static uint32_t glShaderType(Shader::Type type) {
	switch (type) {
	case Shader::Type::Vertex:
		return GL_VERTEX_SHADER;
	case Shader::Type::Fragment:
		return GL_FRAGMENT_SHADER;
	}
	return 0;
}

Shader::Shader(Type type, const std::string& code)
	: id(glCreateShader(glShaderType(type))) {
	const char* c_code = code.c_str();
	glShaderSource(id, 1, &c_code, NULL);
	glCompileShader(id);

	// check shader
	GLint result = GL_FALSE;
	int len;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		std::vector<char> msg(len + 1);
		glGetShaderInfoLog(id, len, NULL, &msg[0]);
		if (msg[0] != 0) {
			auto str = std::string(msg.begin(), msg.end());
			throw Shader::CompileError{ str };
		}
	}
}
Shader::~Shader() {
	glDeleteShader(id);
}

ShaderProgram::ShaderProgram() : id(glCreateProgram()) {}
ShaderProgram::~ShaderProgram() { glDeleteProgram(id); }

void ShaderProgram::link(std::initializer_list<Shader> shaders) {
	for (Shader shader : shaders)
		glAttachShader(id, shader.id);

	glLinkProgram(id);

	for (Shader shader : shaders)
		glDetachShader(id, shader.id);

	// check program
	GLint result = GL_FALSE;
	int len;
	glGetProgramiv(id, GL_COMPILE_STATUS, &result);
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		std::vector<char> msg(len + 1);
		glGetShaderInfoLog(id, len, NULL, &msg[0]);
		if (msg[0] != 0) {
			auto str = std::string(msg.begin(), msg.end());
			for (Shader shader : shaders)
				glDetachShader(id, shader.id);
			throw ShaderProgram::LinkError{ str };
		}
	}
}

void ShaderProgram::use() {
	glUseProgram(id);
}

uint32_t ShaderProgram::getAttribLocation(const std::string& name) {
	return glGetAttribLocation(id, name.c_str());
}