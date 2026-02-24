#pragma once
#include "ProjectWasModifiedPopUp.h"

struct FESceneGraphNodeStateData
{
	bool bExpanded = false;
	bool bSelected = false;
};

struct FESceneGraphNodeWidget
{
	friend class FESceneGraphUI;
private:
	std::string ID;

public:
	FESceneGraphNodeWidget::FESceneGraphNodeWidget()
	{
		ID = APPLICATION.GetUniqueHexID();
	}

	std::string GetID() const
	{
		return ID;
	}

	std::string TooltipText = "";

	FETexture* Icon = nullptr;
	std::function<FETexture* (FENaiveSceneGraphNode*)> DynamicIconProvider = nullptr;

	ImVec4 HoveredColor = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
	ImVec4 ActiveColor = ImVec4(1.0f, 1.0f, 1.0f, 0.3f);

	bool bIsInteractive = false;
	std::function<void(FENaiveSceneGraphNode*)> OnClickCallback = nullptr;
	bool bIsVisibleByDefault = true;
	std::function<bool(FENaiveSceneGraphNode*)> IsVisiblePredicate = nullptr;
};

class FESceneGraphUI
{
	// Core.
	std::string CurrentSceneID;
	bool bVisible = true;
	FENaiveSceneGraphNode* RenderingRoot = nullptr;
	bool bRenderRootItself = false;
	void RenderNode(FENaiveSceneGraphNode* Node);


	// Appearance.
	bool bBackgroundColorSwitch = true;
	ImVec4 BackgroundColor = ImVec4(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 1.0f);
	ImVec4 EvenNodeBackgroundColor = ImVec4(50.0f / 255.0f, 50.0f / 255.0f, 50.0f / 255.0f, 1.0f);
	ImVec4 OddNodeBackgroundColor = ImVec4(90.0f / 255.0f, 90.0f / 255.0f, 90.0f / 255.0f, 1.0f);
	ImVec4 ConnectorLineColor = ImVec4(128.0f / 255.0f, 140.0f / 255.0f, 170.0f / 255.0f, 1.0f);
	float ConnectorLineThickness = 2.0f;
	ImVec4 SelectedNodeConnectorLineColor = ImVec4(48.0f / 255.0f, 95.0f / 255.0f, 213.0f / 255.0f, 1.0f);
	float SelectedConnectorLineThickness = 2.6f;
	bool bAlternatingNodeBackground = true;
	//bool bOnlyTextPartOfNodeUsesBackground = true;
	static bool bLastNodeWasEven;
	
	ImFont* CousineFont = nullptr;
	float FontSize = 32.0f;
	float TooltipFontSize = FontSize / 2.0f;
	float NodeHeight = 32.0f;
	ImVec2 IconsSize = ImVec2(NodeHeight, NodeHeight);
	bool bRenderUIScaleControl = true;
	void RenderUIScaleControl(float Min = 8.0f, float Max = 128.0f);


	// Node state (expand/collapse/selection).
	std::unordered_map<std::string, FESceneGraphNodeStateData> NodeState;
	bool bAllowMultipleNodeSelection = false;
	std::function<bool(FENaiveSceneGraphNode*)> NodeSelectionPredicate = nullptr;
	bool IsNodeSelected(FENaiveSceneGraphNode* Node);
	void SetNodeSelected(FENaiveSceneGraphNode* Node, bool bSelected);


	// Predicates and providers.
	std::function<bool(FENaiveSceneGraphNode*)> NodeRenderPredicate = nullptr;
	std::function<std::string(FENaiveSceneGraphNode*)> NodeDisplayNameProvider = nullptr;
	std::string GetNodeDisplayName(FENaiveSceneGraphNode* Node);
	std::function<bool(FENaiveSceneGraphNode*)> NodeChildrenVisiblePredicate = nullptr;
	std::function<FETexture*(FENaiveSceneGraphNode*)> NodeIconProvider = nullptr;
	FETexture* GetNodeIcon(FENaiveSceneGraphNode* Node);


