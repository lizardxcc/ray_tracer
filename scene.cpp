#include <iostream>
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


void ImgViewer::LoadImage(const std::shared_ptr<const double[]>& img, int width, int height)
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
		BiliteralFilter filter(orig_img.get(), width, height);
		filter.sigma_d = sigma_d;
		filter.sigma_r = sigma_r;
		filter.window = window;
		filter.FilterImage();
		double *n = filter.result;
		retouched.reset(n);
		retouched_viewer.LoadImage(retouched, width, height);
	}
	ImGui::Separator();
	ImGui::Text("Tone Mapping");
	ImGui::Separator();
	original_viewer.Render();
	ImGui::SameLine();
	retouched_viewer.Render();
}

void ImgRetouch::LoadImage(std::shared_ptr<double[]>& img, int width, int height)
{
	orig_img = img;
	retouched.reset(new double[width*height*4]);
	for (size_t i = 0; i < width*height*4; i++) {
		retouched[i] = orig_img[i];
	}
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


void RetouchWindow::AddImage(std::string& name, std::shared_ptr<double[]>& img, int width, int height)
{
	img_names.push_back(name);
	ImgRetouch new_retouch;
	new_retouch.LoadImage(img, width, height);
	tabs.push_back(new_retouch);
}

void RetouchWindow::AddImage(std::shared_ptr<double[]>& img, int width, int height)
{
	std::string name = "image." + std::to_string(img_names.size());
	AddImage(name, img, width, height);
}

Scene::Scene(void)
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

	//unsigned int indices[] = {
	//	0, 1, 3,
	//	1, 2, 3
	//};

}


void Scene::Load(const char *objfilename, const char *matfilename)
{
	renderer.Load(objfilename, matfilename);
	for (int o = 0; o < renderer.obj_loader.objects.size(); o++) {
		VAOs.push_back(0);
		glGenVertexArrays(1, &VAOs[o]);
		GLuint VBO;
		glGenBuffers(1, &VBO);
		//glGenBuffers(1, &EBO);

		glBindVertexArray(VAOs[o]);

		int triangle_num = renderer.obj_loader.objects[o]->f.size();
		vertices_num.push_back(triangle_num*3);
		vertices_array.push_back(new float[vertices_num[o]*6]);
		for (int i = 0; i < triangle_num; i++) {
			auto& face = renderer.obj_loader.objects[o]->f[i];
			if (face.size() != 3) {
				std::cout << "Error!! unsupported vertex size" << std::endl;
			}
			vec3 normal, v[3];
			for (int j = 0; j < face.size(); j++) {
				v[j] = renderer.obj_loader.objects[o]->v[*face[j][0]];
			}
			normal = unit_vector(cross(v[1] - v[0], v[2] - v[1]));
			for (int j = 0; j < face.size(); j++) {

				//vertex
				vertices_array[o][i*18+j*6] = v[j].x();
				vertices_array[o][i*18+j*6+1] = v[j].y();
				vertices_array[o][i*18+j*6+2] = v[j].z();;

				//normal
				vertices_array[o][i*18+j*6+3] = normal.x();
				vertices_array[o][i*18+j*6+4] = normal.y();
				vertices_array[o][i*18+j*6+5] = normal.z();;
				//std::cout << x << " " << y << " " << z << std::endl;
			}
		}
		std::shared_ptr<Material> mat = renderer.Material_loader.Materials[renderer.Material_loader.obj_mat_names[o]];
		if (mat == nullptr)
			std::cout << "WARNING" << std::endl;
		if (typeid(*mat) == typeid(Lambertian)) {
			vec3 col = r_rgb(std::dynamic_pointer_cast<Lambertian>(mat)->albedo);
			colors.push_back(std::array<float, 3>({(float)col[0], (float)col[1], (float)col[2]}));
		} else if (typeid(*mat) == typeid(Dielectric)) {
			colors.push_back(std::array<float, 3>({1.0f, 1.0f, 1.0f}));
		} else if (typeid(*mat) == typeid(Metal)) {
			vec3 col = r_rgb(std::dynamic_pointer_cast<Metal>(mat)->albedo);
			colors.push_back(std::array<float, 3>({(float)col[0], (float)col[1], (float)col[2]}));
		} else if (typeid(*mat) == typeid(TorranceSparrow)) {
			vec3 col = r_rgb(std::dynamic_pointer_cast<TorranceSparrow>(mat)->albedo);
			colors.push_back(std::array<float, 3>({(float)col[0], (float)col[1], (float)col[2]}));
		} else if (typeid(*mat) == typeid(DiffuseLight)) {
			vec3 col = unit_vector(r_rgb(std::dynamic_pointer_cast<DiffuseLight>(mat)->light_color));
			colors.push_back(std::array<float, 3>({(float)col[0], (float)col[1], (float)col[2]}));
		} else {
			colors.push_back(std::array<float, 3>({(float)drand48(), (float)drand48(), (float)drand48()}));
		}

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices_num[o]*6, vertices_array[o], GL_STATIC_DRAW);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float) ,(void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float) ,(void*)(3*sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// do not unbind the EBO while a VAO is active.
		glBindVertexArray(0);
	}

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

	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	//cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Scene::ClearData(void)
{
	VAOs.clear();
	vertices_array.clear();
	vertices_num.clear();
	colors.clear();
	renderer.Clear();
	activeObjectIndex = 0;
	img_loaded = false;
}


