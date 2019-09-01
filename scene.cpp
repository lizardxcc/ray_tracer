#include <iostream>
#include <fstream>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <thread>
#include "scene.h"
#include "vec3.h"
#include "filter.h"
#include "pdf.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <nfd.h>

#include "json.hpp"
using json = nlohmann::json;


uint8_t *env_mapping_texture = nullptr;
int env_mapping_width, env_mapping_height, env_mapping_bpp;
double env_brightness = 0.5;


void ImgViewer::LoadImage(const std::vector<double>& img, int width, int height)
{
	if (glimg != nullptr)
		delete[] glimg;
	glimg = new GLubyte[width*height*4];
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			size_t i_ = width-i-1;
			size_t j_ = height-j-1;
			const double dr = img[((height-j_-1)*width+i_)*4];
			const double dg = img[((height-j_-1)*width+i_)*4+1];
			const double db = img[((height-j_-1)*width+i_)*4+2];
			int ir = std::min(std::max(int(255.99*dr), 0), 255);
			int ig = std::min(std::max(int(255.99*dg), 0), 255);
			int ib = std::min(std::max(int(255.99*db), 0), 255);
			glimg[((height-j_-1)*width+i_)*4] = ir;
			glimg[((height-j_-1)*width+i_)*4+1] = ig;
			glimg[((height-j_-1)*width+i_)*4+2] = ib;
			glimg[((height-j_-1)*width+i_)*4+3] = 255;
		}
	}

	glGenTextures(1, &opengl_texture);
	glBindTexture(GL_TEXTURE_2D, opengl_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, glimg);
	this->width = width;
	this->height = height;
}

void ImgViewer::Render(void)
{
	ImGui::Image((void*)(intptr_t)opengl_texture, ImVec2(width, height));
}

void ImgRetouch::Render(void)
{
	ImGui::Text("Filters");
	const double sigma_d_min = 0.0;
	const double sigma_d_max = 20.0;
	const double sigma_r_min = 0.0;
	const double sigma_r_max = 1.0;
	const uint64_t win_min = 0;
	const uint64_t win_max = 16;
	ImGui::SliderScalar("sigma_d", ImGuiDataType_Double, &sigma_d, &sigma_d_min, &sigma_d_max, "%f");
	ImGui::SliderScalar("sigma_r", ImGuiDataType_Double, &sigma_r, &sigma_r_min, &sigma_r_max, "%f");
	ImGui::SliderScalar("window", ImGuiDataType_U64, &window, &win_min, &win_max, "%d");
	if (ImGui::Button("Apply filter")) {
		BiliteralFilter filter(orig_img, width, height);
		filter.sigma_d = sigma_d;
		filter.sigma_r = sigma_r;
		filter.window = window;
		filter.FilterImage();
		retouched = filter.result;
		retouched_viewer.LoadImage(retouched, width, height);
	}
	ImGui::Separator();
	ImGui::Text("Tone Mapping");
	ImGui::Separator();
	original_viewer.Render();
	ImGui::SameLine();
	retouched_viewer.Render();
}

void ImgRetouch::LoadImage(const std::vector<double>& img, int width, int height)
{
	orig_img = img;
	retouched = img;
	this->width = width;
	this->height = height;
	original_viewer.LoadImage(orig_img, width, height);
	retouched_viewer.LoadImage(retouched, width, height);
}

