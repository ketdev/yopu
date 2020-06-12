#include "render.h"

// Shader sources
static const char* vertexSource =
	" #version 330 core                      \n"
	"                                        \n"
	" layout(location = 0) in vec4 position; \n"
	"                                        \n"
	" void main(){                           \n"
	"     gl_Position.xyz = position.xyz;    \n"
	"     gl_Position.w = 1.0;               \n"
	"                                        \n"
	" }                                      \n";
static const char* fragmentSource =
	" #version 330 core                      \n"
	"                                        \n"
	" out vec4 color;                        \n"
	"                                        \n"
	" void main() {                          \n"
	"     color = vec4(1.0, 1.0, 1.0, 1.0);  \n"
	" }                                      \n";

Render::Render() {
	// Compile and link the vertex and fragment shader into a shader program
	_program.link({
		Shader(Shader::Type::Vertex, vertexSource),
		Shader(Shader::Type::Fragment, fragmentSource) });

	// Copy the vertex data to the Vertex Buffer Object
	float vertices[] = {
		0.0f, 0.5f,
		0.5f, -0.5f,
		-0.5f, -0.5f
	};
	_vbo.set(vertices, 2, 3);

	// Specify the layout of the vertex data in the Vertex Array Object
	_vao.setAttrib(_program.getAttribLocation("position"), _vbo, false, 0, nullptr);
}

void Render::draw() {
	// Draw a triangle from the 3 vertices
	_program.use();
	_vao.drawTriangles(0, 3);
}