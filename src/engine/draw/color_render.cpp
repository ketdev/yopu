#include "color_render.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../utils/xgl.h"

// Shader sources
static const char* vertexSource =
" #version 330 core															\n"
" layout (location = 0) in vec2 vertex;										\n"
" 																			\n"
" uniform mat4 model;														\n"
" uniform mat4 projection;													\n"
" 																			\n"
" void main() {																\n"
" 	gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);			\n"
" }																			\n";
static const char* fragmentSource =
" #version 330 core															\n"
" out vec4 color;															\n"
" uniform vec4 tint;														\n"
" 																			\n"
" void main() {																\n"
" 	color = tint;															\n"
" }																			\n";

ColorRender::ColorRender() : _projection(glm::mat4(1.0f)) {
	// Compile and link the vertex and fragment shader into a shader program
	_program.link({
		Shader(Shader::Type::Vertex, vertexSource),
		Shader(Shader::Type::Fragment, fragmentSource) });

	// Copy the vertex data to the Vertex Buffer Object
	float vertices[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
	};
	_vbo.set(vertices, 2, 6);

	// Specify the layout of the vertex data in the Vertex Array Object
	_vao.setAttrib(_program.getAttribLocation("vertex"), _vbo, false, 0, nullptr);
}

void ColorRender::update(int width, int height) {
	// recompute projection matrix
	_projection = glm::ortho(0.0f, float(width), float(height), 0.0f, -1.0f, 1.0f);
	_program.setMatrix4(_program.getUniformLocation("projection"), _projection);
}

void ColorRender::draw(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
	_program.use();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos.x, pos.y, 0.0f));
	model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

	_program.setMatrix4(_program.getUniformLocation("model"), model);
	_program.setVector4f(_program.getUniformLocation("tint"), color);

	_vao.drawTriangleStrips(0, 4);
}
