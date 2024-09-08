#include "FEEditorHaloSelectionEffect.h"
#include "FEEditor.h"
using namespace FocalEngine;

FEEditorHaloSelectionEffect::FEEditorHaloSelectionEffect() {}
FEEditorHaloSelectionEffect::~FEEditorHaloSelectionEffect() {}

void FEEditorHaloSelectionEffect::InitializeResources()
{
	HaloMaterial = RESOURCE_MANAGER.CreateMaterial("haloMaterial");
	RESOURCE_MANAGER.SetTag(HaloMaterial, EDITOR_RESOURCE_TAG);

	HaloDrawObjectShader = RESOURCE_MANAGER.CreateShader("HaloDrawObjectShader", RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_HaloDrawObject_VS.glsl").c_str(),
																				 RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_HaloDrawObject_FS.glsl").c_str(),
																				 nullptr,
																				 nullptr,
																				 nullptr,
																				 nullptr,
																				 "E4F5165B2E1B05321A182C77"/*"HaloDrawObjectShader"*/);
	
	RESOURCE_MANAGER.SetTag(HaloDrawObjectShader, EDITOR_RESOURCE_TAG);
	HaloMaterial->Shader = HaloDrawObjectShader;

	HaloDrawInstancedObjectShader = RESOURCE_MANAGER.CreateShader("HaloDrawInstancedObjectShader", RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_HaloDrawObject_INSTANCED_VS.glsl").c_str(),
																								   RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_HaloDrawObject_FS.glsl").c_str(),
																								   nullptr,
																								   nullptr,
																								   nullptr,
																								   nullptr,
																								   "16A2A65B2C1B013217219C67"/*"HaloDrawInstancedObjectShader"*/);
	RESOURCE_MANAGER.SetTag(HaloDrawInstancedObjectShader, EDITOR_RESOURCE_TAG);

	HaloFinalShader = RESOURCE_MANAGER.CreateShader("HaloFinalShader", RESOURCE_MANAGER.LoadGLSL("SubSystems//FocalEngine//CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl").c_str(),
																	   RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_HaloSelectionEffect_FS.glsl").c_str(),
																	   nullptr,
																	   nullptr,
																	   nullptr,
																	   nullptr,
																	   "4AC7365B2C1B07324721A127"/*"HaloFinalShader"*/);

	RESOURCE_MANAGER.SetTag(HaloFinalShader, EDITOR_RESOURCE_TAG);
}

