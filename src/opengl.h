#pragma once

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>
#include<stb/stb_image.h>
#include<string>
#include<vector>
#include<fstream>
#include<iostream>
#include<cerrno>

// ─────────────────────────────────────────────
// Vertex
// ─────────────────────────────────────────────
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texUV;
};

// ─────────────────────────────────────────────
// VBO
// ─────────────────────────────────────────────
class VBO
{
public:
	GLuint ID;
	VBO(std::vector<Vertex>& v)
	{
		glGenBuffers(1, &ID);
		glBindBuffer(GL_ARRAY_BUFFER, ID);
		glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(Vertex), v.data(), GL_STATIC_DRAW);
	}
	void Bind()   { glBindBuffer(GL_ARRAY_BUFFER, ID); }
	void Unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
	void Delete() { glDeleteBuffers(1, &ID); }
};

// ─────────────────────────────────────────────
// EBO
// ─────────────────────────────────────────────
class EBO
{
public:
	GLuint ID;
	EBO(std::vector<GLuint>& idx)
	{
		glGenBuffers(1, &ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);
	}
	void Bind()   { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); }
	void Unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
	void Delete() { glDeleteBuffers(1, &ID); }
};

// ─────────────────────────────────────────────
// VAO
// ─────────────────────────────────────────────
class VAO
{
public:
	GLuint ID;
	VAO() { glGenVertexArrays(1, &ID); }
	void LinkAttrib(VBO& vbo, GLuint layout, GLuint numComp, GLenum type, GLsizeiptr stride, void* offset)
	{
		vbo.Bind();
		glVertexAttribPointer(layout, numComp, type, GL_FALSE, stride, offset);
		glEnableVertexAttribArray(layout);
		vbo.Unbind();
	}
	void Bind()   { glBindVertexArray(ID); }
	void Unbind() { glBindVertexArray(0); }
	void Delete() { glDeleteVertexArrays(1, &ID); }
};

// ─────────────────────────────────────────────
// Shader
// ─────────────────────────────────────────────
class Shader
{
public:
	GLuint ID;
	Shader(const char* vertFile, const char* fragFile)
	{
		auto src = [](const char* path) {
			std::ifstream f(path, std::ios::binary);
			if (!f) throw(errno);
			std::string s; f.seekg(0, std::ios::end); s.resize(f.tellg());
			f.seekg(0); f.read(&s[0], s.size()); return s;
		};
		auto check = [](GLuint id, const char* type) {
			GLint ok; char log[1024];
			if (std::string(type) != "PROGRAM") {
				glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
				if (!ok) { glGetShaderInfoLog(id, 1024, NULL, log); std::cout << "COMPILE_ERROR:" << type << "\n" << log << "\n"; }
			} else {
				glGetProgramiv(id, GL_LINK_STATUS, &ok);
				if (!ok) { glGetProgramInfoLog(id, 1024, NULL, log); std::cout << "LINK_ERROR\n" << log << "\n"; }
			}
		};
		std::string vs = src(vertFile), fs = src(fragFile);
		const char* vp = vs.c_str(), *fp = fs.c_str();

		GLuint vert = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vert, 1, &vp, NULL); glCompileShader(vert); check(vert, "VERTEX");

		GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag, 1, &fp, NULL); glCompileShader(frag); check(frag, "FRAGMENT");

		ID = glCreateProgram();
		glAttachShader(ID, vert); glAttachShader(ID, frag);
		glLinkProgram(ID); check(ID, "PROGRAM");
		glDeleteShader(vert); glDeleteShader(frag);
	}
	void Activate() { glUseProgram(ID); }
	void Delete()   { glDeleteProgram(ID); }
};

// ─────────────────────────────────────────────
// Texture
// ─────────────────────────────────────────────
class Texture
{
public:
	GLuint ID;
	const char* type;
	GLuint unit;

