#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#include <glm/glm.hpp>
#include "renderer.h"


class ImgViewer {
	public:
		void Render(void);
		void LoadImage(const std::shared_ptr<const double[]>& img, int width, int height);
	private:
		GLuint opengl_texture;
		GLubyte *glimg = nullptr;
		int width;
		int height;
};


class ImgRetouch {
	public:
		void Render(void);
		void LoadImage(std::shared_ptr<double[]>& img, int width, int height);
		std::shared_ptr<double[]> orig_img;
		std::shared_ptr<double[]> retouched;
		int width;
		int height;
	private:
		ImgViewer original_viewer;
		ImgViewer retouched_viewer;
		double sigma_d = 1.0;
		double sigma_r = 1.0;
		uint64_t window = 2;
};

class RetouchWindow {
	public:
		void Render(void);
		void AddImage(std::shared_ptr<double[]>& img, int width, int height);
		void AddImage(std::string& name, std::shared_ptr<double[]>& img, int width, int height);
	private:
		std::vector<std::string> img_names;
		std::vector<ImgRetouch> tabs;
};


class Scene {
	public:
		Scene(void);
		void RenderImage(int nx, int ny, int ns, const char *filename);

		void RenderSceneWindow(void);
		void RenderPreviewWindow(void);
		void RenderMaterialEditorWindow(void);

		RetouchWindow retouch_window;
	private:
		void Load(const char *objfilename, const char *matfilename);
		void ClearData(void);
		void RenderScene(void);
		bool scene_loaded = false;
		Renderer renderer;
		glm::vec3 cameraPos;
		glm::vec3 cameraFront;
		glm::vec3 cameraUp;
		float d, focal_length, aperture;
		unsigned int activeObjectIndex = 0;
		GLubyte *img = nullptr;
		int img_width = 500;
		int img_height = 500;
		int img_Samples = 100;
		bool img_loaded = false;
		GLuint my_opengl_texture;


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

		float pitch = 0.0f, yaw = -90.0f;

};

#endif
