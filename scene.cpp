#include <iostream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "scene.h"
#include "vec3.h"

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
	obj_loader.Load("test.obj");
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
		colors.push_back(std::array<float, 3>({(float)drand48(), (float)drand48(), (float)drand48()}));

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


void Scene::Render(void)
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
		glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.5*colors[o][0], 0.5*colors[o][1], 0.5*colors[o][2]);
		if (activeObjectIndex != 0 && o == (activeObjectIndex-1)) {
			glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.8f, 0.8f, 0.8f);
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
				glm::vec3 v(vertices_array[index-1][0], vertices_array[index-1][1], vertices_array[index-1][2]);
				std::cout << v[0] << std::endl;
				std::cout << v[1] << std::endl;
				std::cout << v[2] << std::endl;
				float a = glm::length(v-cameraPos);
				std::cout << "a: " << a << std::endl;
				d = 1.0/(1.0/focal_length-1.0/a);
				std::cout << "d: " << d << std::endl;
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
