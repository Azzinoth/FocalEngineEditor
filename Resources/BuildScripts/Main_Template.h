#include "SubSystems/FocalEngine/FEngine.h"
using namespace FocalEngine;

// FIX ME! Move that to a RESOURCE_MANAGER
void LoadResources(std::string DirectoryPath);

// TO-DO: Make that process in memory, not involving creating temporary files.
//FEAssetPackage* SaveResourcesToAssetPackage();
void LoadResourcesFromAssetPackage(FEAssetPackage* AssetPackage);