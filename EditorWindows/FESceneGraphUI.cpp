#include "FESceneGraphUI.h"

bool FESceneGraphUI::bLastNodeWasEven = true;

FESceneGraphUI::FESceneGraphUI()
{
	strcpy_s(CharFilterText, PlaceHolderTextString.c_str());
}

FESceneGraphUI::~FESceneGraphUI() {}

std::string FESceneGraphUI::GetSceneID() const
{
	return CurrentSceneID;
}

FEScene* FESceneGraphUI::GetScene() const
{
	return SCENE_MANAGER.GetSceneByID(CurrentSceneID);
}

FENaiveSceneGraphNode* FESceneGraphUI::GetCurrentRenderingRoot() const
{
	return RenderingRoot;
}

void FESceneGraphUI::SetNodeRenderPredicate(std::function<bool(FENaiveSceneGraphNode*)> Predicate)
{
	NodeRenderPredicate = Predicate;
}

void FESceneGraphUI::SetNodeDisplayNameProvider(std::function<std::string(FENaiveSceneGraphNode*)> Provider)
{
	NodeDisplayNameProvider = Provider;
}

void FESceneGraphUI::SetNodeChildrenVisiblePredicate(std::function<bool(FENaiveSceneGraphNode*)> Predicate)
{
	NodeChildrenVisiblePredicate = Predicate;
}

void FESceneGraphUI::SetNodeIconProvider(std::function<FETexture* (FENaiveSceneGraphNode*)> Provider)
{
	NodeIconProvider = Provider;
}

void FESceneGraphUI::SetNodeSelectionPredicate(std::function<bool(FENaiveSceneGraphNode*)> Predicate)
{
	NodeSelectionPredicate = Predicate;
}

void FESceneGraphUI::ClearAllProvidersAndPredicates()
{
	NodeRenderPredicate = nullptr;
	NodeDisplayNameProvider = nullptr;
	NodeChildrenVisiblePredicate = nullptr;
	NodeIconProvider = nullptr;
	NodeSelectionPredicate = nullptr;
}

FETexture* FESceneGraphUI::GetNodeIcon(FENaiveSceneGraphNode* Node)
{
	if (NodeIconProvider != nullptr)
		return NodeIconProvider(Node);
	
	return nullptr;
}

std::string FESceneGraphUI::GetNodeDisplayName(FENaiveSceneGraphNode* Node)
{
	FEEntity* CurrentEntity = Node->GetEntity();
	std::string DisplayedName = CurrentEntity == nullptr ? Node->GetName() : CurrentEntity->GetName();
	if (NodeDisplayNameProvider != nullptr)
		DisplayedName = NodeDisplayNameProvider(Node);

	return DisplayedName;
}

std::vector<std::string> FESceneGraphUI::GetHiddenEntityTags() const
{
	return HiddenEntityTags;
}

void FESceneGraphUI::SetHiddenEntityTags(const std::vector<std::string>& NewHiddenEntityTags)
{
	HiddenEntityTags = NewHiddenEntityTags;
}

void FESceneGraphUI::AddHiddenEntityTag(const std::string& TagToAdd)
{
	if (std::find(HiddenEntityTags.begin(), HiddenEntityTags.end(), TagToAdd) == HiddenEntityTags.end())
		HiddenEntityTags.push_back(TagToAdd);
}

void FESceneGraphUI::RemoveHiddenEntityTag(const std::string& TagToRemove)
{
	auto Iterator = std::find(HiddenEntityTags.begin(), HiddenEntityTags.end(), TagToRemove);
	if (Iterator != HiddenEntityTags.end())
		HiddenEntityTags.erase(Iterator);
}

void FESceneGraphUI::ClearHiddenEntityTags()
{
	HiddenEntityTags.clear();
}

bool FESceneGraphUI::DoesNodePassTextFilter(FENaiveSceneGraphNode* Node)
{
	if (Node == nullptr)
		return false;

	if (!bFilterEnabled)
		return true;

	if (FilterText.empty())
		return true;

	std::string UsedFilterText = FilterText;
	std::string NodeDisplayName = GetNodeDisplayName(Node);
	if (!bCaseSensitiveFiltering)
	{
		std::transform(NodeDisplayName.begin(), NodeDisplayName.end(), NodeDisplayName.begin(), ::tolower);
		std::transform(UsedFilterText.begin(), UsedFilterText.end(), UsedFilterText.begin(), ::tolower);
	}

	if (NodeDisplayName.find(UsedFilterText) != std::string::npos)
		return true;

	// Check all children recursively, if at least one of them passes the filter, then this node should be visible as well.
	for (FENaiveSceneGraphNode* Child : Node->GetChildren())
	{
		if (DoesNodePassTextFilter(Child))
			return true;
	}

	return false;
}

bool FESceneGraphUI::ShouldNodeBeVisible(FENaiveSceneGraphNode* Node)
{
	if (Node == nullptr)
		return false;

	FEEntity* CurrentEntity = Node->GetEntity();
	if (CurrentEntity != nullptr)
	{
		std::string EntityTag = CurrentEntity->GetTag();
		if (std::find(HiddenEntityTags.begin(), HiddenEntityTags.end(), EntityTag) != HiddenEntityTags.end())
			return false;

		if (!DoesNodePassTextFilter(Node))
			return false;
	}

	if (NodeRenderPredicate != nullptr)
		return NodeRenderPredicate(Node);
	
	return true;
}

bool FESceneGraphUI::AreNodeChildrenVisible(FENaiveSceneGraphNode* Node)
{
	if (Node == nullptr)
		return false;

	if (Node->GetChildren().size() == 0)
		return false;

	if (NodeChildrenVisiblePredicate != nullptr)
		return NodeChildrenVisiblePredicate(Node);
	
	return true;
}

