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

#include <thread>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "vec3.h"
#include "onb.h"
#include "ray.h"
#include "camera.h"
#include "hitablelist.h"
#include "material.h"
#include "object.h"
#include "spectrum.h"
#include "scene.h"


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


lens_camera cam;

//void set_camera(double lx, double ly, double lz, double rax, double ray, double raz, double rot_theta, double vfov, double aspect)
//{
//	cam = new camera(vec3(lx, ly, lz), vec3(rax, ray, raz), rot_theta, vfov, aspect);
//}

double get_radiance(ray& r, const hitable *world, int count)
{
	hit_record rec;
	if (world->hit(r, 0.001, std::numeric_limits<double>::max(), rec)) {
		double bxdf, pdf;
		double radiance = rec.mat_ptr->emitted(r, rec);
		onb uvw;
		uvw.build_from_w(rec.normal);
		vec3 generated_vi;
		double wli;
		bool respawn = rec.mat_ptr->sample(rec, uvw, uvw.worldtolocal(-r.direction()), r.central_wl, generated_vi, wli, bxdf, pdf);
		double prr;
		if (respawn) {
			prr = std::min(1.0, 0.1+bxdf);
		} else {
			prr = 0.0;
		}

		if (drand48() < prr) {
			if (respawn) {
				ray scattered(rec.p, uvw.localtoworld(generated_vi));
				scattered.central_wl = wli;
				scattered.min_wl = r.min_wl;
				scattered.max_wl = r.max_wl;
				radiance += bxdf * get_radiance(scattered, world, count+1) *
					abs(generated_vi.z()) / pdf / prr;
			}
		}
		return radiance;
	} else {
		return 0.0;
	}
}



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
	lambertian mat(Spectrum(0));
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
	mat.lights.push_back(light);

	return new hitable_list(list);
}


