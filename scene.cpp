#include <iostream>
#include <fstream>
#include "imgui.h"
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
#include "filebrowser.h"
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


	NodeMaterial test_material;
	//test_material.context = ax::NodeEditor::CreateEditor();
	LambertianNode *node = new LambertianNode(test_material.unique_id);
	test_material.material_nodes.push_back(node);
	test_material.name = "lambertian";
	test_material.AddLink(&node->outputs[0], &test_material.material_nodes[1]->inputs[0]);
	materials.push_back(test_material);

	NodeMaterial test_material2;
	//test_material2.context = ax::NodeEditor::CreateEditor();
	ColoredMetal *nodec = new ColoredMetal(test_material2.unique_id);
	test_material2.material_nodes.push_back(nodec);
	test_material2.name = "metal";
	materials.push_back(test_material2);
}


void Scene::LoadProject(const char *path)
{
	std::ifstream i(path);
	json j;
	i >> j;
	LoadModel(j["model"].get<std::string>().c_str());
	LoadEnvTexture(j["env"].get<std::string>().c_str());
	LoadMaterial(j["mat"].get<std::string>().c_str());
}

void Scene::LoadModel(const char *obj_path)
{
	if (obj_path == nullptr)
		return;
	renderer.Load(obj_path);
	obj_materials.resize(renderer.obj_loader.objects.size());
	for (size_t i = 0; i < obj_materials.size(); i++) {
		obj_materials[i] = nullptr;
	}


	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	//cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
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

	for (const auto& material_j : j) {
		NodeMaterial mat(material_j);
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
	for (const auto& mat : materials) {
		json mat_j;
		mat.DumpJson(mat_j);
		j.push_back(mat_j);
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
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 640, 480);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 640, 480);
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
	ImGui::Begin("3D Scene", nullptr, ImGuiWindowFlags_MenuBar);
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
			if (ImGui::MenuItem("Load test.obj")) {
				if (!scene_loaded) {
					LoadModel("test.obj");
					scene_loaded = true;
				}
			}
			if (ImGui::MenuItem("Load test2.obj")) {
				if (!scene_loaded) {
					LoadModel("test2.obj");
					scene_loaded = true;
				}
			}
			if (ImGui::MenuItem("Load cubetest.obj")) {
				if (!scene_loaded) {
					LoadModel("cubetest.obj");
					scene_loaded = true;
				}
			}
			if (ImGui::MenuItem("Load suzanne.obj")) {
				if (!scene_loaded) {
					LoadModel("suzanne.obj");
					scene_loaded = true;
				}
			}
			if (ImGui::MenuItem("Load testscene.obj")) {
				if (!scene_loaded) {
					LoadModel("testscene.obj");
					scene_loaded = true;
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	if (scene_loaded)
		RenderScene();
	ImGui::End();
}

void Scene::RenderScene(void)
{
	ImGui::SliderFloat("vfov", &vfov, 0, 180);
	//ImGui::SliderFloat("d", &d, focal_length, focal_length+5.0);
	//ImGui::SliderFloat("focal length", &focal_length, 0, 50);
	//ImGui::SliderFloat("aperture", &aperture, 0, 10);
	if (ImGui::IsWindowFocused()) {
		float cameraSpeed = 0.02f;
		if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_UpArrow))) {
			pitch += 1.0f;
		} else if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_DownArrow))) {
			pitch -= 1.0f;
		}
		if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
			yaw += 1.0f;
		} else if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) {
			yaw -= 1.0f;
		}
		cameraFront.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		cameraFront.y = sin(glm::radians(pitch));
		cameraFront.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		if (ImGui::IsKeyDown('W')) {
			cameraPos += cameraSpeed * cameraFront;
		} else if (ImGui::IsKeyDown('S')) {
			cameraPos -= cameraSpeed * cameraFront;
		}
		if (ImGui::IsKeyDown('D')) {
			cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
		} else if (ImGui::IsKeyDown('A')) {
			cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
		}
	}
	glViewport(0, 0, 640, 480);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glUseProgram(shaderProgram);

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glm::mat4 projection;
	//projection = glm::perspective(glm::radians(60.0f), 640.0f/480.0f, 0.1f, 100.0f);
	projection = glm::perspective(glm::radians(vfov), 640.0f/480.0f, 0.1f, 100.0f);


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
		if (x >= 0 && x < 640 && y >= 0 && y < 480) {
			unsigned int index = 0;
			glReadPixels(x, 480-y-1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);
			activeObjectIndex = index;
			if (index != 0) {
				AABB box;
				if (renderer.world->models[index-1]->BoundingBox(box)) {
					glm::vec3 v(box.center[0], box.center[1], box.center[2]);
					float a = glm::length(v-cameraPos);
					d = 1.0/(1.0/focal_length-1.0/a);
				}
			}
		}
	}
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);


	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::GetWindowDrawList()->AddImage(reinterpret_cast<void *>(texture), ImVec2(pos.x, pos.y),
			ImVec2(pos.x+640, pos.y+480),
			ImVec2(0, 1), ImVec2(1, 0));
}






