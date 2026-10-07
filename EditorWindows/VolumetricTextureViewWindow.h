#pragma once

#include "CombineChannelsToTexturePopUp.h"
#include "SceneWindow/FEEditorSceneWindow.h"

class VolumetricTextureViewWindow : public FEEditorSceneWindow
{
	friend class FEPrefabEditorManager;

	FEUUID TextureToViewID;
	FEUUID ViewCameraID;

	std::string ConvertStatusMessage;
	bool bConvertSucceeded = false;

	FENewMaterial* MaterialFor3DTextures = nullptr;
	static FEScene* CreateViewScene(FETexture* TextureToView);
	FEEntity* InjectModelViewCamera(FEScene* Scene);

	void RenderInfoOverlay();

	~VolumetricTextureViewWindow();

	ImGuiButton* CloseButton = nullptr;
public:
	VolumetricTextureViewWindow(FETexture* TextureToView);
	void Render() override;
};