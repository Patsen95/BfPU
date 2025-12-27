#include "app.h"

#include <Windows.h>
#include <string>
#include <chrono>


#define _CRT_SECURE_NO_WARNINGS

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

//#pragma warning(disable : 4267)



namespace p95
{
	static void glfw_error_callback(int error, const char* description) {}

	/******************************************************************************/
	const char* App::VERSION = "1.0";

	static std::chrono::steady_clock::time_point startTime;

	/******************************************************************************/
	static constexpr ImVec2 SIZE_WINDOW(1024, 768);
	static constexpr ImColor COLOR_MEMO_CONTENT(50, 95, 55, 255);
	static constexpr ImColor COLOR_CELL_FRAME(158, 47, 47, 255);

	/******************************************************************************/
	App::App() :
		m_window(nullptr),
		m_io(nullptr),
		m_windowSize(SIZE_WINDOW),
		m_frameBufWidth(0),
		m_frameBufHeight(0)
	{
		m_simConfig.intructionsPerSec = 5;
		m_simConfig.maxDataMemorySize = 32;

		m_machine = new bf::BF_Machine();
		m_machine->init(&m_simConfig);

		// "HELLO WORLD!" source
		m_machine->m_sourceBuffer = "++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.\n<<+++++++++++++++.>.+++.------.--------.>+.>.";
	}

	App::~App() 
	{
	}

	void App::initUI()
	{
		glfwSetErrorCallback(glfw_error_callback);
		if(!glfwInit())
			return;

		// GL 3.0 + GLSL 130
		const char* glslVersion = "#version 130";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_window = glfwCreateWindow((int)m_windowSize.x, (int)m_windowSize.y, "Brainfuck sim", nullptr, nullptr);
		if(!m_window)
			return;

		glfwMakeContextCurrent(m_window);
		glfwSwapInterval(1); // VSync

		IMGUI_CHECKVERSION();
		imgui::CreateContext();
		m_io = &imgui::GetIO();
		m_io->IniFilename = NULL;
		m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		imgui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(m_window, true);
		ImGui_ImplOpenGL3_Init(glslVersion);
	}

	void App::loop()
	{
		startTime = std::chrono::steady_clock::now();
		
		while(!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();
			if(glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0)
			{
				ImGui_ImplGlfw_Sleep(10);
				continue;
			}

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			imgui::NewFrame();

			drawUI();

			imgui::Render();
			glfwGetFramebufferSize(m_window, &m_frameBufWidth, &m_frameBufHeight);
			glViewport(0, 0, (GLsizei)m_windowSize.x, (GLsizei)m_windowSize.y);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(imgui::GetDrawData());

			glfwSwapBuffers(m_window);
		}
	}

	void App::shutdown()
	{
		m_machine->reset();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		imgui::DestroyContext();

		glfwDestroyWindow(m_window);
		glfwTerminate();
		m_window = nullptr;
		m_io = nullptr;
		m_machine = nullptr;

		delete m_window;
		delete m_io;
		delete m_machine;
	}