hitable *obj_room(void)
{
	std::vector<hitable *> list;
	list.push_back(new objmodel("test.obj"));
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


GLubyte *img = nullptr;
int img_width = 500;
int img_height = 500;
bool img_loaded = false;
bool img_updated = false;
GLuint my_opengl_texture;

void execute(int nx, int ny, int ns, const char *filename)
{
	Spectrum **spectrum_array = new Spectrum*[nx];
	//vec3 **rgb_array = new vec3*[nx];
	for (int i = 0; i < nx; i++) {
		spectrum_array[i] = new Spectrum[ny];
		//rgb_array[i] = new vec3[ny];
	}


	std::ofstream ofs;
	ofs.open(filename);

	ofs << "P3\n" << nx << " " << ny << "\n255\n";
	//ofs << "P3\n" << nx << " " << ny << "\n65535\n";

	hitable *world = obj_room();
	//camera cam(vec3(-1.0, 2.0, 6.4), vec3(0.0, 4.2, 0.0), vec3(0, 1, 0), 90.0, 1.0);
	//camera cam(vec3(0.0, 3.0, 3.0), vec3(0.0, 1.0, 0.0), vec3(0, 1, 0), 60.0, 1.0);
	//pinhole_camera cam(vec3(0.0, 3.0, 12.0), vec3(0.0, 0.0, -10.0), vec3(0, 1, 0), 1.0, 1.0);
	//lens_camera cam(vec3(0.0, 3.0, 12.0), vec3(0.0, 0.0, -10.0), vec3(0, 1, 0), 1.0, 0.85, 0.8, 1.0);
	//camera cam(vec3(-2.0, 3.0, -3.0), vec3(0.0, 0.0, 0.0), vec3(0, 1, 0), 60.0, 1.0);
	//camera cam(vec3(0.0, 10.0, 10.0), vec3(0.0, 0.0, 0.0), vec3(0, 1, 0), 60.0, (double)nx/(double)ny);

	size_t count = 0;

int i, j, s;
#ifdef _OPENMP
#pragma omp parallel for private(j, s) schedule(static)
#endif
	for (i = 0; i < nx; i++) {
		for (j = 0; j < ny; j++) {
			//vec3 col(0, 0, 0);
			Spectrum radiance(0);
			for (s = 0; s < ns; s++) {
				double u = double(i + drand48()) / double(nx);
				double v = double(j + drand48()) / double(ny);
				ray r = cam.get_ray(u, v);

				double rand = drand48();
				const size_t num = 10;
				for (size_t k = 0; k < num; k++) {
					if (rand <= ((double)(k+1)/ (double)num)) {
						double min_wl = 400 + 300.0/(double)num*(double)k;
						double max_wl = min_wl + 300.0/(double)num - 0.00001;
						//double max_wl = min_wl + SAMPLE_SIZE - 0.00001;
						//double max_wl = 400 + 300.0/(double)num*(double)(k+1) - 0.00001;
						r.min_wl = min_wl;
						r.max_wl = max_wl;
						r.central_wl = (min_wl + max_wl) / 2.0;
						double rad = get_radiance(r, world, 0);
						//if (rad > std::numeric_limits<double>::max()) {
						//	std::cout << "ALARM" << std::endl;
						//}
						if (!std::isnan(rad)) {
							radiance.add((double)(rad/(double)ns), min_wl, max_wl);
						}
						break;
					}
				}
			}
			vec3 rgb_col = rgb(radiance);
			for (size_t i = 0; i < 3; i++) {
				if (rgb_col[i] >= 0.0) {
					rgb_col.e[i] = pow(rgb_col[i], 1.0/2.2);
				}
			}
			int ir = std::min(std::max(int(255.99*rgb_col[0]), 0), 255);
			int ig = std::min(std::max(int(255.99*rgb_col[1]), 0), 255);
			int ib = std::min(std::max(int(255.99*rgb_col[2]), 0), 255);
			size_t i_ = nx-i-1;
			size_t j_ = ny-j-1;
			img[((ny-j_-1)*nx+i_)*4] = ir;
			img[((ny-j_-1)*nx+i_)*4+1] = ig;
			img[((ny-j_-1)*nx+i_)*4+2] = ib;
			img[((ny-j_-1)*nx+i_)*4+3] = 255;

			count++;
			spectrum_array[i][j] = radiance;
			if (count % 5000 == 0) {
#ifdef _OPENMP
				std::cout << "thread: " << omp_get_thread_num() << "  ";
#endif
				std::cout << 100.0 * (double)count / (double)(nx*ny) << "%" << std::endl;
				img_updated = true;
			}

		}
	}

	//for (int j = ny-1; j >= 0; j--) {
	//	for (int i = 0; i < nx; i++) {
	for (int j = 0; j < ny; j++) {
		for (int i = nx-1; i >= 0; i--) {

			size_t i_ = nx-i-1;
			size_t j_ = ny-j-1;
			int ir = img[((ny-j_-1)*nx+i_)*4];
			int ig = img[((ny-j_-1)*nx+i_)*4+1];
			int ib = img[((ny-j_-1)*nx+i_)*4+2];

			ofs << ir << " " << ig << " " << ib << "\n";
		}
	}
	img_updated = true;
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
	bool window_active = true;

        int my_image_width = 200, my_image_height = 200;
        //unsigned char* my_image_data = stbi_load("my_image.png", &my_image_width, &my_image_height, NULL, 4);
	unsigned char *my_image_data = new unsigned char[my_image_width*my_image_height*4];
	for (int i = 0; i < my_image_width*my_image_height*4; i++) {
		my_image_data[i] = 255;
	}

	Scene scene;

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

		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world! こんにちは世界");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		if (window_active) {
			static int slider_img_width = 500;
			static int slider_img_height = 500;
			static int slider_img_samples = 100;
			ImGui::Begin("Render", &window_active, ImGuiWindowFlags_MenuBar);
			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Render")) {
					if (ImGui::MenuItem("Render Image")) {
					img_width = slider_img_width;
					img_height = slider_img_height;
					std::cout << img_width << " " << img_height << std::endl;
					img = new GLubyte[img_width*img_height*4];
					for (int i = 0; i < img_width*img_height*4; i++)
						img[i] = 255;
					//std::memset(img, 255, img_width*img_height*4);
					img_updated = true;
					//set_camera(0, 0, 3, 1, 0, 0, M_PI/2.0, 60, (double)img_width/img_height);
					vec3 cameraPos = vec3(scene.cameraPos.x, scene.cameraPos.y, scene.cameraPos.z);
					vec3 cameraUp = vec3(scene.cameraUp.x, scene.cameraUp.y, scene.cameraUp.z);
					glm::vec3 lookat = scene.cameraPos + scene.cameraFront;
					vec3 vlookat = vec3(lookat.x, lookat.y, lookat.z);
					//cam.set_camera(cameraPos, vlookat, cameraUp, 60, (double)img_width/img_height);
					cam.set_camera(cameraPos, vlookat, cameraUp, (double)img_width/img_height, scene.d, scene.focal_length, scene.aperture);
					std::thread t(execute, img_width, img_height, slider_img_samples, "test.pnm");
					t.detach();
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			ImGui::SliderInt("Image Width", &slider_img_width, 0, 2000);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::SliderInt("Image Height", &slider_img_height, 0, 2000);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::SliderInt("Image Samples", &slider_img_samples, 0, 1000);            // Edit 1 float using a slider from 0.0f to 1.0f
			if (img_updated) {
				glGenTextures(1, &my_opengl_texture);
				glBindTexture(GL_TEXTURE_2D, my_opengl_texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
				img_updated = false;
				img_loaded = true;
			}
			if (img_loaded)
				ImGui::Image((void*)(intptr_t)my_opengl_texture, ImVec2(img_width, img_height));
			ImGui::End();
		}
		{
			scene.Render();
		}

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