std::string FESceneGraphUI::GetNodeInternalID(FENaiveSceneGraphNode* Node)
{
	FEEntity* CurrentEntity = Node->GetEntity();
	return CurrentEntity != nullptr ? CurrentEntity->GetObjectID() : RenderingRoot != nullptr ? RenderingRoot->GetObjectID() : "";
}

bool FESceneGraphUI::IsNodeExpanded(FENaiveSceneGraphNode* Node)
{
	std::string NodeID = GetNodeInternalID(Node);
	bool bResult = false;
	if (NodeState.find(NodeID) == NodeState.end())
	{
		NodeState[NodeID].bExpanded = false;
	}
	else
	{
		bResult = NodeState[NodeID].bExpanded;
	}

	return bResult;
}

void FESceneGraphUI::SetNodeExpanded(FENaiveSceneGraphNode* Node, bool bExpanded)
{
	NodeState[GetNodeInternalID(Node)].bExpanded = bExpanded;
}

bool FESceneGraphUI::IsNodeSelected(FENaiveSceneGraphNode* Node)
{
	bool bResult = false;
	std::string NodeID = GetNodeInternalID(Node);
	if (NodeSelectionPredicate != nullptr)
	{
		bool bResult = NodeSelectionPredicate(Node);
		NodeState[NodeID].bSelected = bResult;

		return bResult;
	}
	
	if (NodeState.find(NodeID) == NodeState.end())
	{
		NodeState[NodeID].bSelected = false;
	}
	else
	{
		bResult = NodeState[NodeID].bSelected;
	}

	return bResult;
}

void FESceneGraphUI::SetNodeSelected(FENaiveSceneGraphNode* Node, bool bSelected)
{
	if (bAllowMultipleNodeSelection)
	{
		NodeState[GetNodeInternalID(Node)].bSelected = bSelected;
	}
	else if (!bAllowMultipleNodeSelection && bSelected)
	{
		for (auto& NodeStatePair : NodeState)
			NodeStatePair.second.bSelected = false;
	}

	NodeState[GetNodeInternalID(Node)].bSelected = bSelected;
}

std::vector<std::string> FESceneGraphUI::GetSelectedNodeIDs() const
{
	std::vector<std::string> SelectedNodeIDs;
	for (const auto& NodeStatePair : NodeState)
	{
		if (NodeStatePair.second.bSelected)
			SelectedNodeIDs.push_back(NodeStatePair.first);
	}

	return SelectedNodeIDs;
}

bool FESceneGraphUI::IsNodePartOfBranch(FENaiveSceneGraphNode* NodeToCheck, FENaiveSceneGraphNode* BranchRoot, FENaiveSceneGraphNode* BranchLeaf)
{
	if (NodeToCheck == nullptr || BranchRoot == nullptr || BranchLeaf == nullptr)
		return false;

	if (NodeToCheck == BranchRoot || NodeToCheck == BranchLeaf)
		return true;

	// Walk from BranchLeaf up to BranchRoot.
	// NodeToCheck is part of the branch only if it lies on this path.
	FENaiveSceneGraphNode* CurrentNode = BranchLeaf;
	while (CurrentNode != nullptr)
	{
		if (CurrentNode == NodeToCheck)
			return true;

		// Stop once we have reached the root of the branch.
		if (CurrentNode == BranchRoot)
			break;

		CurrentNode = CurrentNode->GetParent();
	}

	return false;
}

void FESceneGraphUI::DrawTreeConnectorLines(FENaiveSceneGraphNode* Node, float ParentBottomY)
{
	ImColor ConnectorLineColorToUse = ImColor(this->ConnectorLineColor);
	float ConnectorLineThicknessToUse = ConnectorLineThickness;
	bool bNeedToHighlightNodeBranch = false;
	std::vector<std::string> SelectedNodeIDs = GetSelectedNodeIDs();
	if (bHighlightSelectedNodeConnectorLines && !SelectedNodeIDs.empty())
	{
		FEScene* CurrentScene = GetScene();
		for (size_t i = 0; i < SelectedNodeIDs.size(); i++)
		{
			if (IsNodePartOfBranch(Node, RenderingRoot, CurrentScene->SceneGraph.GetNodeByEntityID(SelectedNodeIDs[i])))
			{
				ConnectorLineColorToUse = SelectedNodeConnectorLineColor;
				ConnectorLineThicknessToUse = SelectedConnectorLineThickness;
				bNeedToHighlightNodeBranch = true;
			}
		}
	}
	ImGui::GetWindowDrawList()->ChannelsSetCurrent(bNeedToHighlightNodeBranch ? 1 : 0);

	size_t Depth = Node->GetDepth();
	Depth -= RenderingRoot->GetDepth() + (bRenderRootItself ? 0 : 1);

	float BaseX = ImGui::GetCursorScreenPos().x;

	int HorizontalOffset = static_cast<int>((Depth - 1) * NodeHeight);
	ImVec2 VerticalStart = ImVec2(BaseX + HorizontalOffset + NodeHeight / 2.0f,
								  ParentBottomY + NodeHeight);

	ImVec2 ElbowPoint = ImVec2(VerticalStart.x,
							   ImGui::GetCursorScreenPos().y + NodeHeight / 2.0f);

	ImGui::GetWindowDrawList()->AddLine(VerticalStart, ElbowPoint, ImColor(ConnectorLineColorToUse), ConnectorLineThicknessToUse);

	bool bHasChildren = AreNodeChildrenVisible(Node);
	ImVec2 HorizontalEnd = ImVec2(ElbowPoint.x + NodeHeight / (bHasChildren ? 2.0f : 0.7f),
		                          ElbowPoint.y);

	ImGui::GetWindowDrawList()->AddLine(ElbowPoint, HorizontalEnd, ImColor(ConnectorLineColorToUse), ConnectorLineThicknessToUse);

	if (bNeedToHighlightNodeBranch)
		ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);
}

