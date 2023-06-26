# Focal Engine Editor

![build](https://github.com/Azzinoth/FocalEngineEditor/actions/workflows/Build.yml/badge.svg?branch=master)

The Focal Engine Editor is a comprehensive tool for the [Focal Engine](https://github.com/Azzinoth/FocalEngine/), providing a variety of utilities for creating and managing your scene. The engine has been designed to function independently from the editor, ensuring modularity. For this reason, the engine and editor are separate projects.

Here are some images from scene created using the Focal Engine Editor:

![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/1.png)
![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/2.png)
![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/3.png)
![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/4.png)
![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/5.png)

## Features

The Focal Engine Editor provides the ability to load, save, create new projects, and delete existing ones through the Project Browser window:

![Project Browser](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Project%20Browser.png)

In the Editor, the following key windows are available:

1. **Scene Entities**: Displays all objects, cameras, light sources, and terrains in the scene.
2. **Scene**: Presents the main in-game camera view.
3. **Inspector**: Provides the properties of the selected object.
4. **Content Browser**: Serves as an explorer window for the current project's virtual file system with all resources structured.

![Editor](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Editor.png)

5. **Effects Settings**: Allows adjustments of different effects available in the engine.

![Effects Settings](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Effects%20Settings.png)

The Material Editor employs the [Visual Node System](https://github.com/Azzinoth/VisualNodeSystem) to enhance editing intuitiveness.

![Material Editor](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Material%20editor.png)

Terrain editing is facilitated with different brushes (sculpt, smooth, paint layers). Each terrain can support up to eight material layers, with each layer capable of enforcing the type of foliage that can spawn on it.

![Terrain](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Terrain.png)

## Debugging

Multiple additional debugging windows are available for users to toggle:

![Debug Render Targets](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Debug%20Render%20Targets.png)

Users can edit shaders on-the-fly in the built-in shader editor and debug compilation errors:

![Shader Compilation errors](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Shader%20Compilation%20errors.png)

In addition, the editor supports real-time retrieval of shader variable values from the GPU. (Please note that not all variables are supported and further testing is needed for this feature.)

![Shader values read back](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Shader%20values%20read%20back.png)

The Log window will display chosen categories of warnings, errors, and info.

![Log window](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Log%20window.png)

## Building the Project for Visual Studio (Windows)

```bash
# Initialize a new Git repository
git init

# Add the remote repository
git remote add origin https://github.com/Azzinoth/FocalEngineEditor

# Pull the contents of the remote repository
git pull origin master

# Initialize and update submodules
git submodule update --init --recursive

# Generate the build files using CMake
# Will work in Windows PowerShell
cmake CMakeLists.txt
```

## Third Party Licenses

This project uses the following third-party libraries:

1) **ImGuiColorTextEdit**: This library is licensed under the MIT License. The full license text can be found at [ImGuiColorTextEdit's GitHub repository](https://github.com/BalazsJako/ImGuiColorTextEdit/blob/master/LICENSE).
