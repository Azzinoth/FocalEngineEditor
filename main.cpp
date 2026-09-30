#include "FEEditor.h"

void OnTriggerRelease()
{
	int y = 0;
	y++;

	FEOpenXR_INPUT.TriggerHapticFeedback(0.5f, 0.5f, 0.5f, false);

	//std::string test = FEOpenXR_INPUT.CurrentlyActiveInteractionProfile(true);
	//test = ";";
}

void OnSomething(float Value)
{
	int y = 0;
	y++;

	FEOpenXR_INPUT.TriggerHapticFeedback(0.5f, 0.5f, 0.5f, false);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ENGINE.InitWindow();
	ENGINE.SetVsyncEnabled(true);
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

	FEOpenXR_INPUT.SetLeftValveSqueezeValueCallBack(OnSomething);

	while (ENGINE.IsNotTerminated())
	{
		PROFILING.StartProfiling();

		ENGINE.BeginFrame();
		EDITOR.UpdateBeforeRender();
		ENGINE.Render();
		LEIA_3D_MANAGER.Render();

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
		}

		bool bVRMode = ENGINE.IsVREnabled();
		if (ImGui::Checkbox("Enter VR mode", &bVRMode))
		{
			if (bVRMode)
			{
				if (ENGINE.EnableVR())
				{

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

#ifdef FOCAL_ENGINE_LEIA_3D_MONITOR
		bool b3DMonitorMode = LEIA_3D_MANAGER.Is3DModeEnabled();
		if (ImGui::Checkbox("Leia 3D monitor mode", &b3DMonitorMode))
		{
			LEIA_3D_MANAGER.Set3DModeEnabled(b3DMonitorMode);
		}

		static FEViewport* LeiaViewport = nullptr;
		if (ImGui::Begin("Leia 3D monitor rendering", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			if (LeiaViewport == nullptr)
			{
				std::string ViewPortID = ENGINE.CreateViewport(FE_IMGUI_WINDOW_MANAGER.GetCurrentWindowImpl());
				LeiaViewport = ENGINE.GetViewport(ViewPortID);
			}

			FETexture* FinalResult = LEIA_3D_MANAGER.GetFinalResult();
			// Whole pixels, because each "3D" pixel should be shown exactly on the pixel it was meant for.
			const ImVec2 Position = ImFloor(ImGui::GetCursorScreenPos());
			const ImVec2 Size = ImFloor(ImGui::GetContentRegionAvail());
			if (FinalResult != nullptr && Size.x > 0.0f && Size.y > 0.0f)
			{
				// Final result has the size of the main window, so each screen pixel shows the texel at the same position.
				const ImVec2 Scale = ImGui::GetIO().DisplayFramebufferScale;
				const float TextureWidth = static_cast<float>(FinalResult->GetWidth());
				const float TextureHeight = static_cast<float>(FinalResult->GetHeight());
				// Texture is upside down for ImGui, so V of the top edge is bigger than V of the bottom edge.
				const ImVec2 TopLeftUV = ImVec2(Position.x * Scale.x / TextureWidth, 1.0f - Position.y * Scale.y / TextureHeight);
				const ImVec2 BottomRightUV = ImVec2((Position.x + Size.x) * Scale.x / TextureWidth, 1.0f - (Position.y + Size.y) * Scale.y / TextureHeight);

				ImGui::SetCursorScreenPos(Position);
				ImGui::Image(FinalResult->GetTextureID(), Size, TopLeftUV, BottomRightUV);
			}
		}
		ImGui::End();

		if (ImGui::Button("Set scene in focus current scene for Leia 3D"))
		{
			FEScene* FocusedScene = EDITOR.GetFocusedScene();
			if (FocusedScene != nullptr)
			{
				if (LeiaViewport != nullptr)
					LEIA_3D_MANAGER.Initialize(FocusedScene->GetObjectID(), LeiaViewport->GetID());
			}
		}
#endif

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

		std::string Caption = "CPU time : ";
		Caption += CPUMs;
		Caption += " ms";
		Caption += "  GPU time : ";
		Caption += GPUMs;
		Caption += " ms";
		Caption += "  Frame time : ";
		Caption += FrameMs;
		Caption += " ms";

		ENGINE.SetWindowCaption(Caption.c_str());

		PROFILING.StopProfiling();
		if (bPutThisFrameToTimeline)
		{
			PROFILING.SaveTimelineToJSON("timeline.json");
			bPutThisFrameToTimeline = false;
		}
	}

	return 0;
}