void FESceneGraphUI::DrawAppropriateTreeArrow(FENaiveSceneGraphNode* Node)
{
	float ArrowRegionWidth = FontSize;
	ImVec2 ArrowCursorPos = ImGui::GetCursorScreenPos();

	bool bNodeExpanded = IsNodeExpanded(Node);
	bool bHasChildren = AreNodeChildrenVisible(Node);

	if (bHasChildren)
	{
		// Draw arrow centered within the reserved region
		float ArrowHalfSize = FontSize * 0.25f;
		float Thickness = FontSize * TreeArrowsThicknessCoefficient;
		ImU32 Color = ImGui::GetColorU32(ImGuiCol_Text);
		float CenterX = ArrowCursorPos.x + ArrowRegionWidth * 0.5f;
		float CenterY = ArrowCursorPos.y + NodeHeight * 0.5f;

		if (bNodeExpanded)
		{
			// Down-pointing arrow (collapse)
			ImGui::GetWindowDrawList()->AddLine(ImVec2(CenterX - ArrowHalfSize, CenterY - ArrowHalfSize * 0.3f),
												ImVec2(CenterX, CenterY + ArrowHalfSize * LineJoinOverlapFactor),
												Color, Thickness);

			ImGui::GetWindowDrawList()->AddLine(ImVec2(CenterX, CenterY + ArrowHalfSize * 0.6f),
												ImVec2(CenterX + ArrowHalfSize, CenterY - ArrowHalfSize * 0.3f),
												Color, Thickness);
		}
		else
		{
			// Right-pointing arrow (expand)
			ImGui::GetWindowDrawList()->AddLine(ImVec2(CenterX - ArrowHalfSize * 0.3f, CenterY - ArrowHalfSize),
												ImVec2(CenterX + ArrowHalfSize * LineJoinOverlapFactor, CenterY),
												Color, Thickness);

			ImGui::GetWindowDrawList()->AddLine(ImVec2(CenterX + ArrowHalfSize * 0.6f, CenterY),
												ImVec2(CenterX - ArrowHalfSize * 0.3f, CenterY + ArrowHalfSize),
												Color, Thickness);
		}

		// Occupy the space in ImGui layout.
		ImGui::InvisibleButton(("##Arrow" + GetNodeInternalID(Node)).c_str(), ImVec2(ArrowRegionWidth, NodeHeight));
		if (ImGui::IsItemClicked())
			SetNodeExpanded(Node, !bNodeExpanded);
	}
	else
	{
		ImGui::Dummy(ImVec2(ArrowRegionWidth, NodeHeight));
	}

	ImGui::SameLine();
}

void FESceneGraphUI::CheckInputs(FENaiveSceneGraphNode* Node)
{
	if (NodeIDBeingRenamed == Node->GetObjectID())
		return;

	if (ImGui::IsItemHovered())
	{
		HoveredNodeID = Node->GetObjectID();

		for (auto& Callback : OnNodeHoveredCallbacks)
			Callback(Node);

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			SetNodeSelected(Node, !IsNodeSelected(Node));

			for (auto& Callback : OnNodeClickedCallbacks)
				Callback(Node, ImGuiMouseButton_Left);
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			for (auto& Callback : OnNodeClickedCallbacks)
				Callback(Node, ImGuiMouseButton_Right);
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Middle))
		{
			for (auto& Callback : OnNodeClickedCallbacks)
				Callback(Node, ImGuiMouseButton_Middle);
		}

		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			for (auto& Callback : OnNodeDoubleClickedCallbacks)
				Callback(Node, ImGuiMouseButton_Left);
		}

		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right))
		{
			for (auto& Callback : OnNodeDoubleClickedCallbacks)
				Callback(Node, ImGuiMouseButton_Right);
		}

		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Middle))
		{
			for (auto& Callback : OnNodeDoubleClickedCallbacks)
				Callback(Node, ImGuiMouseButton_Middle);
		}
	}
}

void FESceneGraphUI::AddOnNodeClickedCallback(std::function<void(FENaiveSceneGraphNode*, ImGuiMouseButton_)> Callback)
{
	for (size_t i = 0; i < OnNodeClickedCallbacks.size(); i++)
	{
		if (OnNodeClickedCallbacks[i].target_type() == Callback.target_type())
			return;
	}

	OnNodeClickedCallbacks.push_back(Callback);
}

void FESceneGraphUI::ClearOnNodeClickedCallbacks()
{
	OnNodeClickedCallbacks.clear();
}

void FESceneGraphUI::AddOnNodeDoubleClickedCallback(std::function<void(FENaiveSceneGraphNode*, ImGuiMouseButton_)> Callback)
{
	for (size_t i = 0; i < OnNodeDoubleClickedCallbacks.size(); i++)
	{
		if (OnNodeDoubleClickedCallbacks[i].target_type() == Callback.target_type())
			return;
	}

	OnNodeDoubleClickedCallbacks.push_back(Callback);
}

void FESceneGraphUI::ClearOnNodeDoubleClickedCallbacks()
{
	OnNodeDoubleClickedCallbacks.clear();
}

void FESceneGraphUI::ClearAllInputCallbacks()
{
	ClearOnNodeHoveredCallbacks();
	ClearOnNodeClickedCallbacks();
	ClearOnNodeDoubleClickedCallbacks();
}

void FESceneGraphUI::SetContextMenuRenderingFunction(std::function<void(FENaiveSceneGraphNode*)> Function)
{
	ContextMenuRenderingFunction = Function;
}

void FESceneGraphUI::ClearContextMenuRenderingFunction()
{
	ContextMenuRenderingFunction = nullptr;
}

