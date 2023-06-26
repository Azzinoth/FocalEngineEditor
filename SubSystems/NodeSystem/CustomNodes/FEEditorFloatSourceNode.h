#pragma once

#include "../VisualNodeSystem/VisualNodeSystem.h"

class FEEditorFloatSourceNode : public VisualNode
{
	float Data = 0.0f;

	bool ContextMenu = false;
	bool OpenContextMenu();

	bool CanConnect(NodeSocket* OwnSocket, NodeSocket* CandidateSocket, char** MsgToUser);
	void SocketEvent(NodeSocket* OwnSocket, NodeSocket* ConnectedSocket, VISUAL_NODE_SOCKET_EVENT EventType);

	void MouseClick(int MouseButton);

	Json::Value GetInfoForSaving();
public:
	FEEditorFloatSourceNode(float InitialData = 0.0f);

	void Draw();
	float GetData() const;
};