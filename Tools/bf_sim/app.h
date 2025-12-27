#pragma once

#include <stdio.h>
#include <string>

//#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#include "bfsim.h"



namespace p95
{
	namespace imgui = ImGui;

	class App
	{
	public:
		
		App();
		~App();

		void initUI();
		void loop();
		void shutdown();

	private:

		void drawUI();

		void drawProgMemoryView();
		void drawDataMemoryView();
		void drawExecPanel();

	private:

		static const char* VERSION;

		GLFWwindow* m_window;
		ImGuiIO* m_io;
		ImVec2 m_windowSize;
		int m_frameBufWidth;
		int m_frameBufHeight;

		bf::SimConfig m_simConfig;
		bf::BF_Machine* m_machine;

	};
}