	// Visibility/filtering.
	std::vector<std::string> HiddenEntityTags;
	bool ShouldNodeBeVisible(FENaiveSceneGraphNode* Node);
	bool AreNodeChildrenVisible(FENaiveSceneGraphNode* Node);

	bool bRenderTextFilterInput = true;
	bool bFilterEnabled = false;
	bool bCaseSensitiveFiltering = false;
	std::string FilterText = "";
	bool DoesNodePassTextFilter(FENaiveSceneGraphNode* Node);
	void RenderFilterTextInput();
	static constexpr size_t FilterInputBufferSize = 2048;
	char CharFilterText[FilterInputBufferSize];
	bool bIsPlaceHolderTextUsed = true;
	bool bFilterInputWasFocused = false;
	std::string PlaceHolderTextString = "Filter entities...";
	int FilterInputTextCallback(ImGuiInputTextCallbackData* Data);
	

	// Tree visualization.
	float TreeArrowsThicknessCoefficient = 0.07f;
	float LineJoinOverlapFactor = 0.77f;
	bool IsNodePartOfBranch(FENaiveSceneGraphNode* NodeToCheck, FENaiveSceneGraphNode* BranchRoot, FENaiveSceneGraphNode* BranchLeaf);
	bool bHighlightSelectedNodeConnectorLines = true;
	void DrawTreeConnectorLines(FENaiveSceneGraphNode* Node, float ParentBottomY);
	void DrawAppropriateTreeArrow(FENaiveSceneGraphNode* Node);


	// Input handling.
	bool bSceneGraphWindowHovered = false;
	std::string HoveredNodeID = "";
	std::string HoveredNodeIDWhenContextMenuWasOpened = "";
	bool bShouldOpenContextMenu = false;
	std::function<void(FENaiveSceneGraphNode*)> ContextMenuRenderingFunction = nullptr;
	void RenderContextMenu();

	std::vector<std::function<void(FENaiveSceneGraphNode*)>> OnNodeHoveredCallbacks;
	std::vector<std::function<void(FENaiveSceneGraphNode*, ImGuiMouseButton_)>> OnNodeClickedCallbacks;
	std::vector<std::function<void(FENaiveSceneGraphNode*, ImGuiMouseButton_)>> OnNodeDoubleClickedCallbacks;
	void CheckInputs(FENaiveSceneGraphNode* Node);


	// Before/After render callbacks.
	std::vector<std::function<void(FENaiveSceneGraphNode*)>> BeforeNodeRenderCallbacks;
	std::vector<std::function<void(FENaiveSceneGraphNode*)>> AfterNodeRenderCallbacks;


	// Renaming functionality.
	std::string NodeIDBeingRenamed = "";
	char RenameBuffer[1024];
	bool bLastFrameRenameEditWasVisible = false;


	// Node widgets.
	std::vector<FESceneGraphNodeWidget> NodeWidgets;
	float WidgetIconVisualRenderingFactor = 0.9f;
	float YCursorPositionBeforeRenderingWidgets = 0.0f;
	float YCursorPositionAfterRenderingWidgets = 0.0f;

	bool ShouldRenderWidgetForNode(FENaiveSceneGraphNode* Node, FESceneGraphNodeWidget& Widget, FETexture** IconToUse);
	float GetNodeWidgetAreaWidth(FENaiveSceneGraphNode* Node);
	size_t GetNodeWidgetCount(FENaiveSceneGraphNode* Node);
	void RenderNodeWidgets(FENaiveSceneGraphNode* Node);

	
	// Debug stuff.
	bool bDebugMode = false;
	bool bDebugRenderRoot = true;
	bool bDebugRenderRandomNodeIcons = false;

