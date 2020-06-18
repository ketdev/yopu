#include "sprite_render.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../utils/xgl.h"

// Shader sources
static const char* vertexSource =
" #version 330 core															\n"
" layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>	\n"
" out vec2 TexCoords;														\n"
" 																			\n"
" uniform mat4 model;														\n"
" uniform mat4 projection;													\n"
" 																			\n"
" void main() {																\n"
" 	TexCoords = vertex.zw;													\n"
" 	gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);			\n"
" }																			\n";
static const char* fragmentSource =
" #version 330 core															\n"
" in vec2 TexCoords;														\n"
" out vec4 color;															\n"
" 																			\n"
" uniform sampler2D image;													\n"
" uniform vec3 tint;														\n"
" 																			\n"
" void main() {																\n"
" 	color = vec4(tint, 1.0) * texture(image, TexCoords);					\n"
" }																			\n";

SpriteRender::SpriteRender() : _projection(glm::mat4(1.0f)) {	
	// Compile and link the vertex and fragment shader into a shader program
	_program.link({
		Shader(Shader::Type::Vertex, vertexSource),
		Shader(Shader::Type::Fragment, fragmentSource) });

	// Copy the vertex data to the Vertex Buffer Object
	float vertices[] = {
		// pos      // tex
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
	};
	_vbo.set(vertices, 4, 6);

	// Specify the layout of the vertex data in the Vertex Array Object
	_vao.setAttrib(_program.getAttribLocation("vertex"), _vbo, false, 0, nullptr);
}

void SpriteRender::update(int width, int height) {
	// recompute projection matrix
	_projection = glm::ortho(0.0f, float(width), float(height), 0.0f, -1.0f, 1.0f);
	_program.setMatrix4(_program.getUniformLocation("projection"), _projection);
}

void SpriteRender::draw(const Texture& texture, const Rect& src, const Rect& dst, const glm::vec3& tint) {
	_program.use();
	texture.use();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(dst.x, dst.y, 0.0f));
	model = glm::scale(model, glm::vec3(dst.w, dst.h, 1.0f));

	_program.setMatrix4(_program.getUniformLocation("model"), model);
	_program.setVector3f(_program.getUniformLocation("tint"), tint);

	float u0 = src.x / texture.width;
	float u1 = (src.x + src.w) / texture.width;
	float v0 = src.y / texture.height;
	float v1 = (src.y + src.h) / texture.height;
	float vertices[] = {
		// pos      // tex
		0.0f, 0.0f, u0, v0,
		0.0f, 1.0f, u0, v1,
		1.0f, 0.0f, u1, v0,
		1.0f, 1.0f, u1, v1,
	};
	_vbo.update(0, vertices, sizeof(vertices));

	_vao.drawTriangleStrips(0, 4);
}
