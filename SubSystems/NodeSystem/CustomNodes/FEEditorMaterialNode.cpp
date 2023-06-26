#include "FEEditorMaterialNode.h"
using namespace FocalEngine;

FEEditorMaterialNode::FEEditorMaterialNode(FEMaterial* Material) : VisualNode()
{
	Type = "FEEditorMaterialNode";

	Data = Material;

	SetSize(ImVec2(380, 500));
	SetName(Data->GetName());

	TitleBackgroundColor = ImColor(200, 50, 200);
	TitleBackgroundColorHovered = ImColor(245, 50, 245);
	
	AddSocket(new NodeSocket(this, "RGBA", "albedo", false));
	AddSocket(new NodeSocket(this, "RGBA", "normal", false));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "AO", false));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "Roughness", false));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "Metalness", false));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "Displacement", false));

	AddSocket(new NodeSocket(this, "RGBA", "albedo_1", false));
	AddSocket(new NodeSocket(this, "RGBA", "normal_1", false));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "AO_1", false));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "Roughness_1", false));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "Metalness_1", false));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "Displacement_1", false));
}

void FEEditorMaterialNode::Draw()
{	
	VisualNode::Draw();

	ImVec2 CurrentPosition = ImVec2(ImGui::GetCursorScreenPos().x + 180.0f, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT + 13.0f);
	ImGui::SetCursorScreenPos(CurrentPosition);
	const float FieldWidth = 160.0f;
	const float FieldStep = 30.0f;

	bool bCompactFlag = Data->IsCompackPacking();
	ImGui::Checkbox("##Compact flag", &bCompactFlag);
	ImGui::SameLine();
	ImGui::Text("Compact packing");
	Data->SetCompackPacking(bCompactFlag);

	FEShaderParam* DebugFlag = Data->GetParameter("debugFlag");
	if (DebugFlag != nullptr)
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("Debug flag:");
		int IData = *static_cast<int*>(DebugFlag->Data);

		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::SetNextItemWidth(FieldWidth);
		ImGui::SliderInt("##Debug flag", &IData, 0, 10);
		DebugFlag->UpdateData(IData);
	}

	// ************* Normal *************
	CurrentPosition.y += FieldStep;
	ImGui::SetCursorScreenPos(CurrentPosition);
	ImGui::Text("Normal map \nintensity:");
	
	float NormalMapIntensity = Data->GetNormalMapIntensity();
	CurrentPosition.y += FieldStep * 1.5f;
	ImGui::SetCursorScreenPos(CurrentPosition);
	ImGui::SetNextItemWidth(FieldWidth);
	ImGui::DragFloat("##Normal map intensity", &NormalMapIntensity, 0.01f, 0.0f, 1.0f);
	Data->SetNormalMapIntensity(NormalMapIntensity);

	// ************* AO *************
	if (Data->GetAOMap() == nullptr)
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("AO intensity:");
		ImGui::SetNextItemWidth(FieldWidth);
		float AmbientOcclusionIntensity = Data->GetAmbientOcclusionIntensity();
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::DragFloat("##AO intensity", &AmbientOcclusionIntensity, 0.01f, 0.0f, 10.0f);
		Data->SetAmbientOcclusionIntensity(AmbientOcclusionIntensity);
	}
	else
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("AO map \nintensity:");
		ImGui::SetNextItemWidth(FieldWidth);
		float AOMapIntensity = Data->GetAmbientOcclusionMapIntensity();
		CurrentPosition.y += FieldStep * 1.5f;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::DragFloat("##AO map intensity", &AOMapIntensity, 0.01f, 0.0f, 10.0f);
		Data->SetAmbientOcclusionMapIntensity(AOMapIntensity);
	}

	// ************* Roughness *************
	if (Data->GetRoughnessMap() == nullptr)
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("Roughness:");
		ImGui::SetNextItemWidth(FieldWidth);
		float roughness = Data->GetRoughness();
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::DragFloat("##Roughness", &roughness, 0.01f, 0.0f, 1.0f);
		Data->SetRoughness(roughness);
	}
	else
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("Roughness map \nintensity:");
		ImGui::SetNextItemWidth(FieldWidth);
		float roughness = Data->GetRoughnessMapIntensity();
		CurrentPosition.y += FieldStep * 1.5f;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::DragFloat("##Roughness map intensity", &roughness, 0.01f, 0.0f, 10.0f);
		Data->SetRoughnessMapIntensity(roughness);
	}

	// ************* Metalness *************
	if (Data->GetMetalnessMap() == nullptr)
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("Metalness:");
		ImGui::SetNextItemWidth(FieldWidth);
		float metalness = Data->GetMetalness();
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::DragFloat("##Metalness", &metalness, 0.01f, 0.0f, 1.0f);
		Data->SetMetalness(metalness);
	}
	else
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("Metalness map \nintensity:");
		ImGui::SetNextItemWidth(FieldWidth);
		float metalness = Data->GetMetalnessMapIntensity();
		CurrentPosition.y += FieldStep * 1.5f;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::DragFloat("##Metalness map intensity", &metalness, 0.01f, 0.0f, 10.0f);
		Data->SetMetalnessMapIntensity(metalness);
	}

	CurrentPosition.y += FieldStep;
	ImGui::SetCursorScreenPos(CurrentPosition);
	ImGui::Text("Tiling :");
	ImGui::SetNextItemWidth(FieldWidth);
	float tiling = Data->GetTiling();
	CurrentPosition.y += FieldStep;
	ImGui::SetCursorScreenPos(CurrentPosition);
	ImGui::DragFloat("##Tiling", &tiling, 0.01f, 0.0f, 64.0f);
	Data->SetTiling(tiling);
	
	if (bContextMenu)
	{
		bContextMenu = false;
		ImGui::OpenPopup("##context_menu");
	}
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	//if (ImGui::BeginPopup("##context_menu"))
	//{
	//	/*if (ImGui::MenuItem("Remove float node"))
	//	{
	//		bShouldBeDestroyed = true;
	//	}*/

	//	ImGui::EndPopup();
	//}

	ImGui::PopStyleVar();
}