	/******************************************************************************/
	void App::drawUI()
	{
		static const int _PANEL_FLAGS = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf;
		static ImVec2 _dispSize = m_io->DisplaySize;

		imgui::SetNextWindowPos(ImVec2());
		imgui::SetNextWindowSize(_dispSize);
		imgui::Begin("##main_window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDecoration);
		{
			// MEMORY DISPLAY
			imgui::SetNextWindowPos(ImVec2());
			imgui::BeginChild("##panel_memo", { _dispSize.x * 0.65f, _dispSize.y * 0.6f }, ImGuiChildFlags_Borders);
			{
				if(ImGui::BeginTabBar("##tabs"))
				{
					if(ImGui::BeginTabItem("Program memory"))
					{
						drawProgMemoryView();
						ImGui::EndTabItem();
					}
					if(ImGui::BeginTabItem("Data memory"))
					{
						drawDataMemoryView();
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
				imgui::EndChild();
			}

			// CODE EXECUTION & CONFIG PANEL
			imgui::SetNextWindowPos({ _dispSize.x - (_dispSize.x * 0.35f), 0 });
			imgui::BeginChild("##panel_exec", { _dispSize.x * 0.35f, _dispSize.y }, ImGuiChildFlags_Borders);
			{
				if(imgui::CollapsingHeader("Execution & config", _PANEL_FLAGS))
					drawExecPanel();

				imgui::EndChild();
			}
			
			// SOURCE INPUT PANEL
			imgui::SetNextWindowPos({ 0, _dispSize.y * 0.6f });
			imgui::BeginChild("##panel_source", { _dispSize.x * 0.65f, _dispSize.y * 0.4f }, ImGuiChildFlags_Borders);
			{
				if(imgui::CollapsingHeader("Source code", _PANEL_FLAGS))
				{
					static char _srcBuf[m_machine->MAX_PROG_SOURCE_LEN] = { 0 };
					sprintf_s(_srcBuf, m_machine->m_sourceBuffer.c_str());
					imgui::InputTextMultiline("##input_source", _srcBuf, m_machine->MAX_PROG_SOURCE_LEN + 1, imgui::GetContentRegionAvail() - ImVec2(0, 30.f));
					m_machine->m_sourceBuffer = _srcBuf;
					
					//imgui::PushStyleVar(ImGuiStyleVar_CellPadding, {280.f, 0.f});
					imgui::PushStyleVar(ImGuiStyleVar_CellPadding, {125.f, 0.f});
					imgui::BeginTable("##tab", 2);
					{
						imgui::TableNextColumn();
						imgui::Text("%u / %u", m_machine->m_sourceBuffer.length(), m_machine->MAX_PROG_SOURCE_LEN);
						
						imgui::TableNextColumn();
						if(imgui::Button("Syntax"))
							imgui::OpenPopup("Syntax");

						imgui::SameLine();
						if(imgui::Button("Examples..."))
							imgui::OpenPopup("Examples");

						imgui::SameLine();
						if(imgui::Button("Clear"))
							m_machine->m_sourceBuffer.clear();
						
						if(imgui::BeginPopupModal("Syntax", NULL, ImGuiWindowFlags_AlwaysAutoResize))
						{
							std::string _str = "DP - data pointer\nIP - instruction pointer\n\n\n" \
												">\tIncrement DP\n\n" \
												"<\tDecrement DP\n\n" \
												"+\tIncrement byte at DP\n\n" \
												"-\tDecrement byte at DP\n\n" \
												".\tOutput byte at DP (to STD OUT)\n\n" \
												",\tRead byte (from STD IN) and store it at DP\n\n" \
												"[\tIf byte at DP is zero, set the IP next to the \"]\"\n\n" \
												"]\tIf byte at DP is non-zero, set the IP next to the \"[\"\n\n";

							imgui::TextUnformatted(_str.c_str());

							imgui::Separator();
							if(imgui::Button("Close"))
								imgui::CloseCurrentPopup();

							imgui::EndPopup();
						}

						if(imgui::BeginPopupModal("Examples", NULL, ImGuiWindowFlags_AlwaysAutoResize))
						{
							imgui::TextUnformatted("TODO");


							imgui::Spacing();
							imgui::Separator();
							if(imgui::Button("Close"))
								imgui::CloseCurrentPopup();

							imgui::EndPopup();
						}

						imgui::EndTable();
					}
					imgui::PopStyleVar();
				}
				imgui::EndChild();
			}
			imgui::End();
		}
	}

	void App::drawProgMemoryView()
	{
		static const unsigned int _DISPLAY_VALUES_COUNT = 16;
		const ImVec2 _CURRENT_CURSOR = imgui::GetCursorPos();
		static ImDrawList* _drawList = imgui::GetWindowDrawList();

		imgui::SetCursorPos({ 40.f, _CURRENT_CURSOR.y + 15.f });
		imgui::PushStyleVar(ImGuiStyleVar_CellPadding, { 8.0f, 5.f });
		imgui::BeginTable("#memo_hex_view", _DISPLAY_VALUES_COUNT + 1, // additional "offset" column
			ImGuiTableFlags_SizingFixedFit |
			ImGuiTableFlags_NoPadOuterX |
			ImGuiTableFlags_NoHostExtendX);
		{
			imgui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 75.0f);
			imgui::TableNextRow();

			size_t _progSize = m_machine->getProgMemoSize();

			for(size_t row = 0; row <= (_progSize + _DISPLAY_VALUES_COUNT - 1) / _DISPLAY_VALUES_COUNT; row++)
			{
				for(size_t col = 0; col < _DISPLAY_VALUES_COUNT + 1; col++)
				{
					imgui::TableSetColumnIndex(col);

					if(row == 0)
					{
						if(col == 0)
							imgui::TextUnformatted("Offset(hex)");
						else
							imgui::Text("%02X", col - 1);
					}
					else if(col == 0)
						imgui::Text("%06X", (row - 1) * _DISPLAY_VALUES_COUNT);
					else
					{
						int _memoIdx = ((row - 1) * _DISPLAY_VALUES_COUNT) + (col - 1);
						//imgui::Text("%02X", _memoIdx < m_machine->getProgMemSize() ? m_machine->getProgMemory()[_memoIdx] : 0);
						imgui::PushStyleColor(ImGuiCol_Text, ImVec4(COLOR_MEMO_CONTENT));
						imgui::Text("%02X", m_machine->getProgMemory()[_memoIdx]);
						imgui::PopStyleColor();
					}
				}
				imgui::TableNextRow();
			}
			imgui::EndTable();
			imgui::PopStyleVar();

			/* Draw frame around cell pointed by instruction pointer*/
			if(_progSize > 0)
			{
				unsigned int _instrPtr = m_machine->getInstructionPtr();
				float _xOffset = _instrPtr % _DISPLAY_VALUES_COUNT;
				float _yOffset = _instrPtr / _DISPLAY_VALUES_COUNT;

				ImVec2 _frameStart = _CURRENT_CURSOR + ImVec2(121.f, 40.f) + ImVec2(_CURRENT_CURSOR.x * _xOffset * 3.75, 23.f * _yOffset);

				_drawList->AddRect(
					_frameStart,
					_frameStart + ImVec2(18.f, 18.f),
					COLOR_CELL_FRAME,
					NULL,
					NULL,
					1.f
				);
			}
		}
	}

	void App::drawDataMemoryView()
	{
		static const unsigned int _DISPLAY_VALUES_COUNT = 16;
		const ImVec2 _CURRENT_CURSOR = imgui::GetCursorPos();

		imgui::SetCursorPos({ 40.f, _CURRENT_CURSOR.y + 15.f });
		imgui::PushStyleVar(ImGuiStyleVar_CellPadding, { 8.0f, 5.f });
		imgui::BeginTable("#memo_hex_view", _DISPLAY_VALUES_COUNT + 1, // additional "offset" column
			ImGuiTableFlags_SizingFixedFit |
			ImGuiTableFlags_NoPadOuterX |
			ImGuiTableFlags_NoHostExtendX);
		{
			imgui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 75.f);
			imgui::TableNextRow();

			size_t _dataMemoryCapacity = m_machine->getDataMemoCapacity();

			for(size_t row = 0; row <= (_dataMemoryCapacity + _DISPLAY_VALUES_COUNT - 1) / _DISPLAY_VALUES_COUNT; row++)
			{
				for(size_t col = 0; col < _DISPLAY_VALUES_COUNT + 1; col++)
				{
					imgui::TableSetColumnIndex(col);

					if(row == 0)
					{
						if(col == 0)
							imgui::TextUnformatted("Offset(hex)");
						else
							imgui::Text("%02X", col - 1);
					}					
					else if(col == 0)
						imgui::Text("%06X", (row - 1) * _DISPLAY_VALUES_COUNT);
					else
					{
						int _memoIdx = ((row - 1) * _DISPLAY_VALUES_COUNT) + (col - 1);
						imgui::PushStyleColor(ImGuiCol_Text, ImVec4(COLOR_MEMO_CONTENT));
						imgui::Text("%02X", _memoIdx < m_machine->getDataMemoSize() ? m_machine->getDataMemory()[_memoIdx] : 0);
						imgui::PopStyleColor();
					}
				}
				imgui::TableNextRow();
			}
			imgui::EndTable();
			imgui::PopStyleVar();
		}
	}

	void App::drawExecPanel()
	{
		// SIM SECTION
		{
			static bool _steppingEnabled = false;

			if(m_simConfig.intructionsPerSec < 1) m_simConfig.intructionsPerSec = 1;
			if(m_simConfig.intructionsPerSec > bf::SimConfig::MAX_INSTR_PER_SEC) m_simConfig.intructionsPerSec = bf::SimConfig::MAX_INSTR_PER_SEC;

			/* Load BF source */
			imgui::SeparatorText("Simulation");
			imgui::Spacing();
			if(imgui::Button("LOAD SOURCE"))
			{
				if(m_machine->getState() == bf::MachineState::READY)
					m_machine->parseSource(m_machine->m_sourceBuffer);
			}
			imgui::SameLine();
			imgui::Text("  Program length: %d B", m_machine->getProgMemoSize());
			imgui::NewLine();

			/* Step */
			imgui::TextUnformatted("Step");
			imgui::SameLine();
			{
				if(m_machine->getProgMemoSize() < 1)
					imgui::BeginDisabled();

				{
					if(_steppingEnabled)
						imgui::BeginDisabled();

					imgui::PushStyleColor(ImGuiCol_Button, IM_COL32(57, 100, 0, 255));
					if(imgui::ArrowButton("btn_step", ImGuiDir_Right))
					{
						m_machine->setState(bf::MachineState::RUNNING);
						m_machine->tick();
					}
					else if(!_steppingEnabled)
						m_machine->setState(bf::MachineState::READY);
					imgui::PopStyleColor();

					if(_steppingEnabled)
						imgui::EndDisabled();
				}

				/* Auto-stepping */
				imgui::Checkbox("Auto-step", &_steppingEnabled);
				
				if(_steppingEnabled && m_machine->getState() != bf::MachineState::HALTED)
				{
					if(m_machine->getProgMemoSize() > 0)
					{
						m_machine->setState(bf::MachineState::RUNNING);

						auto _timeNow = std::chrono::steady_clock::now();
						float _elapsed = std::chrono::duration<float, std::milli>(_timeNow - startTime).count();

						if(_elapsed >= 1000 / m_simConfig.intructionsPerSec)
						{
							m_machine->tick();
							startTime = _timeNow;
						}
					}
				}
				
				if(m_machine->getProgMemoSize() < 1)
					imgui::EndDisabled();
			}

			/* Auto-step instructions per second */
			imgui::SameLine();
			imgui::SetCursorPosX(imgui::GetCursorPosX() + 10.f);
			imgui::PushItemWidth(70.f);
			imgui::InputInt("instructions / sec", &m_simConfig.intructionsPerSec, 1, 5);
			imgui::PopItemWidth();
			imgui::NewLine();
			
			/* Memory reset*/
			imgui::PushStyleColor(ImGuiCol_Button, IM_COL32(194, 124, 50, 255));
			if(imgui::Button("Reset data memory"))
				m_machine->clearDataMemory();

			/* Sim reset*/
			imgui::SameLine();
			imgui::PushStyleColor(ImGuiCol_Button, IM_COL32(100, 0, 0, 255));
			if(imgui::Button("Reset sim"))
				m_machine->reset();
			imgui::PopStyleColor(2);
		}

		// STATUS SECTION
		{
			imgui::NewLine();
			imgui::Spacing();
			imgui::SeparatorText("Status");
			imgui::Text("Machine state: %s", stateToStr(m_machine->getState()));
			imgui::Text("Clock ticks: %u", m_machine->getTicks());
			imgui::Text("DP: 0x%02X", m_machine->getDataPtr());
			imgui::Text("IP: 0x%02X", m_machine->getInstructionPtr());
			imgui::Text("Current instruction: %c (0x%02X)", m_machine->getCurrentInstruction(), m_machine->getCurrentInstruction());
			imgui::Text("Data memory size limit: %u B", m_machine->getDataMemoCapacity());
			
			{
				imgui::NewLine();
				imgui::SeparatorText("IO");

				imgui::AlignTextToFramePadding();
				imgui::TextUnformatted("STD IN:");
				imgui::SameLine();

				//static char _tmp_stdinBuf[m_machine->MAX_STD_IN_SIZE + 1] = { 0 };
				if(m_machine->getStdInSize() <= m_machine->MAX_STD_IN_SIZE)
				{
					static std::string _stdIn = m_machine->getStdIn();
					imgui::InputText("##input_stdin", &_stdIn);
					m_machine->writeToStdInBuffer(_stdIn);
				}

				imgui::Text("Buffer size: %u B", m_machine->getStdInSize());

				imgui::NewLine();
				imgui::AlignTextToFramePadding();
				imgui::TextUnformatted("STD OUT:");
				imgui::SameLine();

				static std::string _stdOut = m_machine->getStdOut();
				imgui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(45, 45, 45, 255));
				//imgui::InputText("##input_stdout", (char*)m_machine->getStdOut().c_str(), m_machine->MAX_STD_OUT_SIZE + 1, ImGuiInputTextFlags_ReadOnly);
				imgui::InputText("##input_stdout", &_stdOut, ImGuiInputTextFlags_ReadOnly);
				imgui::PopStyleColor();
				imgui::Text("Buffer size: %u B", m_machine->getStdOutSize());

				imgui::NewLine();
				if(imgui::Button("Clear IO buffers"))
				{
					m_machine->clearIOBuffers();
					//sprintf_s(_tmp_stdinBuf, "");
				}
			}			
		}

		// SIM CONFIG SECTION
		{
			if(m_simConfig.maxProgramMemorySize < 64) m_simConfig.maxProgramMemorySize = 64;
			if(m_simConfig.maxDataMemorySize < 32) m_simConfig.maxDataMemorySize = 32;

			imgui::NewLine();
			imgui::Spacing();
			imgui::SeparatorText("Sim config");
			imgui::Spacing();

			imgui::TextUnformatted("Program memory limit");
			imgui::SetNextItemWidth(100.f);
			imgui::InputInt("##", &m_simConfig.maxProgramMemorySize, 64, 512);
			imgui::SameLine(); imgui::TextUnformatted("bytes");

			imgui::Spacing();

			imgui::TextUnformatted("Data memory limit");
			imgui::SetNextItemWidth(100.f);
			imgui::InputInt("##", &m_simConfig.maxDataMemorySize, 32, 512);
			imgui::SameLine(); imgui::TextUnformatted("bytes");

			imgui::NewLine(); imgui::NewLine();
			if(imgui::Button("Apply"))
			{

			}
		}

		// FOOTER
		{
			ImVec2 _currCursor = imgui::GetCursorPos();
			std::string _footerText = "Brainfuck Sim v" + std::string(VERSION) + "(@Patsen95)";
			imgui::SetCursorPosY(imgui::GetContentRegionMax().y - 18);
			imgui::Separator();
			imgui::SetCursorPosX(_currCursor.x + imgui::CalcTextSize(_footerText.c_str()).x / 3);
			imgui::TextUnformatted(_footerText.c_str());
		}
	}
}