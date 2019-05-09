#include <iostream>
#include <algorithm>
#include <random>
#include <cstring>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#include <GLFW/glfw3.h>

//#include <OpenGL/gl3.h>
//#include <GLFW/glfw3.h>

#include "vec3.h"
#include "onb.h"
#include "ray.h"
#include "camera.h"
#include "hitablelist.h"
#include "material.h"
#include "materialfile.h"
#include "object.h"
#include "spectrum.h"
#include "scene.h"


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


//void set_camera(double lx, double ly, double lz, double rax, double ray, double raz, double rot_theta, double vfov, double aspect)
//{
//	cam = new camera(vec3(lx, ly, lz), vec3(rax, ray, raz), rot_theta, vfov, aspect);
//}


hitable *room(void)
{
	std::vector<hitable *> list;
	Spectrum albedo(1.0);
	//albedo.data[5] = 1.0;
	quadrilateral *quad = new quadrilateral();
	quad->v[0] = vec3(-10.0, -1, 10.0);
	quad->v[1] = vec3(10.0, -1, 10.0);
	quad->v[2] = vec3(10.0, -1, -10.0);
	quad->v[3] = vec3(-10.0, -1, -10.0);
	quad->normal = vec3(0.0, 1.0, 0.0);
	quad->mat_ptr = new lambertian(albedo);
	list.push_back(quad);

	//list.push_back(new sphere(vec3(-0.5, -0.0, -0.5), 0.3, new dielectric(Spectrum(1), 1.72, 0.41342)));

	double size = 1.0;
	hitable *light;
	Spectrum light_s;
	light_s.data[0] = 0.030;
	light_s.data[1] = 0.030;
	light_s.data[2] = 0.030;
	light_s.data[3] = 0.030;
	light_s.data[4] = 0.030;
	light_s.data[5] = 0.030;
	light_s.data[6] = 0.030;
	light_s.data[7] = 0.030;
	light_s.data[8] = 0.030;
	light_s.data[9] = 0.030;
	light = new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.5, 0.5, new diffuse_light(light_s));
	list.push_back(light);
	material::lights.push_back(light);

	return new hitable_list(list);
}


hitable *obj_room(void)
{
	std::vector<hitable *> list;
	//list.push_back(new objmodel("test.obj"));
	//list.push_back(new sphere(vec3(-0.5, -0.0, -0.5), 0.3, new metal(RGBtoSpectrum(vec3(0.8, 0.5, 0.1)))));
	//double size = 5.0;
	//hitable *light;
	//lambertian mat(Spectrum(0));
	//Spectrum light_s(0.01);
	//light = new rectangle(vec3(0, size-0.01, 0), vec3(0, -1, 0), vec3(-1, 0, 0), 0.5, 0.5, new diffuse_light(light_s));
	////list.push_back(light);
	////mat.lights.push_back(light);


	return new hitable_list(list);
}



int main(void)
{
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	// Decide GL+GLSL versions
#if __APPLE__
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

	GLFWwindow* window = glfwCreateWindow(1000, 600, "renderer", NULL, NULL);
	if (window == NULL)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.IniFilename = nullptr;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/ヒラギノ角ゴシック W2.ttc", 14.0f, nullptr,
	io.Fonts->GetGlyphRangesJapanese());

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);

        int my_image_width = 200, my_image_height = 200;
        //unsigned char* my_image_data = stbi_load("my_image.png", &my_image_width, &my_image_height, NULL, 4);
	unsigned char *my_image_data = new unsigned char[my_image_width*my_image_height*4];
	for (int i = 0; i < my_image_width*my_image_height*4; i++) {
		my_image_data[i] = 255;
	}

	Scene scene;
	MaterialEditor material_editor;


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Render"))
			{
				if (ImGui::MenuItem("Render")) {
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		scene.RenderSceneWindow();
		scene.RenderResultWindow();
		scene.RenderMaterialEditorWindow();

		ImGui::Render();
		int display_w, display_h;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
