#pragma once

// Public API of editor module of PLUGIN_NAME_PLACE_HOLDER, editor modules of other plugins include only this header.
// Editor module that uses it should list editor module of PLUGIN_NAME_PLACE_HOLDER in its manifest dependencies, so it is loaded first.
// Usage:
//	using PluginInterface = INTERFACE_NAME_PLACE_HOLDER;
//	PluginInterface* Plugin = Editor->GetEditorModule<PluginInterface>(UNIQUE_ID.FromString(PluginInterface::PluginID));
//	if (Plugin != nullptr && Plugin->GetInterfaceVersion() == PluginInterface::InterfaceVersion)
//		Plugin->...;
class INTERFACE_NAME_PLACE_HOLDER
{
public:
	// Same as "ID" in PLUGIN_NAME_PLACE_HOLDER.json.
	static constexpr const char* PluginID = "PLUGIN_ID_PLACE_HOLDER";

	// Increase when anything in this file changes.
	static constexpr int InterfaceVersion = 1;

	// Order of virtual functions is part of the interface, new functions should be added only at the end.
	virtual ~INTERFACE_NAME_PLACE_HOLDER() = default;
	virtual int GetInterfaceVersion() const { return InterfaceVersion; }
};