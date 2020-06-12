#pragma once
#include <string>

struct Shader {
	enum class Type {
		Vertex,
		Fragment
	};
	struct CompileError {
		std::string msg;
	};

	const uint32_t id;

	Shader(Type type, const std::string& code);
	~Shader();
};

struct ShaderProgram {
	struct LinkError {
		std::string msg;
	};

	const uint32_t id;

	ShaderProgram();
	~ShaderProgram();
	void link(std::initializer_list<Shader> shaders);
	void use();

	uint32_t getAttribLocation(const std::string& name);
};