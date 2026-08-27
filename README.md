# Creation Developer

Creation Developer is the Creation Suite development environment for FRust
projects and Suite plugins.

## Current Scope

- shared Suite shell, settings, Project Manager, and AI communications
- shared active-project session with last-project restore
- docked PowerShell, FRust REPL, and Frate VFS terminal surfaces
- VFS-backed Suite plugin-pod scaffolding through the active project
- Creation Suite plugin-host integration seam
- versioned release packaging for Windows

The standalone FRust IDE remains the source editor while its project/editor
capabilities migrate through explicit integration layers rather than a forked
copy. Creation Developer is the Suite-facing host for FRust plugins: a plugin
pod belongs to the active Suite project and targets either the full Suite or a
specific application.

## Terminals

- **OS Terminal** runs a PowerShell session on the host machine.
- **FRust Terminal** is the `fr->` language REPL.
- **Frate VFS Terminal** is the `frate>` project workspace terminal. Use
  `new pod my_plugin` to create a Suite plugin pod in the active project's VFS
  workspace. Use `help`, `pwd`, `ls`, and `cd` to inspect that workspace.

The terminal surfaces are deliberately separate. Operating-system commands
remain in PowerShell; FRust evaluation remains in the REPL; pod creation and
navigation remain scoped to the active Suite project.

## Build

```powershell
$env:JUCE_DIR = "D:\JUCE2\JUCE"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --target CreationDeveloperApp
```
