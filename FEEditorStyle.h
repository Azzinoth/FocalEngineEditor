#pragma once

#include "../FEngine.h"

// Colors of the editor interface.
namespace FEEditorStyle
{
	constexpr ImU32 Background = IM_COL32(0x16, 0x18, 0x1C, 255);
	constexpr ImU32 Panel = IM_COL32(0x1D, 0x20, 0x25, 255);
	constexpr ImU32 Card = IM_COL32(0x23, 0x26, 0x2C, 255);
	constexpr ImU32 CardHovered = IM_COL32(0x2A, 0x2E, 0x35, 255);
	constexpr ImU32 CardBorder = IM_COL32(0x34, 0x39, 0x41, 255);
	constexpr ImU32 PopupBorder = IM_COL32(0x4A, 0x50, 0x5A, 255);

	constexpr ImU32 Text = IM_COL32(0xE8, 0xE9, 0xEB, 255);
	constexpr ImU32 TextMuted = IM_COL32(0xA3, 0xA9, 0xB2, 255);
	constexpr ImU32 TextDisabled = IM_COL32(0x6B, 0x71, 0x7A, 255);
	constexpr ImU32 TextOnAccent = IM_COL32(0x06, 0x20, 0x2D, 255);

	constexpr ImU32 Accent = IM_COL32(0x00, 0xA2, 0xE8, 255);
	constexpr ImU32 AccentHovered = IM_COL32(0x2B, 0xB6, 0xF2, 255);
	constexpr ImU32 AccentMuted = IM_COL32(0x0B, 0x6C, 0x99, 255);
	constexpr ImU32 SelectedRow = IM_COL32(0x26, 0x30, 0x3A, 255);
	// Selected item of a list that has rows of its own colors.
	constexpr ImU32 SelectedItem = IM_COL32(0x14, 0x52, 0x70, 255);

	constexpr ImU32 Button = IM_COL32(0x2E, 0x32, 0x39, 255);
	constexpr ImU32 ButtonHovered = IM_COL32(0x3A, 0x3F, 0x48, 255);
	constexpr ImU32 ButtonBorder = IM_COL32(0x45, 0x4B, 0x55, 255);

	// Button of the main action that has light text, unlike Accent that needs TextOnAccent.
	constexpr ImU32 PrimaryButton = IM_COL32(0x0B, 0x6C, 0x99, 255);
	constexpr ImU32 PrimaryButtonHovered = IM_COL32(0x0E, 0x86, 0xBD, 255);

	constexpr ImU32 Success = IM_COL32(0x73, 0xCC, 0x73, 255);
	constexpr ImU32 Warning = IM_COL32(0xF0, 0xB8, 0x40, 255);
	constexpr ImU32 Danger = IM_COL32(0xFF, 0x7A, 0x70, 255);
	constexpr ImU32 DangerButton = IM_COL32(0xC8, 0x45, 0x3B, 255);
	constexpr ImU32 DangerButtonHovered = IM_COL32(0xD9, 0x56, 0x4C, 255);

	constexpr ImU32 AxisX = IM_COL32(0xE0, 0x5A, 0x5A, 255);
	constexpr ImU32 AxisY = IM_COL32(0x6C, 0xC0, 0x6C, 255);
	constexpr ImU32 AxisZ = IM_COL32(0x5A, 0x8F, 0xE0, 255);

	// Resource types in the content browser.
	constexpr ImU32 TypeShader = IM_COL32(0xB0, 0x8C, 0xE8, 255);
	constexpr ImU32 TypeMesh = IM_COL32(0x4A, 0xC0, 0xD0, 255);
	constexpr ImU32 TypeTexture = IM_COL32(0xE0, 0x8A, 0x4A, 255);
	constexpr ImU32 TypeMaterial = IM_COL32(0x6C, 0xC0, 0x6C, 255);
	constexpr ImU32 TypeGameModel = IM_COL32(0xE8, 0xC8, 0x50, 255);
	constexpr ImU32 TypePointCloud = IM_COL32(0x4A, 0xD0, 0xA8, 255);
	constexpr ImU32 TypePrefab = IM_COL32(0x5A, 0x8F, 0xE0, 255);
	constexpr ImU32 TypeScene = IM_COL32(0xE0, 0x6A, 0xA8, 255);
	constexpr ImU32 TypeAssetPackage = IM_COL32(0xB0, 0x90, 0x70, 255);
	constexpr ImU32 TypeNativeScriptModule = IM_COL32(0x9A, 0xA8, 0xC0, 255);

