#pragma once

#include "../FEngine.h"

namespace FocalEngine
{
	namespace FEEditorResourceIDs
	{
		// Shaders.
		inline constexpr FEUUID HaloDrawObjectShader = FEUUID::from_string("6a1a8451-1406-5cce-b5bf-935a2d4c2cab").value();
		inline constexpr FEUUID HaloDrawInstancedObjectShader = FEUUID::from_string("46d24d6e-fe42-578a-bbc3-a008f717f21a").value();
		inline constexpr FEUUID HaloFinalShader = FEUUID::from_string("5f8bac6a-5786-5e59-b789-0e5334ea0623").value();
		inline constexpr FEUUID MeshPreviewShader = FEUUID::from_string("1689e70b-fb91-5dcb-8f4a-706283efab65").value();
		inline constexpr FEUUID PixelAccurateSelectionShader = FEUUID::from_string("7e6a723b-40a1-511a-9bb7-3b9ea88d50b6").value();
		inline constexpr FEUUID PixelAccurateInstancedSelectionShader = FEUUID::from_string("a009f2e9-39a0-5582-ba05-b79b55865210").value();
	}
}
