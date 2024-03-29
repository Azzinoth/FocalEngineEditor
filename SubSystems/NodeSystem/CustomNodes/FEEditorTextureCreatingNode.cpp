#include "FEEditorStandardNodes.h"
using namespace FocalEngine;
using namespace VisNodeSys;

FEEditorTextureCreatingNode::FEEditorTextureCreatingNode() : Node()
{
	Type = "FEEditorTextureCreatingNode";

	this->ResultTexture = RESOURCE_MANAGER.NoTexture;

	SetSize(ImVec2(280, 180));
	SetName(ResultTexture->GetName());

	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "r", false));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "g", false));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "b", false));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "a", false));
	AddSocket(new NodeSocket(this, "RGB", "rgb", false));
	AddSocket(new NodeSocket(this, "RGBA", "rgba", false));

	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "r", true));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "g", true));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "b", true));
	AddSocket(new NodeSocket(this, "COLOR_CHANNEL", "a", true));
	AddSocket(new NodeSocket(this, "RGB", "rgb", true));
	AddSocket(new NodeSocket(this, "RGBA", "rgba", true));
}

void FEEditorTextureCreatingNode::Draw()
{
	Node::Draw();

	float Zoom = ParentArea->GetZoomFactor();

	ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 75.0f * Zoom, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT * Zoom + 10.0f * Zoom));
	ImGui::Image((void*)static_cast<intptr_t>(ResultTexture->GetTextureID()), ImVec2(128, 128) * Zoom, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
}

void FEEditorTextureCreatingNode::PutDataFromColorChannelInArray(NodeSocket* SourceSocket, unsigned char* DataArray, const size_t TextureDataLenght, const size_t ToWhatChannel)
{
	const FEEditorTextureSourceNode* SourceNode = reinterpret_cast<FEEditorTextureSourceNode*>(SourceSocket->GetConnectedSockets()[0]->GetParent());
	FETexture* SourceTexture = SourceNode->GetTexture();
	if (SourceTexture->GetWidth() == SourceNode->GetTexture()->GetWidth() &&
		SourceTexture->GetHeight() == SourceNode->GetTexture()->GetHeight())
	{
		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, SourceTexture->GetTextureID()));
		unsigned char* TextureData = new unsigned char[TextureDataLenght];
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, TextureData));

		for (size_t i = 0; i < TextureDataLenght; i += 4)
		{
			DataArray[i] = TextureData[i + ToWhatChannel];
			DataArray[i + 1] = TextureData[i + ToWhatChannel];
			DataArray[i + 2] = TextureData[i + ToWhatChannel];
			DataArray[i + 3] = 255;
		}

		delete[] TextureData;
	}
}

unsigned char* FEEditorTextureCreatingNode::GetInputColorChannelData(const size_t Channel) const
{
	// Check if we have source on this channel
	if (Input.size() > Channel && !Input[Channel]->GetConnectedSockets().empty())
	{
		if (Input[Channel]->GetConnectedSockets()[0]->GetType() == "FLOAT")
		{
			const FEEditorFloatSourceNode* SourceNode = reinterpret_cast<FEEditorFloatSourceNode*>(Input[Channel]->GetConnectedSockets()[0]->GetParent());
			float data = SourceNode->GetData();
			if (data < 0.0f)
				data = 0.0f;

			data *= 255.0f;

			const size_t DataLenght = ResultTexture->GetWidth() * ResultTexture->GetHeight();
			unsigned char* Result = new unsigned char[DataLenght];
			for (size_t i = 0; i < DataLenght; i++)
			{
				Result[i] = static_cast<unsigned char>(data);
			}
			
			return Result;
		}
		else if (Input[Channel]->GetConnectedSockets()[0]->GetType() == "COLOR_CHANNEL")
		{
			const FEEditorTextureSourceNode* SourceNode = reinterpret_cast<FEEditorTextureSourceNode*>(Input[Channel]->GetConnectedSockets()[0]->GetParent());
			FETexture* SourceTexture = SourceNode->GetTexture();

			size_t SourceChannel = 0; // "r"
			if (Input[Channel]->GetConnectedSockets()[0]->GetName() == "g")
			{
				SourceChannel = 1;
			}
			else if (Input[Channel]->GetConnectedSockets()[0]->GetName() == "b")
			{
				SourceChannel = 2;
			}
			else if (Input[Channel]->GetConnectedSockets()[0]->GetName() == "a")
			{
				SourceChannel = 3;
			}

			const size_t TextureDataLenght = SourceTexture->GetWidth() * SourceTexture->GetHeight() * 4;
			FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, SourceTexture->GetTextureID()));
			unsigned char* TextureData = new unsigned char[TextureDataLenght];
			FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, TextureData));

			unsigned char* Result = new unsigned char[SourceTexture->GetWidth() * SourceTexture->GetHeight()];
			int index = 0;
			for (size_t i = 0; i < TextureDataLenght; i += 4)
			{
				Result[index++] = TextureData[i + SourceChannel];
			}

			delete[] TextureData;
			return Result;
		}
	}
	else if ((!Input[4]->GetConnectedSockets().empty() && Input[4]->GetConnectedSockets()[0]->GetType() == "RGB" && Channel < 3) ||
			 (!Input[5]->GetConnectedSockets().empty() && Input[5]->GetConnectedSockets()[0]->GetType() == "RGBA"))
	{
		const FEEditorTextureSourceNode* SourceNode = 
			reinterpret_cast<FEEditorTextureSourceNode*>(
				!Input[4]->GetConnectedSockets().empty() ? Input[4]->GetConnectedSockets()[0]->GetParent() : Input[5]->GetConnectedSockets()[0]->GetParent()
				);

		FETexture* SourceTexture = SourceNode->GetTexture();

		const size_t TextureDataLenght = SourceTexture->GetWidth() * SourceTexture->GetHeight() * 4;
		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, SourceTexture->GetTextureID()));
		unsigned char* TextureData = new unsigned char[TextureDataLenght];
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, TextureData));

		unsigned char* Result = new unsigned char[SourceTexture->GetWidth() * SourceTexture->GetHeight()];
		int index = 0;
		for (size_t i = 0; i < TextureDataLenght; i += 4)
		{
			Result[index++] = TextureData[i + Channel];
		}

		delete[] TextureData;
		return Result;
	}
	else
	{
		return nullptr;
	}

	return nullptr;
}