void RetouchWindow::Render(void)
{
	ImGui::Begin("Retouch");
	if (ImGui::BeginTabBar("Tabs")) {
		for (size_t i = 0; i < tabs.size(); i++) {
			if (ImGui::BeginTabItem(img_names[i].c_str())) {
				tabs[i].Render();
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}


void RetouchWindow::AddImage(const std::vector<double>& img, int width, int height, const char *name)
{
	std::string s = name;
	if (s == "")
		s = std::string("image.") + std::to_string(img_names.size());
	img_names.push_back(s);
	ImgRetouch new_retouch;
	new_retouch.LoadImage(img, width, height);
	tabs.push_back(new_retouch);
}

//void RetouchWindow::AddImage(double *img, int width, int height)
//{
//	std::string name = "image." + std::to_string(img_names.size());
//	AddImage(name, img, width, height);
//}

Scene::Scene(void)
{

	OpenGLInitShader();
	glGenTextures(1, &preview_texture);
	glBindTexture(GL_TEXTURE_2D, preview_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void Scene::LoadProject(const char *path)
{
	std::ifstream i(path);
	json j;
	i >> j;
	i.close();
	project_file = boost::filesystem::path(path);

	if (j.find("model") != j.end()) {
		LoadModel(project_file.parent_path().append(j["model"].get<std::string>()).c_str());
	}
	if (j.find("env") != j.end()) {
		LoadEnvTexture(project_file.parent_path().append(j["env"].get<std::string>()).c_str());
	}
	if (j.find("mat") != j.end()) {
		for (const auto& m : j["mat"]) {
			LoadMaterial(project_file.parent_path().append(m.get<std::string>()).c_str());
		}
	}
	if (j.find("objmatmap") != j.end()) {
		LoadObjMatMap(project_file.parent_path().append(j["objmatmap"].get<std::string>()).c_str());
	}
}

void Scene::LoadModel(const char *obj_path)
{
	if (obj_path == nullptr)
		return;
	renderer.Load(obj_path);
	//obj_materials.resize(renderer.obj_loader.objects.size());
	//for (size_t i = 0; i < obj_materials.size(); i++) {
	//	obj_materials[i] = nullptr;
	//}


	cameraPos = glm::dvec3(0.0f, 0.0f, 3.0f);
	//cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::dvec3(0.0f, 1.0f, 0.0f);
	OpenGLLoadModel();
}

void Scene::LoadEnvTexture(const char *path)
{
	if (path == nullptr)
		return;
	env_mapping_texture = stbi_load(path, &env_mapping_width, &env_mapping_height, &env_mapping_bpp, 0);
}


void Scene::LoadMaterial(const char *path)
{
	if (path == nullptr)
		return;
	std::ifstream i(path);
	json j;
	i >> j;

	boost::filesystem::path p(path);

	for (const auto& material_j : j) {
		std::shared_ptr<NodeMaterial> mat = std::make_shared<NodeMaterial>(material_j, p.parent_path().c_str());
		materials.push_back(mat);
	}
	i.close();
}


void Scene::WriteMaterial(const char *path)
{
	if (path == nullptr)
		return;
	std::ofstream o(path);
	json j;
	for (std::shared_ptr<NodeMaterial>& mat : materials) {
		json mat_j;
		mat->DumpJson(mat_j);
		j.push_back(mat_j);
	}
	o << j.dump(4);
	o.close();
}

void Scene::LoadObjMatMap(const char *path)
{
	if (path == nullptr)
		return;
	std::ifstream i(path);
	json j;
	i >> j;

	for (const auto& map_j : j.items()) {
		auto result = std::find_if(materials.begin(), materials.end(),
				[map_j](std::shared_ptr<NodeMaterial>& m)
				{
				return m->name == map_j.value();
				});
		if (result == materials.end()) {
			continue;
		}
		obj_materials[map_j.key()] = *result;
	}
}

void Scene::WriteObjMatMap(const char *path)
{
	if (path == nullptr)
		return;
	std::ofstream o(path);
	json j;

	for (const auto& m : obj_materials) {
		j[m.first] = m.second->name;
	}
	o << j.dump(4);
	o.close();
}



void Scene::OpenGLInitShader(void)
{
	int success;
	char infoLog[512];

	//glEnable(GL_DEPTH_TEST);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}
void Scene::OpenGLLoadModel(void)
{
	for (size_t vertices_i = 0; vertices_i < renderer.obj_loader.v.size(); vertices_i++) {
		Vertex v = {
			{
				static_cast<GLfloat>(renderer.obj_loader.v[vertices_i].x()),
				static_cast<GLfloat>(renderer.obj_loader.v[vertices_i].y()),
				static_cast<GLfloat>(renderer.obj_loader.v[vertices_i].z()),
			},
			{
				static_cast<GLfloat>(renderer.obj_loader.v[vertices_i].x()),
				static_cast<GLfloat>(renderer.obj_loader.v[vertices_i].x()),
				static_cast<GLfloat>(renderer.obj_loader.v[vertices_i].y()),
			},
		};
		vertices.push_back(v);
	}
	for (size_t object_i = 0; object_i < renderer.obj_loader.objects.size(); object_i++) {
		colors.push_back(vec3(drand48(), drand48(), drand48()));
		auto& object = renderer.obj_loader.objects[object_i];
		size_t index_num = 0;
		for (size_t faces_i = 0; faces_i < object->faces.size(); faces_i++) {
			auto& face = object->faces[faces_i];
			for (size_t v_i = 1; v_i < face.size()-1; v_i++) {
				indices.push_back(*face[0][0]);
				indices.push_back(*face[v_i][0]);
				indices.push_back(*face[v_i+1][0]);
				index_num += 3;
			}
		}

		if (index_partial_sums.empty())
			index_partial_sums.push_back(0);
		else
			index_partial_sums.push_back(index_partial_sums.back()+index_nums.back());
		index_nums.push_back(index_num);
	}
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &index_buffer_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(struct Vertex, xyz)); // positions
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(struct Vertex, rgb)); // rgb colors
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenTextures(1, &scene_texture);
	glBindTexture(GL_TEXTURE_2D, scene_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_texture, 0);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Scene::ClearData(void)
{
	renderer.Clear();
	activeObjectIndex = 0;
	img_loaded = false;
}


void Scene::RenderSceneWindow(void)
{
	//ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	//ImGui::SetNextWindowSize(ImVec2(window_width, window_height), ImGuiCond_Always);
	//ImGui::Begin("3D Scene", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Load Project")) {
				nfdchar_t *path = nullptr;
				nfdresult_t result = NFD_OpenDialog(nullptr, nullptr, &path);
				if (result == NFD_OKAY) {
					LoadProject(path);
					scene_loaded = true;
					free(path);
				}
			}
			if (ImGui::MenuItem("Open a scene file")) {
				nfdchar_t *path = nullptr;
				nfdresult_t result = NFD_OpenDialog(nullptr, nullptr, &path);
				if (result == NFD_OKAY) {
					LoadModel(path);
					scene_loaded = true;
					free(path);
				}
			}
			if (ImGui::MenuItem("Open a environment file")) {
				nfdchar_t *path = nullptr;
				nfdresult_t result = NFD_OpenDialog(nullptr, nullptr, &path);
				if (result == NFD_OKAY) {
					LoadEnvTexture(path);
					free(path);
				}
			}
			if (ImGui::MenuItem("Close a scene file")) {
				if (scene_loaded) {
					ClearData();
					scene_loaded = false;
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	if (scene_loaded)
		RenderScene();
	//ImGui::End();
}

void Scene::RenderScene(void)
{
	ImGui::BeginChild("left pane", ImVec2(WIDTH, 0));
	if (ImGui::IsWindowFocused()) {
		if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_UpArrow))) {
			pyr[0] += pyr_angular_velocity[0];
		} else if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_DownArrow))) {
			pyr[0] -= pyr_angular_velocity[0];
		}
		if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
			pyr[1] += pyr_angular_velocity[1];
		} else if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) {
			pyr[1] -= pyr_angular_velocity[1];
		}
	}
	cameraFront.x = cos(glm::radians(pyr[0])) * cos(glm::radians(pyr[1]));
	cameraFront.y = sin(glm::radians(pyr[0]));
	cameraFront.z = cos(glm::radians(pyr[0])) * sin(glm::radians(pyr[1]));
	if (ImGui::IsWindowFocused()) {
		if (ImGui::IsKeyDown('W')) {
			cameraPos += camera_speed * cameraFront;
		} else if (ImGui::IsKeyDown('S')) {
			cameraPos -= camera_speed * cameraFront;
		}
		if (ImGui::IsKeyDown('D')) {
			cameraPos += camera_speed * glm::normalize(glm::cross(cameraFront, cameraUp));
		} else if (ImGui::IsKeyDown('A')) {
			cameraPos -= camera_speed * glm::normalize(glm::cross(cameraFront, cameraUp));
		}
	}
	glViewport(0, 0, WIDTH, HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glUseProgram(shaderProgram);

	glm::mat4 model = glm::mat4(1.0);
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glm::mat4 projection;
	//projection = glm::perspective(glm::radians(60.0f), WIDTH.0f/HEIGHT.0f, 0.1f, 100.0f);
	projection = glm::perspective(glm::radians(vfov), WIDTH/HEIGHT, 0.1, 100.0);


	int modelLocation = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	int viewLocation = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
	int projectionLocation = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));


	glm::mat4 tmpModel = model;
	glm::mat3 normalModel = glm::mat3(model);
	normalModel = glm::inverse(normalModel);
	normalModel = glm::transpose(normalModel);
	glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "normalmodel"), 1, GL_FALSE, glm::value_ptr(normalModel));
	glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.9, 0.9, 0.9);

	for (size_t o_i = 0; o_i < renderer.obj_loader.objects.size(); o_i++) {
		glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.9*colors[o_i][0], 0.9*colors[o_i][1], 0.9*colors[o_i][2]);
		glStencilFunc(GL_ALWAYS, o_i+1, -1);
		if (activeObjectIndex != 0 && o_i == (activeObjectIndex-1)) {
			glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.9f, 0.9f, 0.9f);
			glDrawElements(GL_TRIANGLES, index_nums[o_i], GL_UNSIGNED_INT, (void *)(sizeof(GLuint)*index_partial_sums[o_i]));
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.0f, 0.0f);
			glDrawElements(GL_TRIANGLES, index_nums[o_i], GL_UNSIGNED_INT, (void *)(sizeof(GLuint)*index_partial_sums[o_i]));
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		} else
			glDrawElements(GL_TRIANGLES, index_nums[o_i], GL_UNSIGNED_INT, (void *)(sizeof(GLuint)*index_partial_sums[o_i]));
	}
	model = tmpModel;
	if (ImGui::IsWindowFocused() && ImGui::IsMouseClicked(0, false)) {
		auto x = ImGui::GetMousePos().x - ImGui::GetCursorScreenPos().x;
		auto y = ImGui::GetMousePos().y - ImGui::GetCursorScreenPos().y;
		if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
			unsigned int index = 0;
			glReadPixels(x, HEIGHT-y-1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);
			activeObjectIndex = index;
			if (index != 0) {
				AABB box;
				if (renderer.world->models[index-1]->BoundingBox(box)) {
					glm::dvec3 v(box.center[0], box.center[1], box.center[2]);
					double a = glm::length(v-cameraPos);
					d = 1.0/(1.0/focal_length-1.0/a);
				}
			}
		}
	}
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);

        ImGui::Image(reinterpret_cast<void *>(scene_texture), ImVec2(WIDTH, HEIGHT), ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::BeginChild("right pane", ImVec2(0, 0));
	{
		if (activeObjectIndex == 0) {
			ImGui::Text("No objects are selected");
		} else {
			ImGui::Text("Object: %s", renderer.obj_loader.objects[activeObjectIndex-1]->name.c_str());
		}
	}
	if (ImGui::CollapsingHeader("Camera Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
		{
			const double min = 0.00001;
			const double max = 180;
			ImGui::SliderScalar("vfov", ImGuiDataType_Double, &vfov, &min, &max, "%f");
		}
		{
			const double min = -10.0;
			const double max = 10.0;
			ImGui::SliderScalarN("camera pos (x/y/z)", ImGuiDataType_Double, &cameraPos.x, 3, &min, &max, "%f");
		}
		{
			const double min = 0.0;
			const double max = 1.0;
			ImGui::SliderScalar("camera speed", ImGuiDataType_Double, &camera_speed, &min, &max, "%f");
		}
		{
			const double min = -180.0;
			const double max = 180.0;
			ImGui::SliderScalarN("pitch/yaw/roll", ImGuiDataType_Double, pyr, 3, &min, &max, "%f");
		}
		{
			const double min = 0.0;
			const double max = 5.0;
			ImGui::SliderScalarN("p/y/r angular speed", ImGuiDataType_Double, pyr_angular_velocity, 3, &min, &max, "%f");
		}
	}
	if (ImGui::CollapsingHeader("Rendering Options", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (renderer.rendering_runnnig) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		const char *algorithms[] = {"Naive", "Next Event Estiation", "NEE and MIS"};
		if (ImGui::BeginCombo("Select Algorithm", algorithms[renderer.algorithm_type])) {
			for (size_t i = 0; i < 3; i++) {
				bool is_selected = (renderer.algorithm_type == i);
				if (ImGui::Selectable(algorithms[i], is_selected)) {
					renderer.algorithm_type = static_cast<RenderingAlgorithm>(i);
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
#ifdef _OPENMP
		ImGui::Checkbox("Enable OpenMP", &enable_openmp);
#endif
		ImGui::SliderInt("Image Width", &img_width, 1, 2000);
		ImGui::SliderInt("Image Height", &img_height, 1, 2000);
		ImGui::SliderInt("Image Samples", &img_Samples, 1, 1000);
		ImGui::SliderInt("Spectral samples", &img_spectral_samples, 1, N_SAMPLE);
		const double min = 0.001;
		const double max = 3.0;
		ImGui::SliderScalar("Environment Brightness", ImGuiDataType_Double, &env_brightness, &min, &max, "%f");
		if (renderer.rendering_runnnig) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::Checkbox("Enable Preview", &renderer.preview_img_flag);
	}
	ImGui::EndChild();
}






void Scene::RenderPreviewWindow(void)
{
	//ImGui::Begin("Render", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save an image")) {
				nfdchar_t *path = nullptr;
				nfdresult_t result = NFD_SaveDialog(nullptr, nullptr, &path);
				if (result == NFD_OKAY) {
					std::ofstream ofs;
					ofs.open(path, std::ios::out);
					ofs << "P3\n" << img_width << " " << img_height << std::endl << 255 << std::endl;
					for (int j = img_height-1; j >= 0; j--) {
						for (int i = 0; i < img_width; i++) {
							ofs << (int)img[((img_height-j-1)*img_width+i)*3] << " ";
							ofs << (int)img[((img_height-j-1)*img_width+i)*3+1] << " ";
							ofs << (int)img[((img_height-j-1)*img_width+i)*3+2] << std::endl;
						}
					}
					ofs.close();
					free(path);
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Render")) {
			if (ImGui::MenuItem("Render Image", nullptr, false, !renderer.rendering_runnnig)) {
				if (!renderer.rendering_runnnig) {
					img.resize(img_width*img_height*3);
					vec3 veccameraPos = vec3(cameraPos.x, cameraPos.y, cameraPos.z);
					vec3 veccameraUp = vec3(cameraUp.x, cameraUp.y, cameraUp.z);
					glm::vec3 lookat = cameraPos + cameraFront;
					vec3 vlookat = vec3(lookat.x, lookat.y, lookat.z);
					renderer.cam.set_Camera(veccameraPos, vlookat, veccameraUp, glm::radians(static_cast<double>(vfov)), static_cast<double>(img_width)/img_height);
					//renderer.cam.set_Camera(veccameraPos, vlookat, veccameraUp, static_cast<double>(img_width)/img_height, d, focal_length, aperture);
					renderer.LoadMaterials(obj_materials);
					std::thread t(&Renderer::RenderImage, &renderer, img_width, img_height, img_Samples, img_spectral_samples, enable_openmp);
					t.detach();
				}
			}
			if (renderer.rendering_runnnig) {
				if (ImGui::MenuItem("Terminate Rendering")) {
					renderer.stop_rendering = true;
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Retouch")) {
			if (ImGui::MenuItem("Add to Retouch Window")) {
				//retouch_window.AddImage(renderer.orig_img, img_width, img_height);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}


	ImGui::ProgressBar(renderer.progress);
	ImGui::BeginChild("left pane", ImVec2(img_width, 0));
	if (renderer.img_updated) {
		int nx = img_width;
		int ny = img_height;
		for (int i = 0; i < nx; i++) {
			for (int j = 0; j < ny; j++) {
				size_t i_ = nx-i-1;
				size_t j_ = ny-j-1;
				img[((ny-j_-1)*nx+i_)*3] = static_cast<GLubyte>(255.0*renderer.preview_img[i*ny+j][0]);
				img[((ny-j_-1)*nx+i_)*3+1] = static_cast<GLubyte>(255.0*renderer.preview_img[i*ny+j][1]);
				img[((ny-j_-1)*nx+i_)*3+2] = static_cast<GLubyte>(255.0*renderer.preview_img[i*ny+j][2]);
			}
		}
		glBindTexture(GL_TEXTURE_2D, preview_texture);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());
		renderer.img_updated = false;
		img_loaded = true;
	}
	ImVec2 image_cursor_pos = ImGui::GetCursorScreenPos();
	if (img_loaded)
		ImGui::Image((void*)(intptr_t)preview_texture, ImVec2(img_width, img_height));
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("right pane", ImVec2(0, 0));
	if (renderer.progress == 1.0f) {
		ImGui::Text("x:%f, y:%f", image_cursor_pos.x, image_cursor_pos.y);
		int mx = static_cast<int>(ImGui::GetMousePos().x-image_cursor_pos.x);
		mx = img_width-1-mx;
		int my = static_cast<int>(ImGui::GetMousePos().y-image_cursor_pos.y);
		if (mx >= 0 && mx < img_width && my >= 0 && my < img_height) {

			float spectrum[N_SAMPLE];
			for (int i = 0; i < N_SAMPLE; i++) {
				spectrum[i] = renderer.spectrum_img[mx*img_height+my].data[i];
			}
			ImGui::PlotHistogram("Spectrum", spectrum, N_SAMPLE, 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(500, 50));
		}
	}
	ImGui::EndChild();
	//ImGui::End();
}



void Scene::RenderMaterialNodeEditorWindow(void)
{
	//ImGui::Begin("Material Node Editor", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open Material")) {
				nfdchar_t *path = nullptr;
				nfdresult_t result = NFD_OpenDialog(nullptr, nullptr, &path);
				if (result == NFD_OKAY) {
					LoadMaterial(path);
					free(path);
				}
			}
			if (ImGui::MenuItem("Open ObjMatMap")) {
				nfdchar_t *path = nullptr;
				nfdresult_t result = NFD_OpenDialog(nullptr, nullptr, &path);
				if (result == NFD_OKAY) {
					LoadObjMatMap(path);
					free(path);
				}
			}
			if (ImGui::MenuItem("Write Material")) {
				nfdchar_t *path = nullptr;
				nfdresult_t result = NFD_SaveDialog(nullptr, nullptr, &path);
				if (result == NFD_OKAY) {
					WriteMaterial(path);
					free(path);
				}
			}
			if (ImGui::MenuItem("Write ObjMatMap")) {
				nfdchar_t *path = nullptr;
				nfdresult_t result = NFD_SaveDialog(nullptr, nullptr, &path);
				if (result == NFD_OKAY) {
					WriteObjMatMap(path);
					free(path);
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	if (activeObjectIndex == 0) {
		//ImGui::End();
		return;
	}

	char str[32] = "";
	if (ImGui::InputText("Press Enter to add new Material", &str[0], sizeof(str)/sizeof(char), ImGuiInputTextFlags_EnterReturnsTrue)) {
		std::shared_ptr<NodeMaterial> new_material = std::make_shared<NodeMaterial>(str, project_file.parent_path().c_str());
		materials.push_back(new_material);
	}

	auto itr = obj_materials.find(renderer.obj_loader.objects[activeObjectIndex-1]->name);
	if (itr == obj_materials.end()) {
		std::shared_ptr<NodeMaterial> p;
		obj_materials[renderer.obj_loader.objects[activeObjectIndex-1]->name] = p;
		itr = obj_materials.find(renderer.obj_loader.objects[activeObjectIndex-1]->name);
	}
	NodeMaterial *selected_material = itr->second.get();
	const char *preview_name = "";
	if (selected_material != nullptr)
		preview_name = selected_material->name.c_str();
	if (ImGui::BeginCombo("Select Material", preview_name)) {
		for (size_t i = 0; i < materials.size(); i++) {
			bool is_selected = (selected_material == materials[i].get());
			if (ImGui::Selectable(materials[i]->name.c_str(), is_selected)) {
				selected_material = materials[i].get();
				obj_materials[renderer.obj_loader.objects[activeObjectIndex-1]->name] = materials[i];
			}
		}
		ImGui::EndCombo();
	}


	if (selected_material != nullptr){
		ImGui::Checkbox("Light", &selected_material->light_flag);
		ax::NodeEditor::SetCurrentEditor(selected_material->context);
		ax::NodeEditor::Begin("material node editor##tmp");
		selected_material->Render();

		ax::NodeEditor::End();
		ax::NodeEditor::SetCurrentEditor(nullptr);
	}
	//ImGui::End();
}

void Scene::RenderLog(void)
{
}
