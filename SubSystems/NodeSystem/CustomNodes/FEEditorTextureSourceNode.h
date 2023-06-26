#pragma once

#include "../VisualNodeSystem/VisualNodeSystem.h"
#include "../FEngine.h"

class FEEditorTextureSourceNode : public VisualNode
{
	FETexture* Texture = nullptr;

	bool bContextMenu = false;
	bool OpenContextMenu();
	
	bool CanConnect(NodeSocket* OwnSocket, NodeSocket* CandidateSocket, char** MsgToUser);
	void SocketEvent(NodeSocket* OwnSocket, NodeSocket* ConnectedSocket, VISUAL_NODE_SOCKET_EVENT EventType);

	void MouseClick(int MouseButton);

	Json::Value GetInfoForSaving();
public:
	FEEditorTextureSourceNode(FETexture* Texture);

	void Draw();
	FETexture* GetTexture() const;
};