void FESceneGraphUI::ClearAllCallbacks()
{
	ClearAllProvidersAndPredicates();
	ClearAllInputCallbacks();

	BeforeNodeRenderCallbacks.clear();
	AfterNodeRenderCallbacks.clear();
	ClearContextMenuRenderingFunction();
}

void FESceneGraphUI::AddOnNodeHoveredCallback(std::function<void(FENaiveSceneGraphNode*)> Callback)
{
	for (size_t i = 0; i < OnNodeHoveredCallbacks.size(); i++)
	{
		if (OnNodeHoveredCallbacks[i].target_type() == Callback.target_type())
			return;
	}

	OnNodeHoveredCallbacks.push_back(Callback);
}

void FESceneGraphUI::ClearOnNodeHoveredCallbacks()
{
	OnNodeHoveredCallbacks.clear();
}

FESceneGraphNodeWidget* FESceneGraphUI::GetNodeWidgetByID(const std::string& WidgetID)
{
	for (size_t i = 0; i < NodeWidgets.size(); i++)
	{
		if (NodeWidgets[i].GetID() == WidgetID)
			return &NodeWidgets[i];
	}

	return nullptr;
}

std::vector<FESceneGraphNodeWidget> FESceneGraphUI::GetAllNodeWidgets() const
{
	return NodeWidgets;
}

bool FESceneGraphUI::AddNodeWidget(FESceneGraphNodeWidget& Widget)
{
	for (size_t i = 0; i < NodeWidgets.size(); i++)
	{
		if (NodeWidgets[i].GetID() == Widget.GetID())
			return false;
	}

	NodeWidgets.push_back(Widget);
	return true;
}

bool FESceneGraphUI::RemoveNodeWidget(const std::string& WidgetID)
{
	for (size_t i = 0; i < NodeWidgets.size(); i++)
	{
		if (NodeWidgets[i].GetID() == WidgetID)
		{
			NodeWidgets.erase(NodeWidgets.begin() + i);
			return true;
		}
	}

	return false;
}

void FESceneGraphUI::ClearNodeWidgets()
{
	NodeWidgets.clear();
	DebugNodeWidgets.clear();
}

bool FESceneGraphUI::ShouldRenderWidgetForNode(FENaiveSceneGraphNode* Node, FESceneGraphNodeWidget& Widget, FETexture** IconToUse)
{
	bool bVisible = Widget.bIsVisibleByDefault;
	if (Widget.IsVisiblePredicate != nullptr)
		bVisible = Widget.IsVisiblePredicate(Node);

	if (!bVisible)
		return false;

	FETexture* ResultingIcon = Widget.Icon;
	if (Widget.DynamicIconProvider != nullptr)
	{
		FETexture* ProviderIcon = Widget.DynamicIconProvider(Node);
		if (ProviderIcon != nullptr)
			ResultingIcon = ProviderIcon;
	}
	
	if (ResultingIcon == nullptr)
		return false;

	if (IconToUse != nullptr)
		*IconToUse = ResultingIcon;
	return true;
}

float FESceneGraphUI::GetNodeWidgetAreaWidth(FENaiveSceneGraphNode* Node)
{
	float IconSpacing = GetFontSize() * 0.15f;

	int VisibleWidgets = static_cast<int>(GetNodeWidgetCount(Node));
	float SpaceNeededForIconsAtEnd = IconsSize.x * WidgetIconVisualRenderingFactor * VisibleWidgets + IconSpacing * std::max(0, VisibleWidgets - 1);
	return SpaceNeededForIconsAtEnd + ImGui::GetStyle().WindowPadding.x + 6.0f;
}

size_t FESceneGraphUI::GetNodeWidgetCount(FENaiveSceneGraphNode* Node)
{
	int VisibleWidgets = 0;
	for (size_t i = 0; i < NodeWidgets.size(); i++)
	{
		FESceneGraphNodeWidget& Widget = NodeWidgets[i];

		if (!ShouldRenderWidgetForNode(Node, Widget, nullptr))
			continue;

		VisibleWidgets++;
	}

	return VisibleWidgets;
}

void FESceneGraphUI::RenderNodeWidgets(FENaiveSceneGraphNode* Node)
{
	YCursorPositionBeforeRenderingWidgets = ImGui::GetCursorPosY();
	float IconSpacing = GetFontSize() * 0.15f;

	size_t VisibleWidgetCount = GetNodeWidgetCount(Node);
	size_t WidgetIndex = 0;
	for (size_t i = 0; i < NodeWidgets.size(); i++)
	{
		FESceneGraphNodeWidget& Widget = NodeWidgets[i];

		FETexture* IconToUse = nullptr;
		if (!ShouldRenderWidgetForNode(Node, Widget, &IconToUse))
			continue;

		// We change item spacing only for the widgets that are not the first one.
		if (WidgetIndex == 1)
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(IconSpacing, 0));

		ImGui::SameLine();
		// Because we render icons a little bit smaller(using WidgetIconVisualRenderingFactor), we need to adjust the position of the icons to make them look vertically centered.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (IconsSize.y * (1.0f - WidgetIconVisualRenderingFactor)) / 2.0f);

		if (Widget.bIsInteractive)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Widget.HoveredColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, Widget.ActiveColor);

			std::string ButtonID = "##" + Widget.ID + "_" + GetNodeInternalID(Node);
			if (ImGui::ImageButton(ButtonID.c_str(), IconToUse->GetTextureID(), IconsSize * WidgetIconVisualRenderingFactor))
			{
				if (Widget.OnClickCallback != nullptr)
					Widget.OnClickCallback(Node);
			}

			ImGui::PopStyleVar();
			ImGui::PopStyleColor(3);
		}
		else
		{
			ImGui::Image(IconToUse->GetTextureID(), IconsSize * WidgetIconVisualRenderingFactor);
		}

		if (ImGui::IsItemHovered() && !Widget.TooltipText.empty())
		{
			if (CousineFont != nullptr)
				ImGui::PushFont(CousineFont, TooltipFontSize);

			ImGui::BeginTooltip();
			ImGui::TextUnformatted(Widget.TooltipText.c_str());
			ImGui::EndTooltip();

			if (CousineFont != nullptr)
				ImGui::PopFont();
		}

		WidgetIndex++;
	}

	if (WidgetIndex > 1)
		ImGui::PopStyleVar();

	// Node widgets render on the same line as the Selectable and shift the cursor Y position due to vertical centering adjustments.
	// We restore the cursor to the pre-widget Y position so the next node starts at the correct vertical offset.
	// After all nodes are rendered, we set the cursor to correct position to ensure the parent container (e.g. ListBox) accounts for the full content height.
	// If not accounted for, ImGui will assert with request to use Dummy().
	YCursorPositionAfterRenderingWidgets = ImGui::GetCursorPosY();
	if (YCursorPositionBeforeRenderingWidgets != YCursorPositionAfterRenderingWidgets)
		ImGui::SetCursorPosY(YCursorPositionBeforeRenderingWidgets);
}

