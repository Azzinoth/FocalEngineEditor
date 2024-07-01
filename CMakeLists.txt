cmake_minimum_required(VERSION 3.10)

set(BUILD_TYPE "Debug and Release" CACHE STRING "Choose Build type")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

# set the project name
project(FocalEngineEditor)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "/MP /ZI /W3 /Od /MDd /JMC /sdl /FC /D IMGUI_IMPL_OPENGL_LOADER_GLEW /D WIN32 /D _WINDOWS /D _DEBUG /D FE_DEBUG_ENABLED")
set(CMAKE_CXX_FLAGS_RELEASE "/MP /W3 /O2 /Oi /MD /Gy /sdl /FC /GL /OPT:REF /OPT:ICF /D IMGUI_IMPL_OPENGL_LOADER_GLEW /D NDEBUG /D WIN32 /D _WINDOWS")

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Force your path to Dear ImGui
set(DEAR_IMGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty/imgui CACHE PATH "Path to Dear ImGui directory." FORCE)

if(NOT TARGET VisualNodeSystem)
	add_subdirectory(SubSystems/NodeSystem/VisualNodeSystem)
endif()

if(NOT TARGET FocalEngine)
	add_subdirectory(SubSystems/FocalEngine)
endif()

# Extract the relative path of the engine folder
file(RELATIVE_PATH RELATIVE_PATH_TO_ENGINE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}" "${ENGINE_FOLDER}")
# We need to re-configure the Config.h file for the engine
# This is necessary for any project that will include FocalEngine
# to make sure that the engine can find its dependencies
set(ENGINE_FOLDER "${RELATIVE_PATH_TO_ENGINE_FOLDER}")
configure_file(${ENGINE_FOLDER}/ResourceManager/Config.h.in ${ENGINE_FOLDER}/ResourceManager/Config.h @ONLY)

# *************** EDITOR ***************
file(GLOB Editor_SRC
	"main.cpp"
	"FEEditor.cpp"
	"FEEditor.h"
	"FEEditorDragAndDropManager.cpp"
	"FEEditorDragAndDropManager.h"
	"FEEditorGizmoManager.cpp"
	"FEEditorGizmoManager.h"
	"FEEditorHaloSelectionEffect.cpp"
	"FEEditorHaloSelectionEffect.h"
	"FEEditorInternalResources.cpp"
	"FEEditorInternalResources.h"
	"FEEditorPreviewManager.cpp"
	"FEEditorPreviewManager.h"
	"FEEditorSelectedObject.cpp"
	"FEEditorSelectedObject.h"
	"FEEditorVirtualFileSystem.cpp"
	"FEEditorVirtualFileSystem.h"
	"FEProject.cpp"
	"FEProject.h"
)

file(GLOB FEDearImguiWrapper_SRC
	"FEDearImguiWrapper/FEDearImguiWrapper.cpp"
	"FEDearImguiWrapper/FEDearImguiWrapper.h"
)

file(GLOB EditorWindows_SRC
	"EditorWindows/DeletePopups.cpp"
	"EditorWindows/DeletePopups.h"
	"EditorWindows/EditPopups.cpp"
	"EditorWindows/EditPopups.h"
	"EditorWindows/GyzmosSettingsWindow.h"
	"EditorWindows/CombineChannelsToTexturePopUp.cpp"
	"EditorWindows/CombineChannelsToTexturePopUp.h"
	"EditorWindows/ProjectWasModifiedPopUp.cpp"
	"EditorWindows/ProjectWasModifiedPopUp.h"
	"EditorWindows/RenamePopups.cpp"
	"EditorWindows/RenamePopups.h"
	"EditorWindows/SelectPopups.cpp"
	"EditorWindows/SelectPopups.h"
	"EditorWindows/ShaderEditorWindow.cpp"
	"EditorWindows/ShaderEditorWindow.h"
	"EditorWindows/ResizeTexturePopup.cpp"
	"EditorWindows/ResizeTexturePopup.h"
	"EditorWindows/PrefabEditorWindow.cpp"
	"EditorWindows/PrefabEditorWindow.h"
	"EditorWindows/DebugTextureViewWindow.cpp"
	"EditorWindows/DebugTextureViewWindow.h"
	"EditorWindows/SceneGraphWindow.cpp"
	"EditorWindows/SceneGraphWindow.h"
	"EditorWindows/InspectorWindow.cpp"
	"EditorWindows/InspectorWindow.h"
	"EditorWindows/ContentBrowserWindow.cpp"
	"EditorWindows/ContentBrowserWindow.h"
	"EditorWindows/LogWindow.cpp"
)