void FEEditorTextureCreatingNode::SocketEvent(NodeSocket* OwnSocket, NodeSocket* ConnectedSocket, const NODE_SOCKET_EVENT EventType)
{
	Node::SocketEvent(OwnSocket, ConnectedSocket, EventType);

	if (EventType == DESTRUCTION)
		return;

	size_t TextureWidth = 0;
	size_t TextureHeight = 0;

	if (ConnectedSocket->GetType() == "FLOAT")
	{
		if (ResultTexture == RESOURCE_MANAGER.NoTexture)
			return;

		TextureWidth = ResultTexture->GetWidth();
		TextureHeight = ResultTexture->GetHeight();
	}
	else if (ConnectedSocket->GetType() == "COLOR_CHANNEL" || ConnectedSocket->GetType() == "RGB" || ConnectedSocket->GetType() == "RGBA")
	{
		const FEEditorTextureSourceNode* SourceNode = reinterpret_cast<FEEditorTextureSourceNode*>(ConnectedSocket->GetParent());
		TextureWidth = SourceNode->GetTexture()->GetWidth();
		TextureHeight = SourceNode->GetTexture()->GetHeight();
	}

	const size_t TextureDataLenght = TextureWidth * TextureHeight * 4;
	unsigned char* ResultPixels = new unsigned char[TextureDataLenght];

	const unsigned char* RedChannel = GetInputColorChannelData(0);
	const unsigned char* GreenChannel = GetInputColorChannelData(1);
	const unsigned char* BlueChannel = GetInputColorChannelData(2);
	const unsigned char* AlphaChannel = GetInputColorChannelData(3);

	if (RedChannel == nullptr && GreenChannel == nullptr && BlueChannel == nullptr && AlphaChannel == nullptr)
	{
		if (ResultTexture != RESOURCE_MANAGER.NoTexture)
			RESOURCE_MANAGER.DeleteFETexture(ResultTexture);

		delete[] ResultPixels;
		delete[] RedChannel;
		delete[] GreenChannel;
		delete[] BlueChannel;
		delete[] AlphaChannel;

		ResultTexture = RESOURCE_MANAGER.NoTexture;
		return;
	}

	int index = 0;
	for (size_t i = 0; i < TextureDataLenght; i += 4)
	{
		ResultPixels[i] = RedChannel == nullptr ? 0 : RedChannel[index];
		ResultPixels[i + 1] = GreenChannel == nullptr ? 0 : GreenChannel[index];
		ResultPixels[i + 2] = BlueChannel == nullptr ? 0 : BlueChannel[index];
		ResultPixels[i + 3] = AlphaChannel == nullptr ? 255 : AlphaChannel[index];
		index++;
	}

	ResultTexture = RESOURCE_MANAGER.RawDataToFETexture(ResultPixels, static_cast<int>(TextureWidth), static_cast<int>(TextureHeight), -1);

	delete[] ResultPixels;
	delete[] RedChannel;
	delete[] GreenChannel;
	delete[] BlueChannel;
	delete[] AlphaChannel;
}

