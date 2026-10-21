# Editor module, it is loaded only by the editor and uses engine module through public interface.
file(GLOB Editor_SRC
	"Editor/${PROJECT_NAME}Editor.h"
	"Editor/${PROJECT_NAME}Editor.cpp"
	"SubSystems/EditorAPI/FEEditorAPI.h"
)

fe_add_plugin_module(${PROJECT_NAME}Editor SOURCES ${Editor_SRC})

# Only build order, editor module does not link to engine module.
add_dependencies(${PROJECT_NAME}Editor ${PROJECT_NAME})