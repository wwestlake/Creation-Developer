# Creation Developer

Creation Developer is the Creation Suite development environment for FRust
projects and Suite plugins.

## Initial Scope

- shared Suite shell, settings, project, and AI communications
- FRust workspace integration seam
- Creation Suite plugin-host integration seam
- versioned release packaging for Windows

The standalone FRust IDE remains the source editor during the migration. This
app will consume its capabilities through explicit integration layers rather
than fork the editor.

## Build

```powershell
$env:JUCE_DIR = "D:\JUCE2\JUCE"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --target CreationDeveloperApp
```