void FESceneGraphUI::RenderNode(FENaiveSceneGraphNode* Node)
{
	if (!ShouldNodeBeVisible(Node))
		return;

	DrawAppropriateTreeArrow(Node);

	FETexture* BeforeNodeIcon = GetNodeIcon(Node);
	if (BeforeNodeIcon != nullptr)
	{
		ImGui::Image(BeforeNodeIcon->GetTextureID(), IconsSize);
		ImGui::SameLine();
	}

	float IconSpacing = GetFontSize() * 0.15f;
	float SpaceNeededForWidgetAtEnd = GetNodeWidgetAreaWidth(Node);
	float NodeBodyWidth = ImGui::GetContentRegionAvail().x - SpaceNeededForWidgetAtEnd - IconSpacing;

	std::string DisplayedName = GetNodeDisplayName(Node);
	std::string DisplayedText = APPLICATION.TruncateText(DisplayedName, NodeBodyWidth) + "##" + GetNodeInternalID(Node);

	if (bAlternatingNodeBackground)
	{
		bLastNodeWasEven = !bLastNodeWasEven;
		ImVec2 RectMin = ImGui::GetCursorScreenPos();
		ImVec2 RectMax = ImVec2(RectMin.x + NodeBodyWidth, RectMin.y + NodeHeight);
		ImGui::GetWindowDrawList()->AddRectFilled(RectMin, RectMax, bLastNodeWasEven ? ImColor(EvenNodeBackgroundColor) : ImColor(OddNodeBackgroundColor));
	}

	for (size_t i = 0; i < BeforeNodeRenderCallbacks.size(); i++)
		BeforeNodeRenderCallbacks[i](Node);
	
	bool bIsSelected = IsNodeSelected(Node);
	if (NodeIDBeingRenamed == Node->GetObjectID())
	{
		if (!bLastFrameRenameEditWasVisible)
		{
			ImGui::SetKeyboardFocusHere(0);

			ImGuiContext* Context = ImGui::GetCurrentContext();
			if (Context != nullptr)
				ImGui::SetFocusID(ImGui::GetID("##SceneGraphRenameEditor"), Context->CurrentWindow);
			
			ImGui::SetItemDefaultFocus();
			bLastFrameRenameEditWasVisible = true;
		}

		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);

		ImGui::SetNextItemWidth(NodeBodyWidth);
		if (ImGui::InputText("##SceneGraphRenameEditor", RenameBuffer, IM_ARRAYSIZE(RenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue) ||
			ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || !ImGui::IsItemFocused())
		{
			FEEntity* ObjectToRename = Node->GetEntity();
			if (ObjectToRename != nullptr)
				ObjectToRename->SetName(RenameBuffer);
			
			NodeIDBeingRenamed = "";
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(2);
	}
	else
	{
		ImGui::Selectable(DisplayedText.c_str(), bIsSelected, ImGuiSelectableFlags_None, ImVec2(NodeBodyWidth, NodeHeight));
	}

	for (size_t i = 0; i < AfterNodeRenderCallbacks.size(); i++)
		AfterNodeRenderCallbacks[i](Node);

	CheckInputs(Node);
	RenderNodeWidgets(Node);

	if (IsNodeExpanded(Node))
	{
		std::vector<FENaiveSceneGraphNode*> Children = Node->GetChildren();
		float ParentBottomY = ImGui::GetItemRectMin().y;
		for (size_t i = 0; i < Children.size(); i++)
		{
			if (ShouldNodeBeVisible(Children[i]))
			{
				DrawTreeConnectorLines(Children[i], ParentBottomY);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (Children[i]->GetDepth() - RenderingRoot->GetDepth() - (bRenderRootItself ? 0 : 1)) * NodeHeight);
				RenderNode(Children[i]);
			}
		}
	}
}

float FESceneGraphUI::GetFontSize() const
{
	return FontSize;
}

void FESceneGraphUI::SetFontSize(float NewFontSize)
{
	FontSize = NewFontSize;
	TooltipFontSize = std::max(FontSize / 2.0f, 16.0f);
	NodeHeight = FontSize;
	IconsSize = ImVec2(FontSize, FontSize);
}