void Scene::RenderSceneWindow(void)
{
	ImGui::Begin("3D Scene", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Load test.obj")) {
				if (!scene_loaded) {
					Load("test.obj", "test.Material");
					scene_loaded = true;
				}
			}
			if (ImGui::MenuItem("Load test2.obj")) {
				if (!scene_loaded) {
					Load("test2.obj", "test2.Material");
					scene_loaded = true;
				}
			}
			if (ImGui::MenuItem("Close")) {
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
	ImGui::End();
}

void Scene::RenderScene(void)
{
	ImGui::SliderFloat("d", &d, focal_length, focal_length+5.0);
	ImGui::SliderFloat("focal length", &focal_length, 0, 50);
	ImGui::SliderFloat("aperture", &aperture, 0, 10);
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
	projection = glm::perspective(glm::radians(60.0f), 640.0f/480.0f, 0.1f, 100.0f);


	int modelLocation = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	int viewLocation = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
	int projectionLocation = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));


	for (int o = 0; o < VAOs.size(); o++) {
		glStencilFunc(GL_ALWAYS, o+1, -1);
		glBindVertexArray(VAOs[o]);
		glm::mat4 tmpModel = model;
		//model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		glm::mat3 normalModel = glm::mat3(model);
		normalModel = glm::inverse(normalModel);
		normalModel = glm::transpose(normalModel);
		glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "normalmodel"), 1, GL_FALSE, glm::value_ptr(normalModel));

		glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.9*colors[o][0], 0.9*colors[o][1], 0.9*colors[o][2]);
		if (activeObjectIndex != 0 && o == (activeObjectIndex-1)) {
			//glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.8f, 0.8f, 0.8f);
			glDrawArrays(GL_TRIANGLES, 0, vertices_num[o]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.0f, 0.0f);
			glDrawArrays(GL_TRIANGLES, 0, vertices_num[o]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		} else {
			glDrawArrays(GL_TRIANGLES, 0, vertices_num[o]);
		}
		glBindVertexArray(0);
		model = tmpModel;
	}
	if (ImGui::IsWindowFocused() && ImGui::IsMouseClicked(0, false)) {
		auto x = ImGui::GetMousePos().x - ImGui::GetCursorScreenPos().x;
		auto y = ImGui::GetMousePos().y - ImGui::GetCursorScreenPos().y;
		if (x >= 0 && x < 640 && y >= 0 && y < 480) {
			unsigned int index = 0;
			glReadPixels(x, 480-y-1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);
			activeObjectIndex = index;
			if (index != 0) {
				aabb box;
				if (renderer.world->models[index-1]->bounding_box(box)) {
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
	ImGui::GetWindowDrawList()->AddImage((void*)texture, ImVec2(pos.x, pos.y),
			ImVec2(pos.x+640, pos.y+480),
			ImVec2(0, 1), ImVec2(1, 0));
}






void Scene::RenderPreviewWindow(void)
{
	ImGui::Begin("Render", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Render")) {
			if (ImGui::MenuItem("Render Image")) {
				std::cout << img_width << " " << img_height << std::endl;
				img = new GLubyte[img_width*img_height*4];
				vec3 veccameraPos = vec3(cameraPos.x, cameraPos.y, cameraPos.z);
				vec3 veccameraUp = vec3(cameraUp.x, cameraUp.y, cameraUp.z);
				glm::vec3 lookat = cameraPos + cameraFront;
				vec3 vlookat = vec3(lookat.x, lookat.y, lookat.z);
				//cam.set_camera(cameraPos, vlookat, cameraUp, 60, (double)img_width/img_height);
				renderer.cam.set_camera(veccameraPos, vlookat, veccameraUp, static_cast<double>(img_width)/img_height, d, focal_length, aperture);
				std::thread t(&Renderer::RenderImage, &renderer, img_width, img_height, img_Samples);
				t.detach();
			}
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
	ImGui::SliderInt("Image Width", &img_width, 0, 2000);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::SliderInt("Image Height", &img_height, 0, 2000);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::SliderInt("Image Samples", &img_Samples, 0, 1000);            // Edit 1 float using a slider from 0.0f to 1.0f
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
		const char *model_items[] = {"Lambertian", "Dielectric", "Metal", "Microfacet", "Transparent", "Light"};
		static int cur_model_item = -1;
		static int last_model_item = -1;
		if (cur_item != last_item)
			last_model_item = -1;

		std::shared_ptr<Material> mat = it->second;
		auto& id = typeid(*mat);
		if (id == typeid(Lambertian))
			cur_model_item = 0;
		else if (id == typeid(Dielectric))
			cur_model_item = 1;
		else if (id == typeid(Metal))
			cur_model_item = 2;
		else if (id == typeid(TorranceSparrow))
			cur_model_item = 3;
		else if (id == typeid(Transparent))
			cur_model_item = 4;
		else if (id == typeid(DiffuseLight))
			cur_model_item = 5;

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
			colors[objecti][0] = col[0];
			colors[objecti][1] = col[1];
			colors[objecti][2] = col[2];
			ImGui::ColorButton("Albedo", color, ImGuiColorEditFlags_DisplayRGB);
		} else if (cur_model_item == 1) {
			if (cur_model_item != last_model_item && last_model_item != -1) {
				mat = std::make_shared<Dielectric>(Spectrum(1.33333));
				it->second = mat;
			}
			std::shared_ptr<Dielectric> mat_ptr = std::dynamic_pointer_cast<Dielectric>(mat);
			DielectricMaterialEditor(mat_ptr);
			colors[objecti][0] = 1.0;
			colors[objecti][1] = 1.0;
			colors[objecti][2] = 1.0;
		} else if (cur_model_item == 2) {
			if (cur_model_item != last_model_item && last_model_item != -1) {
				mat = std::make_shared<Metal>(Spectrum(1));
				it->second = mat;
			}
			std::shared_ptr<Metal> mat_ptr = std::dynamic_pointer_cast<Metal>(mat);
			MetalMaterialEditor(mat_ptr);
			vec3 col = r_rgb(mat_ptr->albedo);
			ImVec4 color = ImVec4(col[0], col[1], col[2], 1.0f);
			colors[objecti][0] = col[0];
			colors[objecti][1] = col[1];
			colors[objecti][2] = col[2];
			ImGui::ColorButton("Albedo", color, ImGuiColorEditFlags_DisplayRGB);
		} else if (cur_model_item == 3) {
			if (cur_model_item != last_model_item && last_model_item != -1) {
				mat = std::make_shared<TorranceSparrow>(Spectrum(1), 1.0);
				it->second = mat;
			}
			std::shared_ptr<TorranceSparrow> mat_ptr = std::dynamic_pointer_cast<TorranceSparrow>(mat);
			MicrofacetMaterialEditor(mat_ptr);
			vec3 col = r_rgb(mat_ptr->albedo);
			ImVec4 color = ImVec4(col[0], col[1], col[2], 1.0f);
			colors[objecti][0] = col[0];
			colors[objecti][1] = col[1];
			colors[objecti][2] = col[2];
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
			colors[objecti][0] = col[0];
			colors[objecti][1] = col[1];
			colors[objecti][2] = col[2];
			ImGui::ColorButton("Light color", color, ImGuiColorEditFlags_DisplayRGB);
		} else {
		}

		ImGui::Text("Medium");
		if (mat->mi == nullptr) {
			ImGui::Text("Not set");
			if (ImGui::Button("Add Medium")) {
				//mat->mi = static_cast<MediumMaterial>(new Homogenious());
				//mat->mi = static_cast<MediumMaterial>(new Homogenious());
				mat->mi = new henyey_greenstein(Spectrum(0.3), Spectrum(1.0), 0.0);
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
			henyey_greenstein *mi_ptr = dynamic_cast<henyey_greenstein *>(mat->mi);
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