	std::vector<std::string> DebugIconsIDs;
	FETexture* GetDebugIconByIndex(const size_t& IconIndex);
	FETexture* GetRandomDebugIcon();
	void DebugCreateRandomWidgets(bool bInteractive);

	std::vector<FESceneGraphNodeWidget> DebugNodeWidgets;

	std::string TestSceneID;
	FEScene* GetTestScene();
	bool InitiateTestScene();
	void DebugRenderUI();
public:
	FESceneGraphUI();
	~FESceneGraphUI();

	std::string GetSceneID() const;
	FEScene* GetScene() const;
	FENaiveSceneGraphNode* GetCurrentRenderingRoot() const;
	std::string GetNodeInternalID(FENaiveSceneGraphNode* Node);

	void Render(FENaiveSceneGraphNode* RenderingRoot, bool bRenderRootItself = true);
	float GetFontSize() const;
	void SetFontSize(float NewFontSize);

	void SetNodeRenderPredicate(std::function<bool(FENaiveSceneGraphNode*)> Predicate);
	void SetNodeDisplayNameProvider(std::function<std::string(FENaiveSceneGraphNode*)> Provider);
	void SetNodeChildrenVisiblePredicate(std::function<bool(FENaiveSceneGraphNode*)> Predicate);
	void SetNodeSelectionPredicate(std::function<bool(FENaiveSceneGraphNode*)> Predicate);
	void SetNodeIconProvider(std::function<FETexture* (FENaiveSceneGraphNode*)> Provider);
	void ClearAllProvidersAndPredicates();

	void AddBeforeNodeRenderCallback(std::function<void(FENaiveSceneGraphNode*)> Callback);
	void AddAfterNodeRenderCallback(std::function<void(FENaiveSceneGraphNode*)> Callback);

	std::vector<std::string> GetSelectedNodeIDs() const;
	bool IsNodeExpanded(FENaiveSceneGraphNode* Node);
	void SetNodeExpanded(FENaiveSceneGraphNode* Node, bool bExpanded);
	void ExpandAllNodes();
	void CollapseAllNodes();
	
	std::vector<std::string> GetHiddenEntityTags() const;
	void SetHiddenEntityTags(const std::vector<std::string>& NewHiddenEntityTags);
	void AddHiddenEntityTag(const std::string& TagToAdd);
	void RemoveHiddenEntityTag(const std::string& TagToRemove);
	void ClearHiddenEntityTags();

	FESceneGraphNodeWidget* GetNodeWidgetByID(const std::string& WidgetID);
	std::vector<FESceneGraphNodeWidget> GetAllNodeWidgets() const;
	bool AddNodeWidget(FESceneGraphNodeWidget& Widget);
	bool RemoveNodeWidget(const std::string& WidgetID);
	void ClearNodeWidgets();

	void AddOnNodeHoveredCallback(std::function<void(FENaiveSceneGraphNode*)> Callback);
	void ClearOnNodeHoveredCallbacks();

	void AddOnNodeClickedCallback(std::function<void(FENaiveSceneGraphNode*, ImGuiMouseButton_)> Callback);
	void ClearOnNodeClickedCallbacks();

	void AddOnNodeDoubleClickedCallback(std::function<void(FENaiveSceneGraphNode*, ImGuiMouseButton_)> Callback);
	void ClearOnNodeDoubleClickedCallbacks();

	void ClearAllInputCallbacks();
	void ClearAllCallbacks();

	void SetContextMenuRenderingFunction(std::function<void(FENaiveSceneGraphNode*)> Function);
	void ClearContextMenuRenderingFunction();

	bool IsNodeBeingRenamed(FENaiveSceneGraphNode* Node);
	bool ActivateRenameForNode(FENaiveSceneGraphNode* Node);

	bool IsInDebugMode();
	void SetDebugMode(bool bNewValue);

	std::vector<std::string> GetDebugIconsIDs() const;
	void SetDebugIconsIDs(const std::vector<std::string>& NewDebugIconsIDs);
};