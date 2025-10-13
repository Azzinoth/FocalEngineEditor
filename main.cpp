#include "FEEditor.h"

std::string RemoveExcessiveIndentation(const std::string& PythonCode)
{
	std::istringstream PythonCodeStream(PythonCode);
	std::vector<std::string> CodeLines;
	CodeLines.reserve(128);

	// Read all lines
	for (std::string CurrentLine; std::getline(PythonCodeStream, CurrentLine); )
		CodeLines.emplace_back(std::move(CurrentLine));

	// If the FirstNonWhitespaceIndex line is empty, drop it
	size_t FirstLineIndex = 0;
	if (!CodeLines.empty() && CodeLines[0].find_first_not_of(" \t\r") == std::string::npos)
		FirstLineIndex = 1;

	// Find minimal common indentation across non-blank lines.
	size_t MinIndentCount = std::numeric_limits<size_t>::max();
	for (size_t i = FirstLineIndex; i < CodeLines.size(); i++)
	{
		const std::string& CurrentLine = CodeLines[i];
		size_t FirstNonWhitespaceIndex = CurrentLine.find_first_not_of(" \t\r");
		if (FirstNonWhitespaceIndex == std::string::npos)
			continue;

		size_t IndentCount = 0;
		while (IndentCount < CurrentLine.size() && (CurrentLine[IndentCount] == ' ' || CurrentLine[IndentCount] == '\t')) ++IndentCount;
		if (IndentCount < MinIndentCount) MinIndentCount = IndentCount;
	}

	if (MinIndentCount == std::numeric_limits<size_t>::max())
		MinIndentCount = 0;

	// Build output, trimming that many leading white spaces from non-blank CodeLines
	std::string TrimmedCode;
	TrimmedCode.reserve(PythonCode.size());
	for (size_t i = FirstLineIndex; i < CodeLines.size(); i++)
	{
		const std::string& CurrentLine = CodeLines[i];
		size_t FirstNonWhitespaceIndex = CurrentLine.find_first_not_of(" \t\r");
		if (FirstNonWhitespaceIndex == std::string::npos)
		{ 
			TrimmedCode += '\n';
			continue;
		}

		size_t CutIndex = 0;
		while (CutIndex < MinIndentCount && CutIndex < CurrentLine.size() && (CurrentLine[CutIndex] == ' ' || CurrentLine[CutIndex] == '\t'))
			++CutIndex;

		TrimmedCode += CurrentLine.substr(CutIndex);
		TrimmedCode += '\n';
	}

	return TrimmedCode;
}

bool RunSimpleString(const std::string& Command);

bool Initialize()
{
	Py_Initialize();

	if (!Py_IsInitialized())
		return false;

	const char* CaptureOutputCode = R"(
	import sys
	from io import StringIO
	class CaptureOutput:
		def __init__(self):
			self.Output = StringIO()
		def write(self, text):
			self.Output.write(text)
		def flush(self):
			pass

	Capture = CaptureOutput()
	sys.stdout = Capture
	sys.stderr = Capture

	print("Test text")
	)";

	RunSimpleString(CaptureOutputCode);

	return true;
}

bool IsInitialized()
{
	return Py_IsInitialized();
}

std::string GetOutput()
{
	if (!IsInitialized())
		return "";

	PyObject* Main = PyImport_AddModule("__main__");
	if (Main == nullptr)
		return "";

	PyObject* Capture = PyObject_GetAttrString(Main, "Capture");
	if (Capture == nullptr)
	{
		Py_XDECREF(Main);
		return "";
	}

	PyObject* Output = PyObject_GetAttrString(Capture, "Output");
	if (Output == nullptr)
	{
		Py_XDECREF(Capture);
		return "";
	}

	PyObject* Result = PyObject_CallMethod(Output, "getvalue", nullptr);
	if (Result == nullptr)
	{
		Py_XDECREF(Output);
		Py_XDECREF(Capture);
		return "";
	}

	std::string PythonOutputText;
	if (PyUnicode_Check(Result))
		PythonOutputText = PyUnicode_AsUTF8(Result);

	Py_XDECREF(Result);
	Py_XDECREF(Output);
	Py_XDECREF(Capture);

	return PythonOutputText;
}

bool Finalize()
{
	if (IsInitialized())
	{
		Py_Finalize();
		return true;
	}

	return false;
}

