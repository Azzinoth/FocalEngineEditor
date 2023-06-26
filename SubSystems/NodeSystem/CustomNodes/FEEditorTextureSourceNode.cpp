#include "FEEditorTextureSourceNode.h"
using namespace FocalEngine;

FEEditorTextureSourceNode::FEEditorTextureSourceNode(FETexture* Texture) : VisualNode()
{
	Type = "FEEditorTextureSourceNode";
	
	this->Texture = Texture;
	if (Texture == nullptr)
		this->Texture = RESOURCE_MANAGER.NoTexture;

	SetSize(ImVec2(230, 180));
	SetName(Texture->GetName());

	if (Texture->GetInternalFormat() == GL_RED)
	{
		AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "r", true));
	}
	else
	{
		AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "r", true));
		AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "g", true));
		AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "b", true));
		AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "a", true));

		AddSocket(new NodeSocket(this, "RGB", "rgb", true));
		AddSocket(new NodeSocket(this, "RGBA", "rgba", true));
	}
}

void FEEditorTextureSourceNode::Draw()
{
	VisualNode::Draw();
	ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 10.0f, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT + 10.0f));
	ImGui::Image((void*)(intptr_t)Texture->GetTextureID(), ImVec2(128, 128), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

	if (bContextMenu)
	{
		bContextMenu = false;
		ImGui::OpenPopup("##context_menu");
	}
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopup("##context_menu"))
	{
		if (ImGui::MenuItem("Remove texture node"))
		{
			bShouldBeDestroyed = true;
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
}

void FEEditorTextureSourceNode::SocketEvent(NodeSocket* OwnSocket, NodeSocket* ConnectedSocket, const VISUAL_NODE_SOCKET_EVENT EventType)
{
	VisualNode::SocketEvent(OwnSocket,  ConnectedSocket, EventType);
}

FETexture* FEEditorTextureSourceNode::GetTexture() const
{
	return Texture;
}

bool FEEditorTextureSourceNode::CanConnect(NodeSocket* OwnSocket, NodeSocket* CandidateSocket, char** MsgToUser)
{
	if (!VisualNode::CanConnect(OwnSocket, CandidateSocket, nullptr))
		return false;

	return false;
}

bool FEEditorTextureSourceNode::OpenContextMenu()
{
	bContextMenu = true;
	return true;
}

Json::Value FEEditorTextureSourceNode::GetInfoForSaving()
{
	return "";
}