bool FEEditorTextureCreatingNode::CanConnect(NodeSocket* OwnSocket, NodeSocket* CandidateSocket, char** MsgToUser)
{
	if (!Node::CanConnect(OwnSocket, CandidateSocket, nullptr))
		return false;

	// We reject if sockets have incompatible types.
	bool bCorrectType = true;
	if (CandidateSocket->GetType() != "COLOR_CHANNEL" && CandidateSocket->GetType() != "RGB" && CandidateSocket->GetType() != "RGBA")
		bCorrectType = false;

	if ((CandidateSocket->GetType() == "COLOR_CHANNEL" && OwnSocket->GetType() != "COLOR_CHANNEL") ||
		(CandidateSocket->GetType() == "RGB" && OwnSocket->GetType() != "RGB") ||
		(CandidateSocket->GetType() == "RGBA" && OwnSocket->GetType() != "RGBA"))
		bCorrectType = false;

	if (!bCorrectType)
	{
		if (MsgToUser != nullptr)
			*MsgToUser = IncompatibleTypesMsg;
		return false;
	}

	// ***************************** tooManyConnectionOfThisTypeMsg *****************************
	if ((!Input[0]->GetConnectedSockets().empty() && !Input[1]->GetConnectedSockets().empty() && !Input[2]->GetConnectedSockets().empty()) &&
		(CandidateSocket->GetType() == "COLOR_CHANNEL"))
	{
		if (MsgToUser != nullptr)
			*MsgToUser = TooManyConnectionOfThisTypeMsg;
		return false;
	}

	if (!Input[4]->GetConnectedSockets().empty() && ("RGB" || "RGBA"))
	{
		if (OwnSocket->GetName() == "a" && (CandidateSocket->GetType() == "FLOAT" || CandidateSocket->GetType() == "COLOR_CHANNEL"))
		{

		}
		else
		{
			if (MsgToUser != nullptr)
				*MsgToUser = TooManyConnectionOfThisTypeMsg;
			return false;
		}
	}

	if ((!Input[5]->GetConnectedSockets().empty()) &&
		(CandidateSocket->GetType() == "FLOAT" || 
		CandidateSocket->GetType() == "RGB" || 
		CandidateSocket->GetType() == "RGBA"))
	{
		if (MsgToUser != nullptr)
			*MsgToUser = TooManyConnectionOfThisTypeMsg;
		return false;
	}

	if ((!Input[0]->GetConnectedSockets().empty() || !Input[1]->GetConnectedSockets().empty() || !Input[2]->GetConnectedSockets().empty() || !Input[3]->GetConnectedSockets().empty()) &&
		(CandidateSocket->GetType() == "RGBA"))
	{
		if (MsgToUser != nullptr)
			*MsgToUser = TooManyConnectionOfThisTypeMsg;
		return false;
	}

	if ((!Input[0]->GetConnectedSockets().empty() || !Input[1]->GetConnectedSockets().empty() || !Input[2]->GetConnectedSockets().empty()) &&
		(CandidateSocket->GetType() == "RGB"))
	{
		if (MsgToUser != nullptr)
			*MsgToUser = TooManyConnectionOfThisTypeMsg;
		return false;
	}
	// ***************************** tooManyConnectionOfThisTypeMsg END *****************************

	if (CandidateSocket->GetType() == "FLOAT" &&
		ResultTexture == RESOURCE_MANAGER.NoTexture)
	{
		if (MsgToUser != nullptr)
			*MsgToUser = CantInferResolutionMsg;
		return false;
	}

	// In current node own sockets can't be connected.
	if (CandidateSocket->GetParent() == this)
	{
		if (MsgToUser != nullptr)
			*MsgToUser = IncompatibleTypesMsg;
		return false;
	}

	// If we have connection on this input we should reject
	if (!OwnSocket->GetConnectedSockets().empty())
	{
		if (MsgToUser != nullptr)
			*MsgToUser = TooManyConnectionsMsg;
		return false;
	}

	// We should check do we already have some texture created.
	// If not we can accept any resolution.
	if (ResultTexture == RESOURCE_MANAGER.NoTexture)
		return true;

	// But if we already created texture we will check that candidate node have texture with same resolution.
	if (CandidateSocket->GetType() != "FLOAT")
	{
		const FEEditorTextureSourceNode* SourceNode = reinterpret_cast<FEEditorTextureSourceNode*>(CandidateSocket->GetParent());
		if (SourceNode->GetTexture()->GetWidth() == ResultTexture->GetWidth() &&
			SourceNode->GetTexture()->GetHeight() == ResultTexture->GetHeight())
		{
			return true;
		}
		else
		{
			if (MsgToUser != nullptr)
				*MsgToUser = IncompatibleResolutionMsg;
			return false;
		}
	}
	else
	{
		return true;
	}

	return false;
}

FETexture* FEEditorTextureCreatingNode::GetTexture() const
{
	return ResultTexture;
}

Json::Value FEEditorTextureCreatingNode::GetInfoForSaving()
{
	return "";
}