file(GLOB Editor_Materials_SRC
	"Resources/Materials/FE_HaloDrawObject_FS.glsl"
	"Resources/Materials/FE_HaloDrawObject_INSTANCED_VS.glsl"
	"Resources/Materials/FE_HaloDrawObject_VS.glsl"
	"Resources/Materials/FE_HaloSelectionEffect_FS.glsl"
	"Resources/Materials/FE_MeshPreview_FS.glsl"
	"Resources/Materials/FE_MeshPreview_VS.glsl"
	"Resources/Materials/FE_PixelAccurateSelection_FS.glsl"
	"Resources/Materials/FE_PixelAccurateSelection_INSTANCED_VS.glsl"
	"Resources/Materials/FE_PixelAccurateSelection_VS.glsl"
)

file(GLOB VisualNodeSystem_CustomNodes_SRC
	"SubSystems/NodeSystem/CustomNodes/FEEditorStandardNodes.h"
	"SubSystems/NodeSystem/CustomNodes/FEEditorMaterialNode.cpp"
	"SubSystems/NodeSystem/CustomNodes/FEEditorMaterialNode.h"
	"SubSystems/NodeSystem/CustomNodes/FEEditorFloatSourceNode.cpp"
	"SubSystems/NodeSystem/CustomNodes/FEEditorFloatSourceNode.h"
	"SubSystems/NodeSystem/CustomNodes/FEEditorTextureSourceNode.cpp"
	"SubSystems/NodeSystem/CustomNodes/FEEditorTextureSourceNode.h"
	"SubSystems/NodeSystem/CustomNodes/FEEditorTextureCreatingNode.cpp"
	"SubSystems/NodeSystem/CustomNodes/FEEditorTextureCreatingNode.h"
)

# *************** THIRD_PARTY ***************

file(GLOB textEditor_SRC
	"ThirdParty/textEditor/TextEditor.cpp"
)

link_directories(${GLEW_LIB_DIR})
link_directories(${GLFW_LIB_DIR})

# add the executable
add_executable(FocalEngineEditor WIN32
								 ${Editor_SRC}
								 ${FEDearImguiWrapper_SRC}
								 ${EditorWindows_SRC}
								 ${Editor_Materials_SRC}
								 ${VisualNodeSystem_CustomNodes_SRC}
								 # *************** SUB_SYSTEM ***************
								 
								 # *************** THIRD_PARTY ***************
								 ${textEditor_SRC}
)

target_link_libraries(FocalEngineEditor PRIVATE VisualNodeSystem)
target_link_libraries(FocalEngineEditor PRIVATE FocalEngine)

source_group("Source Files" FILES ${Application_SRC})
# *************** EDITOR ***************
source_group("Source Files/" FILES ${Editor_SRC})
source_group("Source Files/FEDearImguiWrapper" FILES ${FEDearImguiWrapper_SRC})
source_group("Source Files/EditorWindows" FILES ${EditorWindows_SRC})
source_group("Source Files/Materials" FILES ${Editor_Materials_SRC})
source_group("Source Files/NodeSystem/CustomNodes" FILES ${VisualNodeSystem_CustomNodes_SRC})
# *************** SUB_SYSTEM ***************

# *************** THIRD_PARTY ***************
source_group("Source Files/ThirdParty/textEditor" FILES ${textEditor_SRC})

# set the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT FocalEngineEditor)

include_directories(
	${GLEW_INCLUDE_DIR}
	${BASICAPP_THIRDPARTY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty/imgui
	${BASICAPP_DIR}
	${FOCAL_ENGINE_INCLUDES}
	${VISUAL_NODE_SYSTEM_DIR}
)