void FEEditorHaloSelectionEffect::UpdateResources(FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	FEHaloSelectionData* CurrentHaloSelectionData = GetSceneData(Scene->GetObjectID());
	if (CurrentHaloSelectionData == nullptr)
		return;

	FECameraRenderingData* CameraData = RENDERER.GetCameraRenderingData(CAMERA_SYSTEM.GetMainCameraEntity(Scene));
	if (CameraData == nullptr)
		return;

	std::vector<FEPostProcess*>& PostProcesses = CameraData->PostProcessEffects;

	delete CurrentHaloSelectionData->HaloObjectsFB;
	CurrentHaloSelectionData->HaloObjectsFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT, CameraData->SceneToTextureFB->GetWidth(), CameraData->SceneToTextureFB->GetHeight());
	CurrentHaloSelectionData->PostProcess = ENGINE.CreatePostProcess("selectionHaloEffect", CameraData->SceneToTextureFB->GetWidth() / 4, CameraData->SceneToTextureFB->GetHeight() / 4);

	FEShader* BlurShader = RESOURCE_MANAGER.GetShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/);
	CurrentHaloSelectionData->PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_OWN_TEXTURE, BlurShader));
	CurrentHaloSelectionData->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	// because input texture at first stage is full resolution, we should blur harder to get simular blur on both sides.
	CurrentHaloSelectionData->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.5f * 4.0f, "BloomSize"));
	CurrentHaloSelectionData->PostProcess->Stages.back()->InTexture.push_back(CurrentHaloSelectionData->HaloObjectsFB->GetColorAttachment());
	CurrentHaloSelectionData->PostProcess->ReplaceOutTexture(0, RESOURCE_MANAGER.CreateSameFormatTexture(CameraData->SceneToTextureFB->GetColorAttachment(), CameraData->SceneToTextureFB->GetWidth() / 4, CameraData->SceneToTextureFB->GetHeight() / 4));

	CurrentHaloSelectionData->PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BlurShader));
	CurrentHaloSelectionData->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	CurrentHaloSelectionData->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.5f, "BloomSize"));
	CurrentHaloSelectionData->PostProcess->Stages.back()->InTexture.push_back(CurrentHaloSelectionData->PostProcess->Stages[0]->OutTexture);
	CurrentHaloSelectionData->PostProcess->ReplaceOutTexture(1, RESOURCE_MANAGER.CreateSameFormatTexture(CameraData->SceneToTextureFB->GetColorAttachment(), CameraData->SceneToTextureFB->GetWidth() / 4, CameraData->SceneToTextureFB->GetHeight() / 4));

	CurrentHaloSelectionData->PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BlurShader));
	CurrentHaloSelectionData->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	CurrentHaloSelectionData->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));
	CurrentHaloSelectionData->PostProcess->Stages.back()->InTexture.push_back(CurrentHaloSelectionData->PostProcess->Stages[0]->OutTexture);
	CurrentHaloSelectionData->PostProcess->ReplaceOutTexture(2, RESOURCE_MANAGER.CreateSameFormatTexture(CameraData->SceneToTextureFB->GetColorAttachment(), CameraData->SceneToTextureFB->GetWidth() / 4, CameraData->SceneToTextureFB->GetHeight() / 4));

	CurrentHaloSelectionData->PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BlurShader));
	CurrentHaloSelectionData->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	CurrentHaloSelectionData->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));
	CurrentHaloSelectionData->PostProcess->Stages.back()->InTexture.push_back(CurrentHaloSelectionData->PostProcess->Stages[0]->OutTexture);
	CurrentHaloSelectionData->PostProcess->ReplaceOutTexture(3, RESOURCE_MANAGER.CreateSameFormatTexture(CameraData->SceneToTextureFB->GetColorAttachment(), CameraData->SceneToTextureFB->GetWidth() / 4, CameraData->SceneToTextureFB->GetHeight() / 4));

	CurrentHaloSelectionData->PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_OWN_TEXTURE, HaloFinalShader));

	CurrentHaloSelectionData->PostProcess->Stages.back()->InTexture.push_back(PostProcesses[PostProcesses.size() - 1]->Stages.back()->OutTexture);
	CurrentHaloSelectionData->PostProcess->Stages.back()->InTextureSource.push_back(FE_POST_PROCESS_OWN_TEXTURE);
	CurrentHaloSelectionData->PostProcess->Stages.back()->InTexture.push_back(CurrentHaloSelectionData->PostProcess->Stages[3]->OutTexture);
	CurrentHaloSelectionData->PostProcess->Stages.back()->InTextureSource.push_back(FE_POST_PROCESS_OWN_TEXTURE);
	CurrentHaloSelectionData->PostProcess->Stages.back()->InTexture.push_back(CurrentHaloSelectionData->PostProcess->Stages[0]->InTexture[0]);
	CurrentHaloSelectionData->PostProcess->ReplaceOutTexture(4, RESOURCE_MANAGER.CreateSameFormatTexture(CameraData->SceneToTextureFB->GetColorAttachment(), CameraData->SceneToTextureFB->GetWidth(), CameraData->SceneToTextureFB->GetHeight()));

	RENDERER.AddPostProcess(CameraData, CurrentHaloSelectionData->PostProcess, true);
}

void FEEditorHaloSelectionEffect::ClearAllSceneData()
{
	auto PerSceneIterator = PerSceneData.begin();
	while (PerSceneIterator != PerSceneData.end())
	{
		delete PerSceneIterator->second;
		PerSceneIterator++;
	}

	PerSceneData.clear();
}

void FEEditorHaloSelectionEffect::ClearSceneData(const std::string& SceneID)
{
	auto FoundSceneData = PerSceneData.find(SceneID);
	if (FoundSceneData != PerSceneData.end())
	{
		delete FoundSceneData->second;
		PerSceneData.erase(FoundSceneData);
	}
}

