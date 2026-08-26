# Creation Developer

## Scope

Creation Developer is the Creation Suite development environment. It hosts
FRust authoring and Suite plugin-development workflows without duplicating
shared Suite infrastructure.

## Local Rules

- Consume shared UI, services, project, and plugin-host contracts from the
  Creation Suite; do not copy them into this repository.
- Keep FRust-specific integration behind app-owned adapters so the standalone
  FRust IDE remains independently usable.
- Treat plugin capabilities and host APIs as versioned contracts.
- Build on D: drive only. Do not invoke vcpkg or rebuild LLVM.

