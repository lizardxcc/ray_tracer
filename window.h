#ifndef WINDOW_H
#define WINDOW_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Window {
	public:
		void exec(void);
	private:
		bool window_active = true;
};

#endif