void FESceneGraphUI::DebugCreateRandomWidgets(bool bInteractive)
{
	FETexture* RandomIcon = GetRandomDebugIcon();
	FESceneGraphNodeWidget NewDebugWidget;
	NewDebugWidget.Icon = RandomIcon;
	NewDebugWidget.bIsInteractive = bInteractive;

	NewDebugWidget.IsVisiblePredicate = [WidgetID = NewDebugWidget.GetID()](FENaiveSceneGraphNode* Node) -> bool {
		FEEntity* CurrentEntity = Node->GetEntity();
		if (CurrentEntity == nullptr)
			return true;

		size_t Seed = std::hash<std::string>{}(CurrentEntity->GetObjectID() + WidgetID);
		srand(static_cast<unsigned int>(Seed));

		float RandomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		if (RandomValue < 0.6f)
			return true;
		else
			return false;
	};

	bool AlreadyExistingWidgetWithSameProperties = false;
	for (size_t i = 0; i < DebugNodeWidgets.size(); i++)
	{
		if (DebugNodeWidgets[i].Icon->GetObjectID() == NewDebugWidget.Icon->GetObjectID() && DebugNodeWidgets[i].bIsInteractive == NewDebugWidget.bIsInteractive)
			AlreadyExistingWidgetWithSameProperties = true;
	}

	if (!AlreadyExistingWidgetWithSameProperties)
	{
		AddNodeWidget(NewDebugWidget);
		DebugNodeWidgets.push_back(NewDebugWidget);
	}
}

void FESceneGraphUI::DebugRenderUI()
{
	if (ImGui::Button("Expand all"))
		ExpandAllNodes();
	
	if (ImGui::Button("Collapse all"))
		CollapseAllNodes();

	ImGui::Checkbox("Render root", &bDebugRenderRoot);

	ImGui::Checkbox("Render random icons", &bDebugRenderRandomNodeIcons);
		
	if (bDebugRenderRandomNodeIcons)
	{
		SetNodeIconProvider([this] (FENaiveSceneGraphNode* Node) -> FETexture* {
			FEEntity* CurrentEntity = Node->GetEntity();
			if (CurrentEntity == nullptr)
				return nullptr;

			// More elegant solution without rand().
			size_t Hash = std::hash<std::string>{}(CurrentEntity->GetObjectID());
			size_t IconCount = DebugIconsIDs.size();
			if (IconCount == 0)
				return nullptr;

			// +1 to include a "no icon" option.
			size_t Index = Hash % (IconCount + 1);
			if (Index >= IconCount)
				return nullptr;

			return GetDebugIconByIndex(Index);
		});
	}
	else
	{
		SetNodeIconProvider(nullptr);
	}

	if (ImGui::Button("Clear Widgets"))
		ClearNodeWidgets();

	ImGui::SameLine();
	if (ImGui::Button("Add interactive widget"))
		DebugCreateRandomWidgets(true);
	
	ImGui::SameLine();
	if (ImGui::Button("Add non interactive widget"))
		DebugCreateRandomWidgets(false);
}

void FESceneGraphUI::RenderContextMenu()
{
	FEScene* CurrentScene = GetScene();
	if (CurrentScene == nullptr)
		return;

	if (bShouldOpenContextMenu)
	{
		HoveredNodeIDWhenContextMenuWasOpened = HoveredNodeID;
		ImGui::OpenPopup(("##Scene Graph Context Menu " + CurrentScene->GetObjectID()).c_str());
	}	

	bShouldOpenContextMenu = false;

	if (ImGui::BeginPopup(("##Scene Graph Context Menu " + CurrentScene->GetObjectID()).c_str()))
	{
		FENaiveSceneGraphNode* ContextNode = CurrentScene->SceneGraph.GetNodeByID(HoveredNodeIDWhenContextMenuWasOpened);
		if (ContextMenuRenderingFunction)
		{
			ContextMenuRenderingFunction(ContextNode);
		}
		else
		{
			ImGui::TextUnformatted("No context menu rendering function set.");
		}
		
		ImGui::EndPopup();
	}
	else
	{
		// Popup was closed (either by user dismissing or item selected).
		HoveredNodeIDWhenContextMenuWasOpened = "";
	}
}

void FESceneGraphUI::Render(FENaiveSceneGraphNode* RenderingRoot, bool bRenderRootItself)
{
	if (!bVisible)
		return;

	// If in debug mode ignore the provided inputs.
	if (IsInDebugMode())
	{
		if (GetTestScene() == nullptr)
			return;

		RenderingRoot = GetTestScene()->SceneGraph.GetRoot();
		bRenderRootItself = bDebugRenderRoot;

		DebugRenderUI();
	}

	if (RenderingRoot == nullptr)
		return;

	FEScene* CurrentScene = SCENE_MANAGER.GetSceneByNodeID(RenderingRoot->GetObjectID());
	if (CurrentScene == nullptr)
		return;

	CurrentSceneID = CurrentScene->GetObjectID();
	HoveredNodeID = "";

	this->RenderingRoot = RenderingRoot;
	this->bRenderRootItself = bRenderRootItself;
	bLastNodeWasEven = false;

	if (CousineFont == nullptr)
		CousineFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Resources/Cousine-Regular.ttf", 32.0f);

	if (bRenderTextFilterInput)
		RenderFilterTextInput();

	if (bRenderUIScaleControl)
	{
		if (bRenderTextFilterInput)
			ImGui::SameLine();

		RenderUIScaleControl();
	}

	ImGui::PushStyleColor(ImGuiCol_FrameBg, BackgroundColor);
	if (ImGui::BeginListBox(("##Scene Graph" + RenderingRoot->GetObjectID()).c_str(), ImVec2(ImGui::GetContentRegionAvail())))
	{
		// 0 - default.
		// 1 - highlighted connector lines (e.g. for selected nodes).
		ImGui::GetWindowDrawList()->ChannelsSplit(2);

		if (CousineFont != nullptr)
			ImGui::PushFont(CousineFont, GetFontSize());

		if (bRenderRootItself)
		{
			RenderNode(RenderingRoot);
		}
		else
		{
			std::vector<FENaiveSceneGraphNode*> Children = RenderingRoot->GetChildren();
			for (size_t i = 0; i < Children.size(); i++)
				RenderNode(Children[i]);
		}

		// Node widgets render on the same line as the Selectable and shift the cursor Y position due to vertical centering adjustments.
		// We restore the cursor to the pre-widget Y position so the next node starts at the correct vertical offset.
		// After all nodes are rendered, we set the cursor to correct position to ensure the parent container (e.g. ListBox) accounts for the full content height.
		// If not accounted for, ImGui will assert with request to use Dummy().
		if (YCursorPositionBeforeRenderingWidgets > YCursorPositionAfterRenderingWidgets)
			ImGui::SetCursorPosY(YCursorPositionAfterRenderingWidgets);
		
		if (CousineFont != nullptr)
			ImGui::PopFont();

		ImGui::EndListBox();
	}
	ImGui::PopStyleColor();

	ImGui::GetWindowDrawList()->ChannelsMerge();

	bSceneGraphWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_ChildWindows);
	if (bSceneGraphWindowHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		bShouldOpenContextMenu = true;

	RenderContextMenu();
}

