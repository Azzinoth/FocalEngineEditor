#pragma once

#include "EditPopups.h"
#pragma warning (disable: 4724)

class ShaderDebugWindow : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(ShaderDebugWindow)

	TextEditor Editor;
	ImGuiButton* CloseButton = nullptr;
	ImGuiButton* UpdateButton = nullptr;
	FEShader* ShaderToWorkWith = nullptr;
	bool bUpdateNeeded = true;
	std::string SelectedDebugData;
	std::vector<std::string> OccurrenceList;
	std::vector<std::vector<float>>* Data = nullptr;
	std::vector<std::vector<float>> DataDump;
public:
	SINGLETON_PUBLIC_PART(ShaderDebugWindow)

	void Show(FEShader* Shader, std::string Caption);
	void Render() override;
};

class ShaderEditorWindow : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(ShaderEditorWindow)

	FEShader* ShaderToEdit = nullptr;
	FEShader* DummyShader = nullptr;
	TextEditor* CurrentEditor = nullptr;
	TextEditor VertexShaderEditor;
	TextEditor TessControlShaderEditor;
	TextEditor TessEvalShaderEditor;
	TextEditor GeometryShaderEditor;
	TextEditor FragmentShaderEditor;
	TextEditor ComputeShaderEditor;

	bool bVertexShaderUsed = false;
	bool bFragmentShaderUsed = false;
	bool bTessControlShaderUsed = false;
	bool bTessEvalShaderUsed = false;
	bool bGeometryShaderUsed = false;
	bool bComputeShaderUsed = false;

	ImGuiButton* CompileButton = nullptr;
	ImGuiButton* CloseButton = nullptr;
	int ActiveTab = 0;

	void ReplaceShader(FEShader* OldShader, FEShader* NewShader);
public:
	SINGLETON_PUBLIC_PART(ShaderEditorWindow)

	void Show(FEShader* Shader);

	void Render() override;
};