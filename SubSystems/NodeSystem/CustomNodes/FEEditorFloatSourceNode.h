#pragma once

#include "../VisualNodeSystem/VisualNodeSystem.h"

class FEEditorFloatSourceNode : public VisNodeSys::Node
{
	float Data = 0.0f;

	bool ContextMenu = false;
	bool OpenContextMenu();

	bool CanConnect(VisNodeSys::NodeSocket* OwnSocket, VisNodeSys::NodeSocket* CandidateSocket, char** MsgToUser);
	void SocketEvent(VisNodeSys::NodeSocket* OwnSocket, VisNodeSys::NodeSocket* ConnectedSocket, VisNodeSys::NODE_SOCKET_EVENT EventType);

	void MouseClick(int MouseButton);

	Json::Value GetInfoForSaving();
public:
	FEEditorFloatSourceNode(float InitialData = 0.0f);

	void Draw();
	float GetData() const;
};