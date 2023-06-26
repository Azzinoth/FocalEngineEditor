#pragma once

#include "../VisualNodeSystem/VisualNodeSystem.h"

class FEEditorTextureCreatingNode : public VisualNode
{
	FETexture* ResultTexture = nullptr;
	char* IncompatibleTypesMsg = "Incompatible socket types.";
	char* TooManyConnectionsMsg = "Too many connections.";
	char* IncompatibleResolutionMsg = "Incompatible texture resolution.";
	char* CantInferResolutionMsg = "Can't infer texture resolution.";
	char* TooManyConnectionOfThisTypeMsg = "Too many connections of this type.";

	void PutDataFromColorChannelInArray(NodeSocket* SourceSocket, unsigned char* DataArray, size_t TextureDataLenght, size_t ToWhatChannel);
	unsigned char* GetInputColorChannelData(size_t Channel) const;

	bool CanConnect(NodeSocket* OwnSocket, NodeSocket* CandidateSocket, char** MsgToUser);
	void SocketEvent(NodeSocket* OwnSocket, NodeSocket* ConnectedSocket, VISUAL_NODE_SOCKET_EVENT EventType);

	Json::Value GetInfoForSaving();
public:
	FEEditorTextureCreatingNode();

	void Draw();
	FETexture* GetTexture() const;
};