	Texture(const char* image, const char* texType, GLuint slot, GLenum format, GLenum pixelType)
		: type(texType), unit(slot)
	{
		int w, h, ch;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* bytes = stbi_load(image, &w, &h, &ch, 0);
		if (!bytes) {
			std::cout << "image!fail!!: " << image << std::endl;
		}
		glGenTextures(1, &ID);
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, format, pixelType, bytes);
		//glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(bytes);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void texUnit(Shader& shader, const char* uniform, GLuint u)
	{
		shader.Activate();
		glUniform1i(glGetUniformLocation(shader.ID, uniform), u);
	}
	void Bind()   { glActiveTexture(GL_TEXTURE0 + unit); glBindTexture(GL_TEXTURE_2D, ID); }
	void Unbind() { glBindTexture(GL_TEXTURE_2D, 0); }
	void Delete() { glDeleteTextures(1, &ID); }
};

// ─────────────────────────────────────────────
// Camera
// ─────────────────────────────────────────────
class Camera
{
public:
	glm::vec3 Position;
	glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up          = glm::vec3(0.0f, 1.0f,  0.0f);
	glm::mat4 cameraMatrix = glm::mat4(1.0f);
	bool firstClick = true;
	int width, height;
	float speed = 0.1f, sensitivity = 100.0f;

	Camera(int w, int h, glm::vec3 pos) : width(w), height(h), Position(pos) {}

	void updateMatrix(float fov, float near, float far)
	{
		cameraMatrix = glm::perspective(glm::radians(fov), (float)width / height, near, far)
		             * glm::lookAt(Position, Position + Orientation, Up);
	}
	void Matrix(Shader& shader, const char* uniform)
	{
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
	}
	void Inputs(GLFWwindow* window)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) Position += speed * Orientation;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) Position -= speed * Orientation;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) Position -= speed * glm::normalize(glm::cross(Orientation, Up));
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) Position += speed * glm::normalize(glm::cross(Orientation, Up));
		if (glfwGetKey(window, GLFW_KEY_SPACE)        == GLFW_PRESS) Position += speed * Up;
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) Position -= speed * Up;
		speed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 0.4f : 0.1f;

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			if (firstClick) { glfwSetCursorPos(window, width / 2, height / 2); firstClick = false; }

			double mx, my; glfwGetCursorPos(window, &mx, &my);
			float rotX = sensitivity * (float)(my - height / 2) / height;
			float rotY = sensitivity * (float)(mx - width  / 2) / width;

			glm::vec3 newOri = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));
			if (abs(glm::angle(newOri, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
				Orientation = newOri;
			Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);
			glfwSetCursorPos(window, width / 2, height / 2);
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			firstClick = true;
		}
	}
};

// ─────────────────────────────────────────────
// Mesh
// ─────────────────────────────────────────────
class Mesh
{
public:
	std::vector<Vertex>  vertices;
	std::vector<GLuint>  indices;
	std::vector<Texture> textures;
	VAO VAO;

	Mesh(std::vector<Vertex>& v, std::vector<GLuint>& idx, std::vector<Texture>& tex)
		: vertices(v), indices(idx), textures(tex)
	{
		VAO.Bind();
		VBO vbo(v); EBO ebo(idx);
		VAO.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
		VAO.LinkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
		VAO.LinkAttrib(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
		VAO.LinkAttrib(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
		VAO.Unbind(); vbo.Unbind(); ebo.Unbind();
	}

	void Draw(Shader& shader, Camera& camera)
	{
		shader.Activate();
		VAO.Bind();

		// 텍스처 바인딩 및 유니폼 설정
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			std::string n = std::to_string(i); // 유니폼 이름을 diffuse0, diffuse1 등으로 통일
			textures[i].texUnit(shader, ("diffuse" + n).c_str(), i);
			textures[i].Bind();
		}

		// 조명과 카메라 정보 전달
		glUniform4f(glGetUniformLocation(shader.ID, "lightColor"), 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		camera.Matrix(shader, "camMatrix");

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		VAO.Unbind(); // 안전한 해제
	}
};
