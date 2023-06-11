# Focal Engine Editor



## Features


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