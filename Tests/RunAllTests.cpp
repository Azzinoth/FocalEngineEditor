#include "RunAllTests.h"

#include <chrono>
#include <iostream>
#include <thread>

using namespace FocalEngine;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	auto KeepConsoleAlive = [](void*) {
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	};
	FEConsoleWindow* Console = APPLICATION.CreateConsoleWindow(KeepConsoleAlive, nullptr);
	Console->WaitForCreation();
	Console->SetTitle("FocalEngineEditor Tests");

	testing::GTEST_FLAG(output) = "xml:report.xml";
	testing::InitGoogleTest();

	ENGINE.InitWindow(1280, 720);
	THREAD_POOL.SetConcurrentThreadCount(10);

	int TestResult = RUN_ALL_TESTS();

	std::cout << std::endl << "Test suite finished with exit code " << TestResult << "." << std::endl;
	std::cout << "Close the GL window (or this console) to exit." << std::endl;

	while (ENGINE.IsNotTerminated())
	{
		ENGINE.BeginFrame();
		ENGINE.Render();
		ENGINE.EndFrame();
	}

	return TestResult;
}
