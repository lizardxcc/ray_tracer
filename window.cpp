#include "window.h"

void Window::exec(void)
{
	ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiSetCond_Once);
	if (window_active) {
		ImGui::Begin("Win Test", &window_active, ImGuiWindowFlags_MenuBar);
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		//ImGui::Begin("Win2 Test", &window_active, ImGuiWindowFlags_MenuBar);
		//ImGui::End();
		}

		ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Once);
		if (ImGui::TreeNode("group 1")) {
			ImGui::Text("test");
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("group 2")) {
			ImGui::Text("fps: %.2f", ImGui::GetIO().Framerate);
			ImGui::TreePop();
		}
		ImGui::End();
	}
}