bool RunSimpleString(const std::string& Command)
{
	if (!IsInitialized())
		return false;

	std::string ConvertedLine = RemoveExcessiveIndentation(Command);

	int Result = PyRun_SimpleString(ConvertedLine.c_str());
	if (Result != 0)
		return false;
	
	return true;
}

void StartEmbeddedPython()
{
	if (!Initialize())
		return;

	
	return;


	Py_Finalize();
}

std::string ReadPythonScript(const std::string& filepath) {
	std::ifstream file(filepath);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open Python script: " + filepath);
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();

	if (content.empty()) {
		throw std::runtime_error("Python script is empty: " + filepath);
	}

	return content;
}

void OnTriggerRelease()
{
	int y = 0;
	y++;

	FEOpenXR_INPUT.TriggerHapticFeedback(0.5f, 0.5f, 0.5f, false);

	//std::string test = FEOpenXR_INPUT.CurrentlyActiveInteractionProfile(true);
	//test = ";";
}

void OnSomething(float value)
{
	int y = 0;
	y++;

	FEOpenXR_INPUT.TriggerHapticFeedback(0.5f, 0.5f, 0.5f, false);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ENGINE.InitWindow();
	EDITOR.InitializeResources();
	THREAD_POOL.SetConcurrentThreadCount(10);
	NODE_SYSTEM.Initialize();

	const int FrameCountTillMeasure = 20;
	double CPUFrameDurations[FrameCountTillMeasure] = { 0.0f };
	double GPUFrameDurations[FrameCountTillMeasure] = { 0.0f };
	int FrameCounter = 0;

	double AverageCpuFrameDuration = 0.0;
	double AverageGpuFrameDuration = 0.0;

	bool bPutThisFrameToTimeline = false;

	/*FEOpenXR_INPUT.SetLeftTriggerReleaseCallBack(OnTriggerRelease);
	FEOpenXR_INPUT.SetRightTriggerReleaseCallBack(OnTriggerRelease);*/
	StartEmbeddedPython();

	FEOpenXR_INPUT.SetLeftValveSqueezeValueCallBack(OnSomething);

	while (ENGINE.IsNotTerminated())
	{
		PROFILING.StartProfiling();

		ENGINE.BeginFrame();
		EDITOR.UpdateBeforeRender();
		ENGINE.Render();

#ifdef EDITOR_SELECTION_DEBUG_MODE
		if (EDITOR.GetFocusedScene() != nullptr)
		{
			FESelectionData* SelectedData = SELECTED.GetSceneData(EDITOR.GetFocusedScene()->GetObjectID());
			if (SelectedData != nullptr)
			{
				std::string ObjectsUnderMouse = "Count of considered entities: " + std::to_string(SelectedData->SceneEntitiesUnderMouse.size());
				ImGui::Text(ObjectsUnderMouse.c_str());

				std::string ColorIndex = "ColorIndex: " + std::to_string(SelectedData->ColorIndex);
				ImGui::Text(ColorIndex.c_str());

				std::string EntityUnderMouse = "Entity under mouse: ";
				if (SelectedData->ColorIndex != -1 && SelectedData->ColorIndex < SelectedData->SceneEntitiesUnderMouse.size())
				{
					EntityUnderMouse += SelectedData->SceneEntitiesUnderMouse[SelectedData->ColorIndex]->GetName();
				}
				else
				{
					EntityUnderMouse += "None";
				}
				ImGui::Text(EntityUnderMouse.c_str());

				ImGui::Image(SelectedData->PixelAccurateSelectionFB->GetColorAttachment()->GetTextureID(), ImVec2(256 * 4, 256 * 4), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			}
		}
#endif

		if (ImGui::Button("Put This Frame To Timeline"))
		{
			bPutThisFrameToTimeline = true;
		}

		if (PROJECT_MANAGER.GetCurrent() != nullptr)
		{
			if (ImGui::Button("Build"))
			{
				EDITOR_PROJECT_BUILD_SYSTEM.BuildExecutable(PROJECT_MANAGER.GetCurrent());
			}

			if (ImGui::Button("Test python"))
			{
				std::string Serverscript = ReadPythonScript("SubSystems/FocalEngine/SubSystems/Python/MCPServer.py");
				bool bRunPythonScriptSuccess = RunSimpleString(Serverscript);

				std::string text = GetOutput();

				int y = 0;
				y++;
			}
		}

		// Let Python threads run
		PyGILState_STATE gstate = PyGILState_Ensure();
		PyRun_SimpleString("pass");  // Dummy command to keep Python active
		PyGILState_Release(gstate);

		bool bVRMode = ENGINE.IsVREnabled();
		if (ImGui::Checkbox("Enter VR mode", &bVRMode))
		{
			if (bVRMode)
			{
				if (ENGINE.EnableVR())
				{

					std::string ActiveRuntime = FEOpenXR_CORE.GetActiveRuntimeInfo();
					std::vector<FEOpenXRExtensionInfo> ExtensionsInfo = FEOpenXR_CORE.GetAvailableExtensionsInfo();

					int y = 0;
					y++;
					//glm::vec2 VRResolution = OpenXR_MANAGER.EyeResolution();
					//POINT_MANAGER.RenderTargetResize(static_cast<int>(VRResolution.x), static_cast<int>(VRResolution.y));

					//AddVirtualUI();
				}

			}
			else
			{
				ENGINE.DisableVR();

				//POINT_MANAGER.RenderTargetResize(static_cast<int>(ENGINE.GetRenderTargetWidth()), static_cast<int>(ENGINE.GetRenderTargetHeight()));
			}
		}

		if (bVRMode)
		{
			glm::vec3 ControllerPosition = FEOpenXR_INPUT.GetLeftControllerPosition();
			ImGui::Text("Left Controller Position : ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(70);
			ImGui::DragFloat("##X Left Controller", &ControllerPosition[0], 0.01f);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(70);
			ImGui::DragFloat("##Y Left Controller", &ControllerPosition[1], 0.01f);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(70);
			ImGui::DragFloat("##Z Left Controller", &ControllerPosition[2], 0.01f);


			ControllerPosition = FEOpenXR_INPUT.GetRightControllerPosition();

			ImGui::Text("Right Controller Position : ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(70);
			ImGui::DragFloat("##X Right Controller", &ControllerPosition[0], 0.01f);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(70);
			ImGui::DragFloat("##Y Right Controller", &ControllerPosition[1], 0.01f);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(70);
			ImGui::DragFloat("##Z Right Controller", &ControllerPosition[2], 0.01f);

			if (ImGui::Button("Haptic"))
			{
				FEOpenXR_INPUT.TriggerHapticFeedback(0.5f, 0.5f, 0.5f, false);
			}
		}

		//ImGui::ShowDemoWindow();
		EDITOR.Render();
		ENGINE.EndFrame();

		// CPU and GPU Time
		CPUFrameDurations[FrameCounter] = ENGINE.GetCpuTime();
		GPUFrameDurations[FrameCounter] = ENGINE.GetGpuTime();
		FrameCounter++;

		if (FrameCounter > FrameCountTillMeasure - 1)
		{
			AverageCpuFrameDuration = 0.0f;
			AverageGpuFrameDuration = 0.0f;
			for (size_t i = 0; i < FrameCountTillMeasure; i++)
			{
				AverageCpuFrameDuration += CPUFrameDurations[i];
				AverageGpuFrameDuration += GPUFrameDurations[i];
			}
			AverageCpuFrameDuration /= FrameCountTillMeasure;
			AverageGpuFrameDuration /= FrameCountTillMeasure;

			FrameCounter = 0;
		}

		std::string CPUMs = std::to_string(AverageCpuFrameDuration);
		CPUMs.erase(CPUMs.begin() + 4, CPUMs.end());

		std::string GPUMs = std::to_string(AverageGpuFrameDuration);
		GPUMs.erase(GPUMs.begin() + 4, GPUMs.end());

		std::string FrameMs = std::to_string(AverageCpuFrameDuration + AverageGpuFrameDuration);
		FrameMs.erase(FrameMs.begin() + 4, FrameMs.end());

		std::string caption = "CPU time : ";
		caption += CPUMs;
		caption += " ms";
		caption += "  GPU time : ";
		caption += GPUMs;
		caption += " ms";
		caption += "  Frame time : ";
		caption += FrameMs;
		caption += " ms";

		ENGINE.SetWindowCaption(caption.c_str());

		PROFILING.StopProfiling();
		if (bPutThisFrameToTimeline)
		{
			PROFILING.SaveTimelineToJSON("timeline.json");
			bPutThisFrameToTimeline = false;
		}
	}

	return 0;
}