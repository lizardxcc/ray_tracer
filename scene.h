#ifndef SCENE_H
#define SCENE_H

#include <GL/gl3w.h>    // Initialize with gl3wInit()
#include <glm/glm.hpp>
#include "obj.h"

class Scene {
	public:
		Scene(void);
		void Render(void);
		glm::vec3 cameraPos;
		glm::vec3 cameraFront;
		glm::vec3 cameraUp;
		float d, focal_length, aperture;
	private:
		std::vector<GLuint> VAOs;
		std::vector<float *>vertices_array;
		std::vector<int> vertices_num;
		std::vector<std::array<float, 3>> colors;
		GLuint rbo;
		GLuint fbo;
		GLuint texture;
		GLuint shaderProgram;
		const char *vertexShaderSource = "#version 330 core\n"
			"layout (location = 0) in vec3 aPos;\n"
			"layout (location = 1) in vec3 aNormal;\n"
			"out vec3 Normal;\n"
			"out vec3 FragPos;\n"
			"uniform mat3 normalmodel;\n"
			"uniform mat4 model;\n"
			"uniform mat4 view;\n"
			"uniform mat4 projection;\n"
			"void main()\n"
			"{\n"
			"	Normal = normalize(normalmodel * aNormal);\n"
			"	FragPos = vec3(model * vec4(aPos, 1.0));\n"
			"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
			"}\0";

		const char *fragmentShaderSource = "#version 330 core\n"
			"in vec3 Normal;\n"
			"in vec3 FragPos;\n"
			"out vec4 FragColor;\n"
			"uniform vec3 lightColor;\n"
			"uniform vec3 objectColor;\n"
			"void main()\n"
			"{\n"
			"	vec3 lightPos = vec3(0.0f, 0.0f, 1.0f);\n"
			"	float ambientStrength = 0.3;\n"
			"	vec3 ambient = ambientStrength * lightColor;\n"
			"	float diff = max(dot(Normal, normalize(lightPos - FragPos)), 0.0);\n"
			"	vec3 diffuse = diff * lightColor;\n"
			"	FragColor = vec4((ambient + diffuse) * objectColor, 1.0f);"
			"}\n\0";

		obj obj_loader;
		float pitch = 0.0f, yaw = -90.0f;
		unsigned int activeObjectIndex = 0;

};

#endif
