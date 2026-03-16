# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and run commands

- This is a CMake + Qt Widgets desktop application in C++17 with `CMAKE_AUTOUIC`, `CMAKE_AUTOMOC`, and `CMAKE_AUTORCC` enabled.
- Configure the existing debug build tree:
  ```bash
  cmake -S /Users/ze/Desktop/QtProjects/XCanvas -B /Users/ze/Desktop/QtProjects/XCanvas/cmake-build-debug
  ```
- Build the debug `XCanvas` target:
  ```bash
  cmake --build /Users/ze/Desktop/QtProjects/XCanvas/cmake-build-debug --target XCanvas
  ```
- Configure the existing release build tree:
  ```bash
  cmake -S /Users/ze/Desktop/QtProjects/XCanvas -B /Users/ze/Desktop/QtProjects/XCanvas/cmake-build-release -DCMAKE_BUILD_TYPE=Release
  ```
- Build the release `XCanvas` target:
  ```bash
  cmake --build /Users/ze/Desktop/QtProjects/XCanvas/cmake-build-release --target XCanvas --config Release
  ```
- Run the macOS debug app bundle:
  ```bash
  open /Users/ze/Desktop/QtProjects/XCanvas/Bin/mac/Debug/XCanvas.app
  ```
- Run the macOS release app bundle:
  ```bash
  open /Users/ze/Desktop/QtProjects/XCanvas/Bin/mac/Release/XCanvas.app
  ```
- `CMakeLists.txt` hardcodes `QT_SDK_DIR` with `FORCE` (`/Users/ze/Qt/6.10.1/macos` on Apple, `D:/software/Qt6.8.3/6.8.3/msvc2022_64` on Windows), so that path is machine-specific and may need to be edited for other environments.

## Testing / linting status

- No project-level test suite, `enable_testing()`, `include(CTest)`, `add_test(...)`, or single-test command was found in the main repo.
- No top-level lint or format command was found in the main repo.
- The root CMake also explicitly disables bundled zstd tests.

## Dependency and setup notes

- Fresh checkouts should initialize submodules, especially `3rdparty/Qt-Fluent-Widgets` from `.gitmodules`.
- Bundled third-party dependencies wired through CMake include `pdfium`, `Clipper2`, `zstd`, and imported static `libdxfrw` from `SDK/lib/...`.
- The native project document format uses the `.xcanvas` extension.

## Architecture overview

- Main startup flow is `main.cpp` -> `MainWindow` -> `EditorPageManager` -> per-tab `CanvasWidget` -> `MyGraphicsView`.
- `MainWindow` creates the tab/page shell and delegates tab lifecycle to `EditorPageManager`.
- `EditorPageManager` builds each editor page from an `EditorSession`, `DrawingToolsBar`, `CanvasWidget`, and `LayerPanel`.
- `CanvasWidget` hosts the `QGraphicsScene`, rulers, and the per-document `MyGraphicsView`.
- `MyGraphicsView` is the page-level composition root and main integration point for controllers, tool management, document lifecycle, rendering, clipboard flows, and file import/open/save behavior.
- `xcanvas::Canvas` is the in-memory document model. It owns the document `ShapeManager`, `LayerManager`, and `QUndoStack`.
- Rendering is custom/manual through `ViewRenderController`, which paints canvas background, grid, normal shapes, selection overlays, and handles directly from model data instead of relying on a `QGraphicsItem` scene graph architecture.
- Document lifecycle is split between `DocumentSessionController` and `DocumentIoController`: the session controller handles dirty-state prompts, save/open decisions, current-path tracking, and title updates; the IO controller performs async/blocking open-save work and import execution. Serialization logic lives under `Serialization/`.
- Import is extensible through `IFileImporter` and `ImportManager`; `MyGraphicsView` registers DXF, image, and PDF importers during startup.
- Layers are not just visual order: `LayerManager` stores per-layer process metadata including visibility/output flags, cut/scan/image mode, speed, and power settings.

## Repository map

- `Canvas/` — core document model types, including `xcanvas::Canvas` and shape ownership/undo integration.
- `Shape/` — shape primitives, editing commands, geometry, and drawing behavior.
- `DrawingTool/` — interactive drawing/editing tools coordinated by the tool manager.
- `Controller/` — view, interaction, selection, document session, and document IO controllers.
- `Layer/` — layer data structures, ordering, parameters, and layer panel UI.
- `Import/` — importer interfaces, registry, and concrete DXF/image/PDF importers.
- `Serialization/` — `.xcanvas` document schema constants and load/save serialization code.
- `Common/` — shared application utilities and reusable UI infrastructure.
