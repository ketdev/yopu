#include "render.h"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
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
" uniform vec3 spriteColor;													\n"
" 																			\n"
" void main() {																\n"
" 	color = vec4(spriteColor, 1.0) * texture(image, TexCoords);				\n"
" }																			\n";

Render::Render() {
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

	// Load our texture into memory
	_texture.load("src/assets/puyos.png");
}

void Render::draw() {
	// Draw a triangle from the 3 vertices
	_program.use();

	//--
	// Texture2D &texture, glm::vec2 position, glm::vec2 size, float rotate, glm::vec3 color
	glm::vec2 size = { 512, 512 };
	glm::vec2 position = { 10, 10 };
	float rotate = 0;
	glm::vec3 color = { 1, 1, 1 };
	//--

	const int SCREEN_WIDTH = 2688; // 1080;
	const int SCREEN_HEIGHT = 1242; // 1920;
	glm::mat4 projection = glm::ortho(0.0f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.0f, -1.0f, 1.0f);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position, 0.0f));

	// rotate around center
	model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
	model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

	model = glm::scale(model, glm::vec3(size, 1.0f));

	_program.setMatrix4(_program.getUniformLocation("model"), model);
	_program.setMatrix4(_program.getUniformLocation("projection"), projection);
	_program.setVector3f(_program.getUniformLocation("spriteColor"), color);
	_texture.use();

	_vao.drawTriangleStrips(0, 4);
}