void FEEditorHaloSelectionEffect::AddSceneData(const std::string& SceneID)
{
	FEScene* CurrentScene = SCENE_MANAGER.GetScene(SceneID);
	if (CurrentScene == nullptr)
		return;

	if (PerSceneData.find(SceneID) != PerSceneData.end())
		return;

	PerSceneData[SceneID] = new FEHaloSelectionData();
	PerSceneData[SceneID]->SceneID = SceneID;

	FEEntity* MainCamera = CAMERA_SYSTEM.GetMainCameraEntity(CurrentScene);
	FECameraRenderingData* CameraData = RENDERER.GetCameraRenderingData(MainCamera);
	if (CameraData == nullptr)
		return;
	std::vector<FEPostProcess*>& PostProcesses = CameraData->PostProcessEffects;

	PerSceneData[SceneID]->HaloObjectsFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT, CameraData->SceneToTextureFB->GetWidth(), CameraData->SceneToTextureFB->GetHeight());
	PerSceneData[SceneID]->PostProcess = ENGINE.CreatePostProcess("selectionHaloEffect", CameraData->SceneToTextureFB->GetWidth() / 4, CameraData->SceneToTextureFB->GetHeight() / 4);

	FEShader* BlurShader = RESOURCE_MANAGER.GetShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/);
	PerSceneData[SceneID]->PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_OWN_TEXTURE, BlurShader));
	PerSceneData[SceneID]->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	// because input texture at first stage is full resolution, we should blur harder to get simular blur on both sides.
	PerSceneData[SceneID]->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.5f * 4.0f, "BloomSize"));
	PerSceneData[SceneID]->PostProcess->Stages.back()->InTexture.push_back(PerSceneData[SceneID]->HaloObjectsFB->GetColorAttachment());
	PerSceneData[SceneID]->PostProcess->ReplaceOutTexture(0, RESOURCE_MANAGER.CreateSameFormatTexture(CameraData->SceneToTextureFB->GetColorAttachment(), CameraData->SceneToTextureFB->GetWidth() / 4, CameraData->SceneToTextureFB->GetHeight() / 4));

	PerSceneData[SceneID]->PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BlurShader));
	PerSceneData[SceneID]->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	PerSceneData[SceneID]->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.5f, "BloomSize"));
	PerSceneData[SceneID]->PostProcess->Stages.back()->InTexture.push_back(PerSceneData[SceneID]->PostProcess->Stages[0]->OutTexture);
	PerSceneData[SceneID]->PostProcess->ReplaceOutTexture(1, RESOURCE_MANAGER.CreateSameFormatTexture(CameraData->SceneToTextureFB->GetColorAttachment(), CameraData->SceneToTextureFB->GetWidth() / 4, CameraData->SceneToTextureFB->GetHeight() / 4));

	PerSceneData[SceneID]->PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BlurShader));
	PerSceneData[SceneID]->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(1.0f, 0.0f), "FEBlurDirection"));
	PerSceneData[SceneID]->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));
	PerSceneData[SceneID]->PostProcess->Stages.back()->InTexture.push_back(PerSceneData[SceneID]->PostProcess->Stages[0]->OutTexture);
	PerSceneData[SceneID]->PostProcess->ReplaceOutTexture(2, RESOURCE_MANAGER.CreateSameFormatTexture(CameraData->SceneToTextureFB->GetColorAttachment(), CameraData->SceneToTextureFB->GetWidth() / 4, CameraData->SceneToTextureFB->GetHeight() / 4));

	PerSceneData[SceneID]->PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BlurShader));
	PerSceneData[SceneID]->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(glm::vec2(0.0f, 1.0f), "FEBlurDirection"));
	PerSceneData[SceneID]->PostProcess->Stages.back()->StageSpecificUniforms.push_back(FEShaderParam(1.0f, "BloomSize"));
	PerSceneData[SceneID]->PostProcess->Stages.back()->InTexture.push_back(PerSceneData[SceneID]->PostProcess->Stages[0]->OutTexture);
	PerSceneData[SceneID]->PostProcess->ReplaceOutTexture(3, RESOURCE_MANAGER.CreateSameFormatTexture(CameraData->SceneToTextureFB->GetColorAttachment(), CameraData->SceneToTextureFB->GetWidth() / 4, CameraData->SceneToTextureFB->GetHeight() / 4));

	PerSceneData[SceneID]->PostProcess->AddStage(new FEPostProcessStage(FE_POST_PROCESS_OWN_TEXTURE, HaloFinalShader));

	PerSceneData[SceneID]->PostProcess->Stages.back()->InTexture.push_back(PostProcesses[PostProcesses.size() - 1]->Stages.back()->OutTexture);
	PerSceneData[SceneID]->PostProcess->Stages.back()->InTextureSource.push_back(FE_POST_PROCESS_OWN_TEXTURE);
	PerSceneData[SceneID]->PostProcess->Stages.back()->InTexture.push_back(PerSceneData[SceneID]->PostProcess->Stages[3]->OutTexture);
	PerSceneData[SceneID]->PostProcess->Stages.back()->InTextureSource.push_back(FE_POST_PROCESS_OWN_TEXTURE);
	PerSceneData[SceneID]->PostProcess->Stages.back()->InTexture.push_back(PerSceneData[SceneID]->PostProcess->Stages[0]->InTexture[0]);
	PerSceneData[SceneID]->PostProcess->ReplaceOutTexture(4, RESOURCE_MANAGER.CreateSameFormatTexture(CameraData->SceneToTextureFB->GetColorAttachment(), CameraData->SceneToTextureFB->GetWidth(), CameraData->SceneToTextureFB->GetHeight()));

	RENDERER.AddPostProcess(CameraData, PerSceneData[SceneID]->PostProcess, true);
}

FEHaloSelectionData* FEEditorHaloSelectionEffect::GetSceneData(const std::string& SceneID)
{
	auto FoundScene = PerSceneData.find(SceneID);
	if (FoundScene != PerSceneData.end())
	{
		return FoundScene->second;
	}

	return nullptr;
}