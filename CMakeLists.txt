cmake_minimum_required(VERSION 3.1)

set(BUILD_TYPE "Debug and Release" CACHE STRING "Choose Build type")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

# set the project name
project(FocalEngineEditor LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_DEBUG "/MP /ZI /W3 /Od /MDd /JMC /sdl /FC /D IMGUI_IMPL_OPENGL_LOADER_GLEW /D WIN32 /D _WINDOWS /D _DEBUG /D FE_DEBUG_ENABLED")
set(CMAKE_CXX_FLAGS_RELEASE "/MP /W3 /O2 /Oi /Gy /sdl /FC /GL /OPT:REF /OPT:ICF /D IMGUI_IMPL_OPENGL_LOADER_GLEW /D NDEBUG /D WIN32 /D _WINDOWS")

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT TARGET FEVisualNodeSystem)
	add_subdirectory(SubSystems/NodeSystem/FEVisualNodeSystem)
endif()

if(NOT TARGET FocalEngine)
	add_subdirectory(SubSystems/FocalEngine)
endif()

# *************** EDITOR ***************
file(GLOB Editor_SRC
	"main.cpp"
	"FEEditor.cpp"
	"FEEditor.h"
	"FEEditorContentBrowser.cpp"
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

file(GLOB FEEditorSubWindows_SRC
	"FEEditorSubWindows/deletePopups.cpp"
	"FEEditorSubWindows/deletePopups.h"
	"FEEditorSubWindows/editPopups.cpp"
	"FEEditorSubWindows/editPopups.h"
	"FEEditorSubWindows/gyzmosSettingsWindow.h"
	"FEEditorSubWindows/combineChannelsToTexturePopUp.cpp"
	"FEEditorSubWindows/combineChannelsToTexturePopUp.h"
	"FEEditorSubWindows/projectWasModifiedPopUp.cpp"
	"FEEditorSubWindows/projectWasModifiedPopUp.h"
	"FEEditorSubWindows/renamePopups.cpp"
	"FEEditorSubWindows/renamePopups.h"
	"FEEditorSubWindows/selectPopups.cpp"
	"FEEditorSubWindows/selectPopups.h"
	"FEEditorSubWindows/shaderEditorWindow.cpp"
	"FEEditorSubWindows/shaderEditorWindow.h"
	"FEEditorSubWindows/resizeTexturePopup.cpp"
	"FEEditorSubWindows/resizeTexturePopup.h"
	"FEEditorSubWindows/prefabEditorWindow.cpp"
	"FEEditorSubWindows/prefabEditorWindow.h"
	"FEEditorSubWindows/debugTextureViewWindow.cpp"
	"FEEditorSubWindows/debugTextureViewWindow.h"
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

file(GLOB FEVisualNodeSystem_CustomNodes_SRC
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
								 ${FEEditorSubWindows_SRC}
								 ${Editor_Materials_SRC}
								 ${FEVisualNodeSystem_CustomNodes_SRC}
								 # *************** SUB_SYSTEM ***************
								 
								 # *************** THIRD_PARTY ***************
								 ${textEditor_SRC}
)

target_link_libraries(FocalEngineEditor FEVisualNodeSystem)
target_link_libraries(FocalEngineEditor FocalEngine)

target_link_libraries(FocalEngineEditor glew32s.lib)
target_link_libraries(FocalEngineEditor glfw3.lib)
target_link_libraries(FocalEngineEditor glfw3dll.lib)
target_link_libraries(FocalEngineEditor opengl32.lib)

source_group("Source Files" FILES ${Application_SRC})
# *************** EDITOR ***************
source_group("Source Files/" FILES ${Editor_SRC})
source_group("Source Files/FEDearImguiWrapper" FILES ${FEDearImguiWrapper_SRC})
source_group("Source Files/FEEditorSubWindows" FILES ${FEEditorSubWindows_SRC})
source_group("Source Files/Materials" FILES ${Editor_Materials_SRC})
source_group("Source Files/NodeSystem/CustomNodes" FILES ${FEVisualNodeSystem_CustomNodes_SRC})
# *************** SUB_SYSTEM ***************

# *************** THIRD_PARTY ***************
source_group("Source Files/ThirdParty/textEditor" FILES ${textEditor_SRC})

# set the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT FocalEngineEditor)

include_directories(
	${GLEW_INCLUDE_DIR}
	${BASICAPP_THIRDPARTY_DIR}
	${BASICAPP_DIR}
	${FOCAL_ENGINE_THIRD_PARTY_DIR}
	${VISUAL_NODE_SYSTEM_DIR}
)