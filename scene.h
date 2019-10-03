#ifndef SCENE_H
#define SCENE_H

#include <boost/filesystem/path.hpp>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "json.hpp"
#ifndef _CLI
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#endif
#include "renderer.h"
#include "materialnode.h"

using json = nlohmann::json;

extern uint8_t *env_mapping_texture;
extern int env_mapping_width, env_mapping_height, env_mapping_bpp;
extern double env_brightness;

extern bool cli;

#ifndef _CLI
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
#endif


#ifndef _CLI
struct Vertex {
	GLfloat xyz[3];
	GLfloat normal[3];
};
#endif

class Scene {
	public:
		explicit Scene(void);

		void RenderSceneWindow(void);
		void RenderPreviewWindow(void);
		void RenderMaterialEditorWindow(void);
		void RenderMaterialNodeEditorWindow(void);
		void RenderLog(void);
		void LoadProject(const char *path);

#ifndef _CLI
		RetouchWindow retouch_window;
#endif
		json scene_json;

		Renderer renderer;
		glm::dvec3 cameraPos;
		glm::dvec3 cameraFront;
		glm::dvec3 cameraUp;
		double vfov = 30;
#ifndef _CLI
		std::vector<GLubyte> img;
#endif
		int img_spectral_samples = N_SAMPLE;
		bool enable_openmp = true;
		std::map<std::string, std::shared_ptr<NodeMaterial>> obj_materials;
	private:
		void WriteProject(const char *path);
		void LoadModel(const char *obj_path);
		void LoadEnvTexture(const char *path);
		void LoadMaterial(const char *path);
		void WriteMaterial(const char *path);
		void LoadObjMatMap(const char *path);
		void WriteObjMatMap(const char *path);
		void ClearData(void);
		void RenderScene(void);

#ifndef _CLI
		void OpenGLInitShader(void);
		void OpenGLLoadModel(void);
#endif
		bool scene_loaded = false;
		double camera_speed = 0.05;
		double pyr[3] = {0.0, -90.0, 0.0};
		double pyr_angular_velocity[3] = {0.5, 0.5, 0.5};
		const double WIDTH = 500;
		const double HEIGHT = 500;
		boost::filesystem::path project_file;
		float d = 0.45, focal_length=0.4, aperture = 0.0;
		unsigned int activeObjectIndex = 0;
		//GLubyte *img = nullptr;
		bool img_loaded = false;
#ifndef _CLI
		GLuint preview_texture;

		GLuint vao_id;
		GLuint vbo_id;

		std::vector<struct Vertex> vertices;
		std::vector<size_t> start_indices;
		std::vector<size_t> vertices_nums;
		std::vector<vec3> colors;


		GLuint rbo;
		GLuint fbo;
		GLuint scene_texture;
		GLuint shaderProgram;
		const char * const vertexShaderSource = "#version 330 core\n"
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

		const char * const fragmentShaderSource = "#version 330 core\n"
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
#endif

		std::vector<std::shared_ptr<NodeMaterial>> materials;

};

#endif
