#include <iostream>
#include <algorithm>
#include <random>
#include <cstring>

#ifndef _CLI
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#include <GLFW/glfw3.h>
#endif

//#include <OpenGL/gl3.h>
//#include <GLFW/glfw3.h>

#include "vec3.h"
#include "onb.h"
#include "ray.h"
#include "camera.h"
#include "hittablelist.h"
#include "material.h"
#include "object.h"
#include "spectrum.h"
#include "scene.h"

#include <boost/program_options.hpp>

#ifndef _CLI
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
#endif


int main(int argc, const char * argv[])
{
	boost::program_options::options_description options("CLI Options");
	options.add_options()
		//("mode", value<std::string>)
		("cli,C", "CLI mode")
		("file,F", boost::program_options::value<std::string>(), "specify project file")
		("output,o", boost::program_options::value<std::string>(), "specify output file")
		("width,w", boost::program_options::value<int>(), "specify img width")
		("height,h", boost::program_options::value<int>(), "specify img height")
		("sample,s", boost::program_options::value<int>(), "specify img samples per pixel")
		("help,H", "show help")
		;

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options), vm);
	boost::program_options::notify(vm);
	if (vm.count("cli")) {
		cli = true;
		std::cout << "CLI mode" << std::endl;
		Scene scene;
		int w, h, s;
		if (vm.count("file")) {
			scene.LoadProject(vm["file"].as<std::string>().c_str());
			std::cout << "loaded" << std::endl;
			dvec3 veccameraPos = dvec3(scene.cameraPos.x, scene.cameraPos.y, scene.cameraPos.z);
			dvec3 veccameraUp = dvec3(scene.cameraUp.x, scene.cameraUp.y, scene.cameraUp.z);
			glm::dvec3 lookat = scene.cameraPos + scene.cameraFront;
			dvec3 vlookat = dvec3(lookat.x, lookat.y, lookat.z);
			//scene.renderer.cam.set_Camera(veccameraPos, vlookat, veccameraUp, glm::radians(static_cast<double>(scene.vfov)), static_cast<double>(scene.scene_json["img_width"].get<int>())/scene.scene_json["img_height"].get<int>());
			//scene.renderer.cam.set_Camera(veccameraPos, vlookat, veccameraUp, static_cast<double>(scene.scene_json["img_width"].get<int>())/scene.scene_json["img_height"].get<int>(), 1.0, 1.0, 1.0);
			//scene.renderer.cam.set_Camera(veccameraPos, vlookat, veccameraUp, static_cast<double>(scene.scene_json["img_width"].get<int>())/scene.scene_json["img_height"].get<int>(), d, focal_length, aperture);
			scene.renderer.preview_img_flag = false;
			scene.renderer.LoadMaterials(scene.obj_materials);
			w = scene.scene_json["img_width"].get<int>();
			h = scene.scene_json["img_height"].get<int>();
			s = scene.scene_json["img_samples"].get<int>();
			if (vm.count("width"))
				w = vm["width"].as<int>();
			if (vm.count("height"))
				h = vm["height"].as<int>();
			if (vm.count("sample"))
				s = vm["sample"].as<int>();

			scene.renderer.RenderImage(w, h, s, scene.img_spectral_samples, scene.enable_openmp, true);

		}
		if (vm.count("output")) {
			std::cout << "Saving an image" << std::endl;
			std::ofstream ofs;
			ofs.open(vm["output"].as<std::string>().c_str(), std::ios::out);
			ofs << "P3\n" << w << " " << h << std::endl << 255 << std::endl;
			for (int i = 0; i < w; i++) {
				for (int j = 0; j < h; j++) {
					ofs << static_cast<int>(255.0*scene.renderer.preview_img[(h-j-1)*w+i][0]) << " ";
					ofs << static_cast<int>(255.0*scene.renderer.preview_img[(h-j-1)*w+i][1]) << " ";
					ofs << static_cast<int>(255.0*scene.renderer.preview_img[(h-j-1)*w+i][2]) << std::endl;
				}
			}
			ofs.close();
			std::cout << "Done" << std::endl;
		}
		return 0;
	}
#ifndef _CLI

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

	const GLFWvidmode *vidmode;
	vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	GLFWwindow* window = glfwCreateWindow(vidmode->width, vidmode->height, "renderer", NULL, NULL);
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
	io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/ヒラギノ角ゴシック W4.ttc", 12.0f, nullptr,
	io.Fonts->GetGlyphRangesJapanese());

	ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);

	Scene scene;


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		int window_w, window_h;
		glfwGetWindowSize(window, &window_w, &window_h);
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(window_w, window_h), ImGuiCond_Always);
		ImGui::Begin("3D Scene", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus);

		if (ImGui::BeginTabBar("TabBar")) {
			if (ImGui::BeginTabItem("3D Scene")) {
				scene.RenderSceneWindow();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Preview")) {
				scene.RenderPreviewWindow();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Matreial Editor")) {
				scene.RenderMaterialNodeEditorWindow();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();

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
#endif // _CLI

	return 0;
}
