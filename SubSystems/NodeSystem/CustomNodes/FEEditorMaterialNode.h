#pragma once

#include "../VisualNodeSystem/VisualNodeSystem.h"
#include "FEEditorTextureSourceNode.h"

class FEEditorMaterialNode: public VisualNode
{
	char* IncompatibleTypesMsg = "Incompatible socket types.";
	char* TooManyConnectionsMsg = "Too many connections.";
	char* IncompatibleResolutionMsg = "Incompatible texture resolution.";
	char* CantInferResolutionMsg = "Can't infer texture resolution.";
	char* TooManyConnectionOfThisTypeMsg = "Too many connections of this type.";

	FEMaterial* Data = nullptr;
	
	bool bContextMenu = false;
	bool OpenContextMenu();

	bool CanConnect(NodeSocket* OwnSocket, NodeSocket* CandidateSocket, char** MsgToUser);
	void SocketEvent(NodeSocket* OwnSocket, NodeSocket* ConnectedSocket, VISUAL_NODE_SOCKET_EVENT EventType);

	void MouseClick(int MouseButton);

	Json::Value GetInfoForSaving();
public:
	FEEditorMaterialNode(FEMaterial* Material);

	void Draw();
	FEMaterial* GetData() const;
};