void FESceneGraphUI::ExpandAllNodes()
{
	if (RenderingRoot == nullptr)
		return;

	std::vector<FENaiveSceneGraphNode*> Stack;
	Stack.push_back(RenderingRoot);
	while (!Stack.empty())
	{
		FENaiveSceneGraphNode* CurrentNode = Stack.back();
		Stack.pop_back();
		SetNodeExpanded(CurrentNode, true);
		for (FENaiveSceneGraphNode* Child : CurrentNode->GetChildren())
			Stack.push_back(Child);
	}
}

void FESceneGraphUI::CollapseAllNodes()
{
	if (RenderingRoot == nullptr)
		return;

	std::vector<FENaiveSceneGraphNode*> Stack;
	Stack.push_back(RenderingRoot);
	while (!Stack.empty())
	{
		FENaiveSceneGraphNode* CurrentNode = Stack.back();
		Stack.pop_back();
		SetNodeExpanded(CurrentNode, false);
		for (FENaiveSceneGraphNode* Child : CurrentNode->GetChildren())
			Stack.push_back(Child);
	}
}

bool FESceneGraphUI::IsInDebugMode()
{
	return bDebugMode;
}

bool FESceneGraphUI::InitiateTestScene()
{
	FEScene* SceneToWorkWith = GetTestScene();
	if (SceneToWorkWith == nullptr)
		return false;

	std::vector<FENaiveSceneGraphNode*> Nodes;
	for (size_t i = 0; i < 30; i++)
	{
		FEEntity* Entity = SceneToWorkWith->CreateEntity("Node_" + std::to_string(i));
		Nodes.push_back(SceneToWorkWith->SceneGraph.GetNodeByEntityID(Entity->GetObjectID()));
	}

	// Create a hierarchy:
	//
	//                  0
	//          /       |       \
    //         1        2        3
	//       / | \    / | \    / | \
    //      4  5  6  7  8  9  10 11 12
	//     /\  |     |  |  |   |  |  |\
    //   13 14 15   16 17 18  19 20 21 22
	//   |     |     |     |      |  |  \
    //  23    24    25    26      27 28  29

	// Level 1
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[1]->GetObjectID(), Nodes[0]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[2]->GetObjectID(), Nodes[0]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[3]->GetObjectID(), Nodes[0]->GetObjectID());

	// Level 2
	for (int i = 1; i <= 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			SceneToWorkWith->SceneGraph.MoveNode(Nodes[3 * i + j + 1]->GetObjectID(), Nodes[i]->GetObjectID());
		}
	}

	// Level 3
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[13]->GetObjectID(), Nodes[4]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[14]->GetObjectID(), Nodes[4]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[15]->GetObjectID(), Nodes[5]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[16]->GetObjectID(), Nodes[7]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[17]->GetObjectID(), Nodes[8]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[18]->GetObjectID(), Nodes[9]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[19]->GetObjectID(), Nodes[10]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[20]->GetObjectID(), Nodes[11]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[21]->GetObjectID(), Nodes[12]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[22]->GetObjectID(), Nodes[12]->GetObjectID());

	// Level 4
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[23]->GetObjectID(), Nodes[13]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[24]->GetObjectID(), Nodes[15]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[25]->GetObjectID(), Nodes[16]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[26]->GetObjectID(), Nodes[18]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[27]->GetObjectID(), Nodes[20]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[28]->GetObjectID(), Nodes[21]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[29]->GetObjectID(), Nodes[22]->GetObjectID());

	return true;
}

void FESceneGraphUI::SetDebugMode(bool bNewValue)
{
	bool bModeChanged = bDebugMode != bNewValue;
	bDebugMode = bNewValue;

	if (bModeChanged)
	{
		NodeState.clear();

		if (bDebugMode)
		{
			FEScene* NewScene = SCENE_MANAGER.CreateScene("Test scene");
			TestSceneID = NewScene->GetObjectID();
			ClearAllCallbacks();
			ClearNodeWidgets();

			InitiateTestScene();
		}
		else
		{
			ClearAllCallbacks();
			ClearNodeWidgets();
			DebugNodeWidgets.clear();

			FEScene* TestScene = GetTestScene();
			if (TestScene != nullptr)
				SCENE_MANAGER.DeleteScene(TestScene);
		}
	}
	
}

FEScene* FESceneGraphUI::GetTestScene()
{
	return SCENE_MANAGER.GetSceneByID(TestSceneID);
}

