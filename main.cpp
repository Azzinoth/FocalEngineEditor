#include "FEEditor.h"

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

	while (ENGINE.IsNotTerminated())
	{
		PROFILING.StartProfiling();

		ENGINE.BeginFrame();
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

				ImGui::Image((void*)(intptr_t)SelectedData->PixelAccurateSelectionFB->GetColorAttachment()->GetTextureID(), ImVec2(256 * 4, 256 * 4), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			}
		}
#endif

		if (ImGui::Button("Put This Frame To Timeline"))
		{
			bPutThisFrameToTimeline = true;
		}

		if (PROJECT_MANAGER.GetCurrent() != nullptr)
		{
			bool bIsVSProjectPresent = EDITOR_SCRIPTING_SYSTEM.IsCurrentProjectHaveVSProjectInitialized();
			std::string Text = "Is VS project present: ";
			Text += bIsVSProjectPresent ? "true" : "false";
			ImGui::Text(Text.c_str());

			if (!bIsVSProjectPresent)
			{
				if (ImGui::Button("Initialize VS Project"))
				{
					EDITOR_SCRIPTING_SYSTEM.GenerateNewNativeScriptProject("FirstScript");
				}
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