void Scene::RenderPreviewWindow(void)
{
	ImGui::Begin("Render", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save an image")) {
				nfdchar_t *path = nullptr;
				nfdresult_t result = NFD_SaveDialog(nullptr, nullptr, &path);
				if (result == NFD_OKAY) {
					//Load(path);
					//scene_loaded = true;
					std::ofstream ofs;
					ofs.open(path, std::ios::out);
					ofs << "P3\n" << img_width << " " << img_height << std::endl << 255 << std::endl;
					for (int j = img_height-1; j >= 0; j--) {
						for (int i = 0; i < img_width; i++) {
							ofs << (int)img[((img_height-j-1)*img_width+i)*4] << " ";
							ofs << (int)img[((img_height-j-1)*img_width+i)*4+1] << " ";
							ofs << (int)img[((img_height-j-1)*img_width+i)*4+2] << std::endl;
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
					img = new GLubyte[img_width*img_height*4];
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
#ifdef _OPENMP
			if (ImGui::MenuItem("Enable OpenMP", nullptr, enable_openmp, !renderer.rendering_runnnig)) {
				enable_openmp = !enable_openmp;
			}
			//std::cout << omp_get_max_threads() << " " << omp_get_thread_limit() << std::endl;
			//omp_set_dynamic(0);
			//static int threads_num = omp_get_max_threads();
			//ImGui::SliderInt("Thread number", &threads_num, 1, 100);            // Edit 1 float using a slider from 0.0f to 1.0f
			//omp_set_num_threads(threads_num);

#endif
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Retouch")) {
			if (ImGui::MenuItem("Add to Retouch Window")) {
				retouch_window.AddImage(renderer.orig_img, img_width, img_height);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImGui::SliderInt("Image Width", &img_width, 1, 2000);
	ImGui::SliderInt("Image Height", &img_height, 1, 2000);
	ImGui::SliderInt("Image Samples", &img_Samples, 1, 1000);
	ImGui::SliderInt("Spectral samples", &img_spectral_samples, 1, N_SAMPLE);
	const double min = 0.001;
	const double max = 3.0;
	ImGui::SliderScalar("Environment Brightness", ImGuiDataType_Double, &env_brightness, &min, &max, "%f");
	if (renderer.img_updated) {
		int nx = img_width;
		int ny = img_height;
		for (int i = 0; i < nx; i++) {
			for (int j = 0; j < ny; j++) {
				size_t i_ = nx-i-1;
				size_t j_ = ny-j-1;
				const double dr = renderer.orig_img[((ny-j_-1)*nx+i_)*4];
				const double dg = renderer.orig_img[((ny-j_-1)*nx+i_)*4+1];
				const double db = renderer.orig_img[((ny-j_-1)*nx+i_)*4+2];
				int ir = std::min(std::max(int(255.99*dr), 0), 255);
				int ig = std::min(std::max(int(255.99*dg), 0), 255);
				int ib = std::min(std::max(int(255.99*db), 0), 255);
				img[((ny-j_-1)*nx+i_)*4] = ir;
				img[((ny-j_-1)*nx+i_)*4+1] = ig;
				img[((ny-j_-1)*nx+i_)*4+2] = ib;
				img[((ny-j_-1)*nx+i_)*4+3] = 255;
			}
		}
		glGenTextures(1, &my_opengl_texture);
		glBindTexture(GL_TEXTURE_2D, my_opengl_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
		renderer.img_updated = false;
		img_loaded = true;
	}
	if (img_loaded)
		ImGui::Image((void*)(intptr_t)my_opengl_texture, ImVec2(img_width, img_height));
	ImGui::End();
}


void Scene::RenderMaterialEditorWindow(void)
{
	ImGui::Begin("Material Editor");
	if (activeObjectIndex == 0) {
		ImGui::Text("No objects selected");
		ImGui::End();
		return;
	}
	static int last_objecti = -1;
	int objecti = activeObjectIndex-1;

	const char *items[renderer.Material_loader.Materials.size()];
	static int cur_item = -1;
	static int last_item = -1;
	int i = 0;
	for (const auto& m : renderer.Material_loader.Materials) {
		items[i] = m.first.c_str();
		if (objecti != last_objecti) {
			if (m.first == renderer.Material_loader.obj_mat_names[objecti]) {
				cur_item = i;
			}
		}
		i++;
	}
	ImGui::Combo("select Material", &cur_item, items, renderer.Material_loader.Materials.size());
	if (objecti == last_objecti && cur_item != last_item) {
		renderer.Material_loader.obj_mat_names[objecti] = std::string(items[cur_item]);
	}

	auto it = renderer.Material_loader.Materials.find(items[cur_item]);
	char str[32] = "";
	if (ImGui::InputText("Press Enter to add new Material", &str[0], sizeof(str)/sizeof(char), ImGuiInputTextFlags_EnterReturnsTrue)) {
		renderer.Material_loader.Materials[std::string(str)] = std::make_shared<Lambertian>(Spectrum(1));
		it = renderer.Material_loader.Materials.find(str);
		cur_item = std::distance(renderer.Material_loader.Materials.begin(), it);
		renderer.Material_loader.obj_mat_names[objecti] = std::string(str);
	}

	if (cur_item != -1) {
		const char *model_items[] = {"Lambertian", "Dielectric", "Metal", "Microfacet", "Transparent", "Light", "TextureMaterial"};
		static int cur_model_item = -1;
		static int last_model_item = -1;
		if (cur_item != last_item)
			last_model_item = -1;

		std::shared_ptr<Material> mat = it->second;
		auto& id = typeid(mat.get());
		if (id == typeid(Lambertian))
			cur_model_item = 0;
		else if (id == typeid(Dielectric))
			cur_model_item = 1;
		else if (id == typeid(Metal))
			cur_model_item = 2;
		else if (id == typeid(Microfacet))
			cur_model_item = 3;
		else if (id == typeid(Transparent))
			cur_model_item = 4;
		else if (id == typeid(DiffuseLight))
			cur_model_item = 5;
		else if (id == typeid(TextureMaterial))
			cur_model_item = 6;

		ImGui::Combo("select model", &cur_model_item, model_items, sizeof(model_items)/sizeof(const char *));
		if (cur_model_item == 0) {
			if (cur_model_item != last_model_item && last_model_item != -1) {
				mat = std::make_shared<Lambertian>(Spectrum(1));
				it->second = mat;
			}
			std::shared_ptr<Lambertian> mat_ptr = std::dynamic_pointer_cast<Lambertian>(mat);
			LambertianMaterialEditor(mat_ptr);
			vec3 col = r_rgb(mat_ptr->albedo);
			ImVec4 color = ImVec4(col[0], col[1], col[2], 1.0f);
			//colors[objecti][0] = col[0];
			//colors[objecti][1] = col[1];
			//colors[objecti][2] = col[2];
			ImGui::ColorButton("Albedo", color, ImGuiColorEditFlags_DisplayRGB);
		} else if (cur_model_item == 1) {
			if (cur_model_item != last_model_item && last_model_item != -1) {
				mat = std::make_shared<Dielectric>(Spectrum(1.33333), Spectrum(0.0));
				it->second = mat;
			}
			std::shared_ptr<Dielectric> mat_ptr = std::dynamic_pointer_cast<Dielectric>(mat);
			DielectricMaterialEditor(mat_ptr);
			//colors[objecti][0] = 1.0;
			//colors[objecti][1] = 1.0;
			//colors[objecti][2] = 1.0;
		} else if (cur_model_item == 2) {
			if (cur_model_item != last_model_item && last_model_item != -1) {
				mat = std::make_shared<Metal>(Spectrum(0.1), Spectrum(3));
				it->second = mat;
			}
			std::shared_ptr<Metal> mat_ptr = std::dynamic_pointer_cast<Metal>(mat);
			MetalMaterialEditor(mat_ptr);
			vec3 col = r_rgb(mat_ptr->albedo);
			ImVec4 color = ImVec4(col[0], col[1], col[2], 1.0f);
			//colors[objecti][0] = col[0];
			//colors[objecti][1] = col[1];
			//colors[objecti][2] = col[2];
			ImGui::ColorButton("Albedo", color, ImGuiColorEditFlags_DisplayRGB);
		} else if (cur_model_item == 3) {
			if (cur_model_item != last_model_item && last_model_item != -1) {
				mat = std::make_shared<Microfacet>(Spectrum(1.3333), Spectrum(0.0), 1.0);
				it->second = mat;
			}
			std::shared_ptr<Microfacet> mat_ptr = std::dynamic_pointer_cast<Microfacet>(mat);
			MicrofacetMaterialEditor(mat_ptr);
			vec3 col = r_rgb(mat_ptr->albedo);
			ImVec4 color = ImVec4(col[0], col[1], col[2], 1.0f);
			//colors[objecti][0] = col[0];
			//colors[objecti][1] = col[1];
			//colors[objecti][2] = col[2];
			ImGui::ColorButton("Microfacet", color, ImGuiColorEditFlags_DisplayRGB);
		} else if (cur_model_item == 4) {
			if (cur_model_item != last_model_item && last_model_item != -1) {
				mat = std::make_shared<Transparent>();
				it->second = mat;
			}
			std::shared_ptr<Transparent> mat_ptr = std::dynamic_pointer_cast<Transparent>(mat);
			TransparentMaterialEditor(mat_ptr);
			//vec3 col = r_rgb(mat_ptr->albedo);
			//ImVec4 color = ImVec4(col[0], col[1], col[2], 1.0f);
			//colors[objecti][0] = col[0];
			//colors[objecti][1] = col[1];
			//colors[objecti][2] = col[2];
			//ImGui::ColorButton("Microfacet", color, ImGuiColorEditFlags_DisplayRGB);
		} else if (cur_model_item == 5) {
			if (cur_model_item != last_model_item && last_model_item != -1) {
				mat = std::make_shared<DiffuseLight>(Spectrum(0.05));
				it->second = mat;
			}
			std::shared_ptr<DiffuseLight> mat_ptr = std::dynamic_pointer_cast<DiffuseLight>(mat);
			LightMaterialEditor(mat_ptr);
			vec3 col = unit_vector(r_rgb(mat_ptr->light_color));
			ImVec4 color = ImVec4(col[0], col[1], col[2], 1.0f);
			//colors[objecti][0] = col[0];
			//colors[objecti][1] = col[1];
			//colors[objecti][2] = col[2];
			ImGui::ColorButton("Light color", color, ImGuiColorEditFlags_DisplayRGB);
		} else if (cur_model_item == 6) {
			if (cur_model_item != last_model_item && last_model_item != -1) {
				std::shared_ptr<TextureMaterial> p = std::make_shared<TextureMaterial>();
				p->LoadImage("desk.png");
				mat = p;
				it->second = mat;
			}
			//std::shared_ptr<DiffuseLight> mat_ptr = std::dynamic_pointer_cast<DiffuseLight>(mat);
			//LightMaterialEditor(mat_ptr);
			//vec3 col = unit_vector(r_rgb(mat_ptr->light_color));
			//ImVec4 color = ImVec4(col[0], col[1], col[2], 1.0f);
			//colors[objecti][0] = col[0];
			//colors[objecti][1] = col[1];
			//colors[objecti][2] = col[2];
			//ImGui::ColorButton("Light color", color, ImGuiColorEditFlags_DisplayRGB);
		} else {
		}

		ImGui::Text("Medium");
		if (mat->mi == nullptr) {
			ImGui::Text("Not set");
			if (ImGui::Button("Add Medium")) {
				//mat->mi = static_cast<MediumMaterial>(new Homogenious());
				//mat->mi = static_cast<MediumMaterial>(new Homogenious());
				mat->mi = new HenyeyGreenstein(Spectrum(0.3), Spectrum(1.0), 0.0);
				//mat->mi = new Homogenious(Spectrum(0.5), Spectrum(1.0));
			}
		} else {
			ImGui::Text("set");
			ImGui::Text("sigma_t");
			const ImVec2 slider_size(18, 160);
			for (int i = 0; i < N_SAMPLE; i++) {
				const double min = 0.0;
				const double max = 7.0;
				if (i > 0)
					ImGui::SameLine();
				ImGui::PushID(i);
				ImGui::VSliderScalar("##sigma_t", slider_size, ImGuiDataType_Double, &mat->mi->sigma_t.data[i], &min, &max, "");
				if (ImGui::IsItemActive() || ImGui::IsItemHovered())
					ImGui::SetTooltip("%f", mat->mi->sigma_t.data[i]);
				ImGui::PopID();
			}
			ImGui::Text("albedo");
			for (int i = 0; i < N_SAMPLE; i++) {
				const double min = 0.0;
				const double max = 1.0;
				if (i > 0)
					ImGui::SameLine();
				ImGui::PushID(i);
				ImGui::VSliderScalar("##albedo", slider_size, ImGuiDataType_Double, &mat->mi->albedo.data[i], &min, &max, "");
				if (ImGui::IsItemActive() || ImGui::IsItemHovered())
					ImGui::SetTooltip("%f", mat->mi->albedo.data[i]);
				ImGui::PopID();
			}
			const double min = -1.0;
			const double max = 1.0;
			HenyeyGreenstein *mi_ptr = dynamic_cast<HenyeyGreenstein *>(mat->mi);
			ImGui::SliderScalar("g ([-1:backward, 1:forward] scattering)", ImGuiDataType_Double, &mi_ptr->g, &min, &max, "%f");

		}

		last_model_item = cur_model_item;
		char str[32] = "";
		if (ImGui::InputText("Save Material", &str[0], sizeof(str)/sizeof(char), ImGuiInputTextFlags_EnterReturnsTrue)) {
			renderer.Material_loader.Write(str);
		}
		//if (ImGui::Button("Save Material")) {
		//	//renderer.Material_loader.Write("test.Material");
		//}
	}


	last_objecti = objecti;
	last_item = cur_item;

	ImGui::End();
}


void Scene::RenderMaterialNodeEditorWindow(void)
{
	ImGui::Begin("Material Node Editor", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Write Material")) {
				nfdchar_t *path = nullptr;
				nfdresult_t result = NFD_SaveDialog(nullptr, nullptr, &path);
				if (result == NFD_OKAY) {
					WriteMaterial(path);
					free(path);
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	if (activeObjectIndex == 0) {
		ImGui::End();
		return;
	}

	NodeMaterial *selected_material = obj_materials[activeObjectIndex-1];
	const char *preview_name = "";
	if (selected_material != nullptr)
		preview_name = selected_material->name.c_str();
	if (ImGui::BeginCombo("Select Material", preview_name)) {
		for (size_t i = 0; i < materials.size(); i++) {
			bool is_selected = (selected_material == &materials[i]);
			if (ImGui::Selectable(materials[i].name.c_str(), is_selected)) {
				selected_material = &materials[i];
				obj_materials[activeObjectIndex-1] = &materials[i];
			}
		}
		ImGui::EndCombo();
	}

	if (selected_material != nullptr){
		ax::NodeEditor::SetCurrentEditor(selected_material->context);
		ax::NodeEditor::Begin("material node editor##tmp");
		selected_material->Render();

		ax::NodeEditor::End();
	}
	ImGui::End();
}

void Scene::RenderLog(void)
{
}
