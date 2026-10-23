# Editor module, it is loaded only by the editor.
file(GLOB Editor_SRC
	"Editor/${PROJECT_NAME}Editor.h"
	"Editor/${PROJECT_NAME}Editor.cpp"
	"SubSystems/EditorAPI/FEEditorAPI.h"
)

fe_add_plugin_module(${PROJECT_NAME}Editor SOURCES ${Editor_SRC})

if(TARGET ${PROJECT_NAME})
	# Only build order, editor module uses engine module through public interface and does not link to it.
	add_dependencies(${PROJECT_NAME}Editor ${PROJECT_NAME})
else()
	# Plugin has no engine module.
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}Editor)
endif()