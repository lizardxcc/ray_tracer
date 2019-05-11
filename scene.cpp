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

#ifdef _OPENMP
#include <omp.h>
#endif


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
	Load("test.obj");
	for (int o = 0; o < obj_loader.objects.size(); o++) {
		VAOs.push_back(0);
		glGenVertexArrays(1, &VAOs[o]);
		GLuint VBO;
		glGenBuffers(1, &VBO);
		//glGenBuffers(1, &EBO);

		glBindVertexArray(VAOs[o]);

		int triangle_num = obj_loader.objects[o]->f.size();
		vertices_num.push_back(triangle_num*3);
		vertices_array.push_back(new float[vertices_num[o]*6]);
		for (int i = 0; i < triangle_num; i++) {
			auto& face = obj_loader.objects[o]->f[i];
			if (face.size() != 3) {
				std::cout << "Error!! unsupported vertex size" << std::endl;
			}
			vec3 normal, v[3];
			for (int j = 0; j < face.size(); j++) {
				v[j] = obj_loader.objects[o]->v[*face[j][0]];
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
		std::shared_ptr<material> mat = material_loader.materials[obj_loader.objects[o]->material_name];
		if (typeid(*mat) == typeid(lambertian)) {
			vec3 col = r_rgb(std::dynamic_pointer_cast<lambertian>(mat)->albedo);
			colors.push_back(std::array<float, 3>({(float)col[0], (float)col[1], (float)col[2]}));
		} else if (typeid(*mat) == typeid(diffuse_light)) {
			vec3 col = unit_vector(r_rgb(std::dynamic_pointer_cast<diffuse_light>(mat)->light_color));
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


void Scene::RenderSceneWindow(void)
{
	ImGui::Begin("3D Scene");
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

	float timeValue = glfwGetTime();
	float greenValue = std::sin(timeValue) / 2.0f + 0.5f;

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
				if (world->models[index-1]->bounding_box(box)) {
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
	ImGui::End();
}



double Scene::GetRadiance(ray& r, int count)
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
			if (count > 10) {
				prr = 0.01;
			} else {
				prr = std::min(0.8, 0.1+bxdf);
			}
		} else {
			prr = 0.0;
		}

		if (drand48() < prr) {
			if (respawn) {
				ray scattered(rec.p, uvw.localtoworld(generated_vi));
				scattered.central_wl = wli;
				scattered.min_wl = r.min_wl;
				scattered.max_wl = r.max_wl;
				radiance += bxdf * GetRadiance(scattered, count+1) *
					abs(generated_vi.z()) / pdf / prr;
			}
		}
		return radiance;
	} else {
		return 0.0;
	}
}


void Scene::RenderImage(int nx, int ny, int ns, const char *filename)
{
	material::lights.clear();
	for (int i = 0; i < world->models.size(); i++) {
		//world->models[i]->set_material(std::shared_ptr<material>(materials[i]));
		auto mat = material_loader.materials[obj_loader.objects[i]->material_name];
		world->models[i]->set_material(mat);
		if (mat->light_flag) {
			material::lights.push_back(world->models[i]);
		}
	}


	std::ofstream ofs;
	ofs.open(filename);

	ofs << "P3\n" << nx << " " << ny << "\n255\n";
	//ofs << "P3\n" << nx << " " << ny << "\n65535\n";

	//camera cam(vec3(-1.0, 2.0, 6.4), vec3(0.0, 4.2, 0.0), vec3(0, 1, 0), 90.0, 1.0);
	//camera cam(vec3(0.0, 3.0, 3.0), vec3(0.0, 1.0, 0.0), vec3(0, 1, 0), 60.0, 1.0);
	//pinhole_camera cam(vec3(0.0, 3.0, 12.0), vec3(0.0, 0.0, -10.0), vec3(0, 1, 0), 1.0, 1.0);
	//lens_camera cam(vec3(0.0, 3.0, 12.0), vec3(0.0, 0.0, -10.0), vec3(0, 1, 0), 1.0, 0.85, 0.8, 1.0);
	//camera cam(vec3(-2.0, 3.0, -3.0), vec3(0.0, 0.0, 0.0), vec3(0, 1, 0), 60.0, 1.0);
	//camera cam(vec3(0.0, 10.0, 10.0), vec3(0.0, 0.0, 0.0), vec3(0, 1, 0), 60.0, (double)nx/(double)ny);

	size_t count = 0;

int i, j, s;
#ifdef _OPENMP
#pragma omp parallel for private(j, s) schedule(dynamic)
#endif
	for (i = 0; i < nx; i++) {
		for (j = 0; j < ny; j++) {
			//vec3 col(0, 0, 0);
			Spectrum radiance(0.0);
			for (s = 0; s < ns; s++) {
				double u = (i + drand48()) / nx;
				double v = (j + drand48()) / ny;
				ray r = cam.get_ray(u, v);

				double rand = drand48();
				const size_t num = N_SAMPLE; // 調整
				for (size_t k = 0; k < num; k++) {
					if (rand <= (static_cast<double>(k+1)/ num)) {
						double min_wl = 400.0 + 300.0/num*k;
						double max_wl = min_wl + 300.0/num - 0.00001;
						//double max_wl = min_wl + SAMPLE_SIZE - 0.00001;
						//double max_wl = 400 + 300.0/(double)num*(double)(k+1) - 0.00001;
						r.min_wl = min_wl;
						r.max_wl = max_wl;
						r.central_wl = (min_wl + max_wl) / 2.0;
						double rad = GetRadiance(r, 0);
						//if (rad > std::numeric_limits<double>::max()) {
						//	std::cout << "ALARM" << std::endl;
						//}
						if (!std::isnan(rad)) {
							radiance.add(rad/ns, min_wl, max_wl);
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
			if (count % 5000 == 0) {
#ifdef _OPENMP
				std::cout << "thread: " << omp_get_thread_num() << "  ";
#endif
				std::cout << 100.0 * static_cast<double>(count) / (nx*ny) << "%" << std::endl;
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

void Scene::Load(const char *filename)
{
	obj_loader.Load(filename);
	//for (const auto& s : obj_loader.mtl_file) {
	//	mtl_loader.Load(s.c_str());
	//}
	material_loader.Load("test.material");
	for (int i = 0; i < obj_loader.objects.size(); i++) {
		std::cout << "name: " << obj_loader.objects[i]->material_name << std::endl;
		//std::shared_ptr<material> mat = material_loader.materials.at(obj_loader.objects[i]->material_name);
		//materials.push_back(mat);
	}
	world = std::make_unique<objmodel>(obj_loader);
}


void Scene::RenderResultWindow(void)
{
	ImGui::Begin("Render", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Render")) {
			if (ImGui::MenuItem("Render Image")) {
				std::cout << img_width << " " << img_height << std::endl;
				if (img != nullptr)
					delete[] img;
				img = new GLubyte[img_width*img_height*4];
				for (int i = 0; i < img_width*img_height*4; i++)
					img[i] = 255;
				//std::memset(img, 255, img_width*img_height*4);
				img_updated = true;
				//set_camera(0, 0, 3, 1, 0, 0, M_PI/2.0, 60, (double)img_width/img_height);
				vec3 veccameraPos = vec3(cameraPos.x, cameraPos.y, cameraPos.z);
				vec3 veccameraUp = vec3(cameraUp.x, cameraUp.y, cameraUp.z);
				glm::vec3 lookat = cameraPos + cameraFront;
				vec3 vlookat = vec3(lookat.x, lookat.y, lookat.z);
				//cam.set_camera(cameraPos, vlookat, cameraUp, 60, (double)img_width/img_height);
				cam.set_camera(veccameraPos, vlookat, veccameraUp, static_cast<double>(img_width)/img_height, d, focal_length, aperture);
				std::thread t(&Scene::RenderImage, this, img_width, img_height, img_samples, "test.pnm");
				t.detach();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImGui::SliderInt("Image Width", &img_width, 0, 2000);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::SliderInt("Image Height", &img_height, 0, 2000);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::SliderInt("Image Samples", &img_samples, 0, 1000);            // Edit 1 float using a slider from 0.0f to 1.0f
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

	const char *items[material_loader.materials.size()];
	static int cur_item = -1;
	static int last_item = -1;
	int i = 0;
	for (const auto& m : material_loader.materials) {
		items[i] = m.first.c_str();
		if (objecti != last_objecti) {
			if (m.first == obj_loader.objects[objecti]->material_name) {
				cur_item = i;
			}
		}
		i++;
	}
	ImGui::Combo("select material", &cur_item, items, material_loader.materials.size());
	if (objecti == last_objecti && cur_item != last_item) {
		obj_loader.objects[objecti]->material_name = std::string(items[cur_item]);
	}

	auto it = material_loader.materials.find(items[cur_item]);
	char str[32] = "";
	if (ImGui::InputText("Press Enter to add new material", &str[0], sizeof(str)/sizeof(char), ImGuiInputTextFlags_EnterReturnsTrue)) {
		material_loader.materials[std::string(str)] = std::make_shared<lambertian>(Spectrum(1));
		it = material_loader.materials.find(str);
		cur_item = std::distance(material_loader.materials.begin(), it);
		obj_loader.objects[objecti]->material_name = std::string(str);
	}

	if (cur_item != -1) {
		const char *model_items[2] = {"Lambertian", "light"};
		static int cur_model_item = -1;
		static int last_model_item = -1;
		if (cur_item != last_item)
			last_model_item = -1;

		std::shared_ptr<material> mat = it->second;
		auto& id = typeid(*mat);
		if (id == typeid(lambertian))
			cur_model_item = 0;
		else if (id == typeid(diffuse_light))
			cur_model_item = 1;

		ImGui::Combo("select model", &cur_model_item, model_items, 2);
		if (cur_model_item == 0) {
			if (cur_model_item != last_model_item && last_model_item != -1) {
				mat = std::make_shared<lambertian>(Spectrum(1));
				it->second = mat;
			}
			std::shared_ptr<lambertian> mat_ptr = std::dynamic_pointer_cast<lambertian>(mat);
			ImGui::Text("Lambertian");
			const ImVec2 slider_size(18, 160);
			static float a[N_SAMPLE];
			if (last_item != cur_item || last_model_item != cur_model_item) {
				for (int i = 0; i < N_SAMPLE; i++) {
					a[i] = mat_ptr->albedo.data[i];
				}
			}
			for (int i = 0; i < N_SAMPLE; i++) {
				if (i > 0)
					ImGui::SameLine();
				ImGui::PushID(i);
				ImGui::VSliderFloat("##v", slider_size, &a[i], 0.0f, 1.0f, "");
				ImGui::PopID();
				mat_ptr->albedo.data[i] = a[i];
			}
			vec3 col = r_rgb(mat_ptr->albedo);
			ImVec4 color = ImVec4(col[0], col[1], col[2], 1.0f);
			colors[objecti][0] = col[0];
			colors[objecti][1] = col[1];
			colors[objecti][2] = col[2];
			ImGui::ColorButton("Albedo", color, ImGuiColorEditFlags_DisplayRGB);
		} else if (cur_model_item == 1) {
			if (cur_model_item != last_model_item && last_model_item != -1) {
				mat = std::make_shared<diffuse_light>(Spectrum(0.05));
				it->second = mat;
			}
			std::shared_ptr<diffuse_light> mat_ptr = std::dynamic_pointer_cast<diffuse_light>(mat);
			ImGui::Text("Light");
			const ImVec2 slider_size(18, 160);
			static float a[N_SAMPLE];
			if (last_item != cur_item || last_model_item != cur_model_item) {
				for (int i = 0; i < N_SAMPLE; i++) {
					a[i] = mat_ptr->light_color.data[i];
				}
			}
			for (int i = 0; i < N_SAMPLE; i++) {
				if (i > 0)
					ImGui::SameLine();
				ImGui::PushID(i);
				ImGui::VSliderFloat("##v", slider_size, &a[i], 0.0f, 1.0f, "");
				ImGui::PopID();
				mat_ptr->light_color.data[i] = a[i];
			}
			vec3 col = unit_vector(r_rgb(mat_ptr->light_color));
			ImVec4 color = ImVec4(col[0], col[1], col[2], 1.0f);
			colors[objecti][0] = col[0];
			colors[objecti][1] = col[1];
			colors[objecti][2] = col[2];
			ImGui::ColorButton("Light color", color, ImGuiColorEditFlags_DisplayRGB);
		} else {
		}

		last_model_item = cur_model_item;
		if (ImGui::Button("Save material")) {
			material_loader.Write("test.material");
		}
	}


	last_objecti = objecti;
	last_item = cur_item;

	ImGui::End();
}
