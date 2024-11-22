#include "FEEditorTextureSourceNode.h"
using namespace FocalEngine;
using namespace VisNodeSys;

bool FEEditorTextureSourceNode::IsRegistered = []()
{
	NODE_FACTORY.RegisterNodeType("FEEditorTextureSourceNode",
		[]() -> Node* {
			return new FEEditorTextureSourceNode(nullptr);
		},

		[](const Node& CurrentNode) -> Node* {
			const FEEditorTextureSourceNode& NodeToCopy = static_cast<const FEEditorTextureSourceNode&>(CurrentNode);
			return new FEEditorTextureSourceNode(NodeToCopy);
		}
	);

	return true;
}();

FEEditorTextureSourceNode::FEEditorTextureSourceNode(FETexture* Texture) : Node()
{
	Type = "FEEditorTextureSourceNode";
	
	this->Texture = Texture;
	if (this->Texture == nullptr)
		this->Texture = RESOURCE_MANAGER.NoTexture;

	SetSize(ImVec2(230, 180));
	SetName(this->Texture->GetName());

	if (this->Texture->GetInternalFormat() == GL_RED)
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
	Node::Draw();

	float Zoom = ParentArea->GetZoomFactor();

	ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 10.0f * Zoom, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT * Zoom + 10.0f * Zoom));
	ImGui::Image((void*)(intptr_t)Texture->GetTextureID(), ImVec2(128, 128) * Zoom, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

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

void FEEditorTextureSourceNode::SocketEvent(NodeSocket* OwnSocket, NodeSocket* ConnectedSocket, const NODE_SOCKET_EVENT EventType)
{
	Node::SocketEvent(OwnSocket,  ConnectedSocket, EventType);
}

FETexture* FEEditorTextureSourceNode::GetTexture() const
{
	return Texture;
}

bool FEEditorTextureSourceNode::CanConnect(NodeSocket* OwnSocket, NodeSocket* CandidateSocket, char** MsgToUser)
{
	if (!Node::CanConnect(OwnSocket, CandidateSocket, nullptr))
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