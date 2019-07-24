#include <iostream>
#include <algorithm>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "filebrowser.h"

FileBrowser::FileBrowser(void)
{
}

FileBrowser::FileBrowser(const char *dir)
{
	current_dir = dir;
	dirp = opendir(dir);
}

FileBrowser::~FileBrowser(void)
{
	if (dirp != nullptr)
		closedir(dirp);
}

static bool comp(const std::string &lh, const std::string &rh) {
	return lh.compare(rh) <= 0;
}

bool FileBrowser::OpenDir(const char *dir)
{
	dirp = opendir(dir);
	if (dirp == nullptr)
		return false;
	long loc = telldir(dirp);
	struct dirent *ent;
	do {
		ent = readdir(dirp);
		if (ent != nullptr) {
			switch (ent->d_type) {
				case DT_DIR:
					entries.push_back(std::string("D: ") + ent->d_name);
					break;
				case DT_REG:
					entries.push_back(std::string("R: ") + ent->d_name);
					break;
				default:
					break;
			}
		}
	} while (ent != nullptr);
	std::sort(entries.begin(), entries.end(), comp);
	seekdir(dirp, loc);
	current_dir = dir;
	return true;
}

std::string FileBrowser::Render(void)
{
	std::string selected_dir = "";
	std::string selected_file = "";

	ImGui::Columns(4, "columns", false);
	for (const auto& e : entries) {
		switch (e[0]) {
			case 'D':
				if (ImGui::Selectable(e.c_str())) {
					selected_dir = current_dir + "/" + e.substr(3);;
				}
				break;
			case 'R':
				if (ImGui::Selectable(e.c_str())) {
					selected_file = current_dir + "/" + e.substr(3);
				}
				break;
			default:
				break;
		}
		ImGui::NextColumn();
	}

	if (selected_dir != "") {
		if (dirp != nullptr)
			closedir(dirp);
		entries.clear();
		OpenDir(selected_dir.c_str());
	}
	return std::string(selected_file);
}
