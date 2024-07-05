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
		std::string objectsUnderMouse = "SceneEntitiesUnderMouse: " + std::to_string(SELECTED.SceneEntitiesUnderMouse.size());
		ImGui::Text(objectsUnderMouse.c_str());

		std::string colorIndex = "colorIndex: " + std::to_string(SELECTED.ColorIndex);
		ImGui::Text(colorIndex.c_str());

		ImGui::Image((void*)(intptr_t)SELECTED.PixelAccurateSelectionFB->GetColorAttachment()->GetTextureID(), ImVec2(256 * 4, 256 * 4), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
#endif

		if (ImGui::Button("Put This Frame To Timeline"))
		{
			bPutThisFrameToTimeline = true;
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