	// Components that have no resource type of their own, the others take the color of their resource type.
	constexpr ImU32 ComponentCamera = IM_COL32(0xE8, 0x7A, 0x6A, 255);
	constexpr ImU32 ComponentLight = IM_COL32(0xF5, 0xE6, 0xA8, 255);
	constexpr ImU32 ComponentInstanced = IM_COL32(0xD0, 0x7A, 0xD8, 255);
	constexpr ImU32 ComponentTerrain = IM_COL32(0xA6, 0xCC, 0x5A, 255);
	constexpr ImU32 ComponentVirtualUI = IM_COL32(0x62, 0xB8, 0xF0, 255);
	constexpr ImU32 ComponentVolume = IM_COL32(0xC4, 0xBC, 0xF0, 255);

	// Drawn over content that is covered by a window or a popup.
	constexpr ImU32 Dim = IM_COL32(0x0E, 0x0F, 0x12, 200);

	inline ImVec4 ToVec4(const ImU32 Color)
	{
		return ImGui::ColorConvertU32ToFloat4(Color);
	}

	// First color at weight 0, second at weight 1.
	inline ImU32 Mix(const ImU32 First, const ImU32 Second, const float Weight)
	{
		const ImVec4 A = ToVec4(First);
		const ImVec4 B = ToVec4(Second);
		return ImGui::ColorConvertFloat4ToU32(ImVec4(A.x + (B.x - A.x) * Weight, A.y + (B.y - A.y) * Weight, A.z + (B.z - A.z) * Weight, A.w + (B.w - A.w) * Weight));
	}

	// Color of a resource type, the only place where a type gets its color.
	inline ImU32 GetTypeColor(const FE_OBJECT_TYPE Type)
	{
		switch (Type)
		{
			case FE_SHADER: return TypeShader;
			case FE_MESH: return TypeMesh;
			case FE_TEXTURE: return TypeTexture;
			case FE_MATERIAL: return TypeMaterial;
			case FE_GAMEMODEL: return TypeGameModel;
			case FE_POINT_CLOUD: return TypePointCloud;
			case FE_PREFAB: return TypePrefab;
			case FE_SCENE: return TypeScene;
			case FE_ASSET_PACKAGE: return TypeAssetPackage;
			case FE_NATIVE_SCRIPT_MODULE: return TypeNativeScriptModule;
			default: return TextDisabled;
		}
	}

	// Color of a component, the only place where a component gets its color. A component without a color of its own has color of the text.
	inline ImU32 GetComponentColor(const std::type_info& ComponentType)
	{
		if (ComponentType == typeid(FEGameModelComponent))
			return GetTypeColor(FE_GAMEMODEL);

		if (ComponentType == typeid(FEPointCloudComponent))
			return GetTypeColor(FE_POINT_CLOUD);

		if (ComponentType == typeid(FENativeScriptComponent))
			return GetTypeColor(FE_NATIVE_SCRIPT_MODULE);

		if (ComponentType == typeid(FEPrefabInstanceComponent))
			return GetTypeColor(FE_PREFAB);

		if (ComponentType == typeid(FECameraComponent))
			return ComponentCamera;

		if (ComponentType == typeid(FELightComponent))
			return ComponentLight;

		if (ComponentType == typeid(FEInstancedComponent))
			return ComponentInstanced;

		if (ComponentType == typeid(FETerrainComponent))
			return ComponentTerrain;

		if (ComponentType == typeid(FEVirtualUIComponent))
			return ComponentVirtualUI;

		if (ComponentType == typeid(FEVolumeComponent))
			return ComponentVolume;

		return Text;
	}
}
