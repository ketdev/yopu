#pragma once
#include <string>
#include <glm/mat4x4.hpp>

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
	uint32_t getUniformLocation(const std::string& name);

	void setVector3f(uint32_t location, glm::vec3 vec);
	void setVector4f(uint32_t location, glm::vec4 vec);
	void setMatrix4(uint32_t location, glm::mat4x4 mat);
};