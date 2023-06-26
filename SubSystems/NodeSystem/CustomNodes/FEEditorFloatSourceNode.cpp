#include "FEEditorFloatSourceNode.h"
using namespace FocalEngine;

FEEditorFloatSourceNode::FEEditorFloatSourceNode(const float InitialData) : VisualNode()
{
	Type = "FEEditorFloatSourceNode";

	Data = InitialData;

	SetSize(ImVec2(220, 78));
	SetName("Float");

	TitleBackgroundColor = ImColor(31, 117, 208);
	TitleBackgroundColorHovered = ImColor(35, 145, 255);
	
	AddSocket(new NodeSocket(this, "FLOAT", "out", true));
}

void FEEditorFloatSourceNode::Draw()
{	
	VisualNode::Draw();
	ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 10.0f, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT + 13.0f));
	ImGui::SetNextItemWidth(140);
	if (ImGui::InputFloat("##data", &Data))
	{
		ParentArea->PropagateUpdateToConnectedNodes(this);
	}
	
	if (ContextMenu)
	{
		ContextMenu = false;
		ImGui::OpenPopup("##context_menu");
	}
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopup("##context_menu"))
	{
		if (ImGui::MenuItem("Remove float node"))
		{
			bShouldBeDestroyed = true;
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
}

void FEEditorFloatSourceNode::SocketEvent(NodeSocket* OwnSocket, NodeSocket* ConnectedSocket, const VISUAL_NODE_SOCKET_EVENT EventType)
{
	VisualNode::SocketEvent(OwnSocket,  ConnectedSocket, EventType);
}

float FEEditorFloatSourceNode::GetData() const
{
	return Data;
}

bool FEEditorFloatSourceNode::CanConnect(NodeSocket* OwnSocket, NodeSocket* CandidateSocket, char** MsgToUser)
{
	if (!VisualNode::CanConnect(OwnSocket, CandidateSocket, nullptr))
		return false;

	return false;
}

bool FEEditorFloatSourceNode::OpenContextMenu()
{
	ContextMenu = true;
	return true;
}

Json::Value FEEditorFloatSourceNode::GetInfoForSaving()
{
	return "";
}