FETexture* FESceneGraphUI::GetDebugIconByIndex(const size_t& IconIndex)
{
	FETexture* Result = RESOURCE_MANAGER.NoTexture;
	if (IconIndex >= DebugIconsIDs.size())
		return Result;

	FETexture* FoundTexture = RESOURCE_MANAGER.GetTexture(DebugIconsIDs[IconIndex]);
	if (FoundTexture != nullptr)
		Result = FoundTexture;

	return Result;
}

FETexture* FESceneGraphUI::GetRandomDebugIcon()
{
	size_t Seed = static_cast<size_t>(time(nullptr));
	srand(static_cast<unsigned int>(Seed));
	if (DebugIconsIDs.size() == 0)
		return RESOURCE_MANAGER.NoTexture;

	size_t RandomIndex = rand() % DebugIconsIDs.size();
	return GetDebugIconByIndex(RandomIndex);
}

std::vector<std::string> FESceneGraphUI::GetDebugIconsIDs() const
{
	return DebugIconsIDs;
}

void FESceneGraphUI::SetDebugIconsIDs(const std::vector<std::string>& NewDebugIconsIDs)
{
	DebugIconsIDs = NewDebugIconsIDs;
}

void FESceneGraphUI::AddBeforeNodeRenderCallback(std::function<void(FENaiveSceneGraphNode*)> Callback)
{
	BeforeNodeRenderCallbacks.push_back(Callback);
}

void FESceneGraphUI::AddAfterNodeRenderCallback(std::function<void(FENaiveSceneGraphNode*)> Callback)
{
	AfterNodeRenderCallbacks.push_back(Callback);
}

void FESceneGraphUI::RenderUIScaleControl(float Min, float Max)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

	float FrameHeight = ImGui::GetFrameHeight();
	float TextHeight = ImGui::GetTextLineHeight();
	float Offset = (FrameHeight - TextHeight) * 0.5f;

	ImGui::Text("UI Scale");
	ImGui::SameLine();
	
	ImGui::SetNextItemWidth(60.0f);
	float LocalFontSize = FontSize;
	ImGui::DragFloat("##FontSize", &LocalFontSize, 0.5f, Min, Max, "%.1f", ImGuiSliderFlags_AlwaysClamp);
	if (LocalFontSize != FontSize)
		SetFontSize(LocalFontSize);
	
	ImGui::PopStyleVar();
}

int FESceneGraphUI::FilterInputTextCallback(ImGuiInputTextCallbackData* Data)
{
	if (Data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
	{
		bool bIsFocused = ImGui::IsItemActive();
		if (bIsPlaceHolderTextUsed)
		{
			// Check if the input just gained focus
			if (bIsFocused && !bFilterInputWasFocused)
			{
				strcpy_s(CharFilterText, "");

				// Update ImGui's buffer.
				Data->BufDirty = true;
				Data->DeleteChars(0, Data->BufTextLen);
				Data->InsertChars(0, CharFilterText);

				bIsPlaceHolderTextUsed = false;
			}
		}

		bFilterInputWasFocused = bIsFocused;
	}

	return 0;
}

void FESceneGraphUI::RenderFilterTextInput()
{
	const bool bIsPlaceHolderTextUsedWasOn = bIsPlaceHolderTextUsed;

	if (bIsPlaceHolderTextUsedWasOn)
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 150));

	// Setting up the callback for the input text.
	std::function<int(ImGuiInputTextCallbackData*)> Callback = std::bind(&FESceneGraphUI::FilterInputTextCallback, this, std::placeholders::_1);
	auto StaticCallback = [](ImGuiInputTextCallbackData* Data) -> int {
		const auto& Callback = *static_cast<std::function<int(ImGuiInputTextCallbackData*)>*>(Data->UserData);
		return Callback(Data);
	};

	float AvailableWidth = ImGui::GetContentRegionAvail().x;
	if (bRenderUIScaleControl)
	{
		ImVec2 TextSize = ImGui::CalcTextSize("UI Scale");
		AvailableWidth -= TextSize.x + ImGui::GetStyle().ItemSpacing.x;
		AvailableWidth -= 60.0f + 4.0f; // The width of the DragFloat control.
	}
	
	ImGui::SetNextItemWidth(AvailableWidth);
	if (ImGui::InputText("##SceneGraphWindowFilter", CharFilterText, FilterInputBufferSize, ImGuiInputTextFlags_CallbackAlways, StaticCallback, &Callback))
	{
		bFilterEnabled = true;
		FilterText = CharFilterText;
	}

	if (!ImGui::IsItemActive())
	{
		if (strlen(CharFilterText) == 0)
		{
			strcpy_s(CharFilterText, PlaceHolderTextString.c_str());
			bIsPlaceHolderTextUsed = true;
			bFilterInputWasFocused = false;
			bFilterEnabled = false;
		}
	}

	if (bIsPlaceHolderTextUsedWasOn)
		ImGui::PopStyleColor();
}

bool FESceneGraphUI::IsNodeBeingRenamed(FENaiveSceneGraphNode* Node)
{
	if (Node == nullptr)
		return false;

	return NodeIDBeingRenamed == Node->GetObjectID();
}

bool FESceneGraphUI::ActivateRenameForNode(FENaiveSceneGraphNode* Node)
{
	if (Node == nullptr)
		return false;

	if (!ShouldNodeBeVisible(Node))
		return false;

	if (IsNodeBeingRenamed(Node))
		return true;

	if (!IsNodeSelected(Node))
		SetNodeSelected(Node, true);

	NodeIDBeingRenamed = Node->GetObjectID();

	std::string NodeDisplayName = GetNodeDisplayName(Node);
	strcpy_s(RenameBuffer, NodeDisplayName.size() + 1, NodeDisplayName.c_str());
	bLastFrameRenameEditWasVisible = false;
	return true;
}