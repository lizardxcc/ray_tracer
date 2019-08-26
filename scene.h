#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#include <glm/glm.hpp>
#include "renderer.h"
#include "materialnode.h"


extern uint8_t *env_mapping_texture;
extern int env_mapping_width, env_mapping_height, env_mapping_bpp;
extern double env_brightness;


class ImgViewer {
	public:
		void Render(void);
		void LoadImage(const std::vector<double>& img, int width, int height);
	private:
		GLuint opengl_texture;
		GLubyte *glimg = nullptr;
		int width;
		int height;
};


class ImgRetouch {
	public:
		void Render(void);
		void LoadImage(const std::vector<double>& img, int width, int height);
		std::vector<double> orig_img;
		std::vector<double> retouched;
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
		void AddImage(const std::vector<double>& img, int width, int height, const char *name = "");
	private:
		std::vector<std::string> img_names;
		std::vector<ImgRetouch> tabs;
};


struct Vertex {
	GLfloat xyz[3];
	GLfloat rgb[3];
};

class Scene {
	public:
		explicit Scene(void);

		void RenderSceneWindow(void);
		void RenderPreviewWindow(void);
		void RenderMaterialEditorWindow(void);
		void RenderMaterialNodeEditorWindow(void);
		void RenderLog(void);

		RetouchWindow retouch_window;
	private:
		void LoadProject(const char *path);
		void LoadModel(const char *obj_path);
		void LoadEnvTexture(const char *path);
		void LoadMaterial(const char *path);
		void WriteMaterial(const char *path);
		void ClearData(void);
		void RenderScene(void);

		void OpenGLInitShader(void);
		void OpenGLLoadModel(void);
		bool scene_loaded = false;
		Renderer renderer;
		glm::vec3 cameraPos;
		glm::vec3 cameraFront;
		glm::vec3 cameraUp;
		float d = 0.45, focal_length=0.4, aperture = 0.0;
		float vfov = 30;
		unsigned int activeObjectIndex = 0;
		GLubyte *img = nullptr;
		int img_width = 500;
		int img_height = 500;
		int img_Samples = 100;
		int img_spectral_samples = N_SAMPLE;
		bool enable_openmp = true;
		bool img_loaded = false;
		GLuint my_opengl_texture;

		GLuint vao_id;
		GLuint vbo_id;
		GLuint index_buffer_id;

		std::vector<struct Vertex> vertices;
		std::vector<GLuint> indices;
		std::vector<size_t> index_nums;
		std::vector<size_t> index_partial_sums;
		std::vector<vec3> colors;


		GLuint rbo;
		GLuint fbo;
		GLuint texture;
		GLuint shaderProgram;
		const char * const vertexShaderSource = "#version 330 core\n"
			"layout (location = 0) in vec3 aPos;\n"
			//"layout (location = 1) in vec3 aNormal;\n"
			"layout (location = 1) in vec3 aColor;\n"
			//"out vec3 Normal;\n"
			"out vec3 Color;\n"
			"out vec3 FragPos;\n"
			"uniform mat3 normalmodel;\n"
			"uniform mat4 model;\n"
			"uniform mat4 view;\n"
			"uniform mat4 projection;\n"
			"void main()\n"
			"{\n"
			//"	Normal = normalize(normalmodel * aNormal);\n"
			//"	Normal = normalize(vec3(1.0, 1.0, 1.0));\n"
			"	Color = aColor;\n"
			"	FragPos = vec3(model * vec4(aPos, 1.0));\n"
			"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
			"}\0";

		const char * const fragmentShaderSource = "#version 330 core\n"
			//"in vec3 Normal;\n"
			"in vec3 Color;\n"
			"in vec3 FragPos;\n"
			"out vec4 FragColor;\n"
			"uniform vec3 lightColor;\n"
			"uniform vec3 objectColor;\n"
			"void main()\n"
			"{\n"
			//"	vec3 lightPos = vec3(0.0f, 0.0f, 1.0f);\n"
			"	float ambientStrength = 0.7;\n"
			"	vec3 ambient = ambientStrength * lightColor;\n"
			//"	float diff = max(dot(Normal, normalize(lightPos - FragPos)), 0.0);\n"
			//"	vec3 diffuse = diff * lightColor;\n"
			//"	FragColor = vec4((ambient + diffuse) * objectColor, 1.0f);"
			"	FragColor = vec4((ambient) * objectColor, 1.0f);"
			"}\n\0";

		float pitch = 0.0f, yaw = -90.0f;
		//ax::NodeEditor::EditorContext *context = nullptr;
		std::vector<std::shared_ptr<NodeMaterial>> materials;
		std::vector<std::shared_ptr<NodeMaterial> > obj_materials;

};

#endif
