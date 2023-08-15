#pragma once

#include "../VisualNodeSystem/VisualNodeSystem.h"
#include "../FEngine.h"
using namespace FocalEngine;

class FEEditorTextureSourceNode : public VisNodeSys::Node
{
	friend class NodeFactory;
	static bool isRegistered;

	FETexture* Texture = nullptr;

	bool bContextMenu = false;
	bool OpenContextMenu();
	
	bool CanConnect(VisNodeSys::NodeSocket* OwnSocket, VisNodeSys::NodeSocket* CandidateSocket, char** MsgToUser);
	void SocketEvent(VisNodeSys::NodeSocket* OwnSocket, VisNodeSys::NodeSocket* ConnectedSocket, VisNodeSys::NODE_SOCKET_EVENT EventType);

	void MouseClick(int MouseButton);

	Json::Value GetInfoForSaving();
public:
	FEEditorTextureSourceNode(FETexture* Texture);

	void Draw();
	FETexture* GetTexture() const;
};