void FEEditorMaterialNode::SocketEvent(NodeSocket* OwnSocket, NodeSocket* ConnectedSocket, const VISUAL_NODE_SOCKET_EVENT EventType)
{
	VisualNode::SocketEvent(OwnSocket,  ConnectedSocket, EventType);

	if (EventType == VISUAL_NODE_SOCKET_DESTRUCTION)
		return;

	FETexture* Texture = reinterpret_cast<FEEditorTextureSourceNode*>(ConnectedSocket->GetParent())->GetTexture();
	if (EventType == VISUAL_NODE_SOCKET_DISCONNECTED)
		Texture = nullptr;

	size_t SocketIndex = 0; // "r"
	if (ConnectedSocket->GetName() == "g")
	{
		SocketIndex = 1;
	}
	else if (ConnectedSocket->GetName() == "b")
	{
		SocketIndex = 2;
	}
	else if (ConnectedSocket->GetName() == "a")
	{
		SocketIndex = 3;
	}

	if (OwnSocket->GetName() == "albedo")
	{
		Data->SetAlbedoMap(Texture, 0);
	} 
	else if (OwnSocket->GetName() == "albedo_1")
	{
		Data->SetAlbedoMap(Texture, 1);
	}

	if (OwnSocket->GetName() == "normal")
	{
		Data->SetNormalMap(Texture, 0);
	}
	else if (OwnSocket->GetName() == "normal_1")
	{
		Data->SetNormalMap(Texture, 1);
	}

	if (OwnSocket->GetName() == "AO")
	{
		Data->SetAOMap(Texture, static_cast<int>(SocketIndex), 0);
	}
	else if (OwnSocket->GetName() == "AO_1")
	{
		Data->SetAOMap(Texture, static_cast<int>(SocketIndex), 1);
	}

	if (OwnSocket->GetName() == "Roughness")
	{
		Data->SetRoughnessMap(Texture, static_cast<int>(SocketIndex), 0);
	}
	else if (OwnSocket->GetName() == "Roughness_1")
	{
		Data->SetRoughnessMap(Texture, static_cast<int>(SocketIndex), 1);
	}

	if (OwnSocket->GetName() == "Metalness")
	{
		Data->SetMetalnessMap(Texture, static_cast<int>(SocketIndex), 0);
	}
	else if (OwnSocket->GetName() == "Metalness_1")
	{
		Data->SetMetalnessMap(Texture, static_cast<int>(SocketIndex), 1);
	}

	if (OwnSocket->GetName() == "Displacement")
	{
		Data->SetDisplacementMap(Texture, static_cast<int>(SocketIndex), 0);
	}
	else if (OwnSocket->GetName() == "Displacement_1")
	{
		Data->SetDisplacementMap(Texture, static_cast<int>(SocketIndex), 1);
	}
}

FEMaterial* FEEditorMaterialNode::GetData() const
{
	return Data;
}

bool FEEditorMaterialNode::CanConnect(NodeSocket* OwnSocket, NodeSocket* CandidateSocket, char** MsgToUser)
{
	if (!VisualNode::CanConnect(OwnSocket, CandidateSocket, nullptr))
		return false;

	// For now it is unsupported type.
	if (CandidateSocket->GetType() == "FLOAT")
	{
		*MsgToUser = IncompatibleTypesMsg;
		return false;
	}

	if ((OwnSocket->GetName() == "albedo" || OwnSocket->GetName() == "albedo_1" || OwnSocket->GetName() == "normal" || OwnSocket->GetName() == "normal_1") && (CandidateSocket->GetType() != "RGBA"))
	{
		*MsgToUser = IncompatibleTypesMsg;
		return false;
	}

	if ((OwnSocket->GetName() == "AO" || OwnSocket->GetName() == "AO_1" ||
		OwnSocket->GetName() == "Roughness" || OwnSocket->GetName() == "Roughness_1" ||
		OwnSocket->GetName() == "Metalness" || OwnSocket->GetName() == "Metalness_1" ||
		OwnSocket->GetName() == "Displacement" || OwnSocket->GetName() == "Displacement_1") && (CandidateSocket->GetType() != "COLOR_CHANNEL" && CandidateSocket->GetType() != "FLOAT"))
	{
		*MsgToUser = IncompatibleTypesMsg;
		return false;
	}

	if (!OwnSocket->GetConnections().empty())
	{
		*MsgToUser = TooManyConnectionsMsg;
		return false;
	}

	return true;
}

bool FEEditorMaterialNode::OpenContextMenu()
{
	bContextMenu = true;
	return true;
}

Json::Value FEEditorMaterialNode::GetInfoForSaving()
{
	return "";
}