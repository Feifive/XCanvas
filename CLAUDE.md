# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and run commands

- This is a CMake + Qt Widgets desktop application in C++17 with `CMAKE_AUTOUIC`, `CMAKE_AUTOMOC`, and `CMAKE_AUTORCC` enabled.
- Unless there is a specific reason not to, use `-j 24` for local `cmake --build` invocations in this repository.
- Configure the existing debug build tree:
  ```bash
  cmake -S /Users/ze/Desktop/QtProjects/XCanvas -B /Users/ze/Desktop/QtProjects/XCanvas/cmake-build-debug
  ```
- Build the debug `XCanvas` target:
  ```bash
  cmake --build /Users/ze/Desktop/QtProjects/XCanvas/cmake-build-debug --target XCanvas -j 24
  ```
- Configure the existing release build tree:
  ```bash
  cmake -S /Users/ze/Desktop/QtProjects/XCanvas -B /Users/ze/Desktop/QtProjects/XCanvas/cmake-build-release -DCMAKE_BUILD_TYPE=Release
  ```
- Build the release `XCanvas` target:
  ```bash
  cmake --build /Users/ze/Desktop/QtProjects/XCanvas/cmake-build-release --target XCanvas --config Release -j 24
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

- Fresh checkouts should initialize submodules, especially `3rdparty/Qt-Fluent-Widgets`, `3rdparty/spdlog`, and `3rdparty/Clipper2` from `.gitmodules`.
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
- Application-wide diagnostics are handled by the centralized logging subsystem in `Common/AppLogging.*` and `Common/LogMacros.h`, which is initialized from `main.cpp` and intended to replace scattered `qDebug()` usage over time.

## Logging system

- The app is migrating from ad hoc `qDebug()` / `qWarning()` usage to a centralized `spdlog`-based logging system rooted in `Common/AppLogging.h` and `Common/AppLogging.cpp`.
- Logging is initialized early in `main.cpp` via `AppLogging::initialize()` and shut down after the Qt event loop exits via `AppLogging::shutdown()`.
- The root logger is named `xcanvas`; module loggers are created as `xcanvas.<module>` through `AppLogging::logger(...)` and `AppLogging::normalizeModuleName(...)`.
- Log output is written to `QStandardPaths::AppConfigLocation/Log/xcanvas.log` using a daily rotating file sink with 30 retained files. Debug builds also add a colored console sink.
- If file logger initialization fails, the app falls back to a stderr color logger instead of aborting startup.
- Qt's message pipeline is bridged into the same logger with `qInstallMessageHandler(...)`, so remaining `qDebug()` / `qWarning()` calls still flow into the central log output.
- Log detail mode is controlled by `XCANVAS_LOG_DETAIL`:
  - compact: normal message-first output
  - verbose: includes thread id in sink output plus file, line, and function context injected by macros
- The main logging call surface is `Common/LogMacros.h`, which provides:
  - generic macros: `XC_LOG_TRACE/DEBUG/INFO/WARN/ERROR/CRITICAL(module, fmt, ...)`
  - scope timing macro: `XC_SCOPE_LOG(module, name)`
  - convenience module macros such as `DXF_LOG_*`, `CANVAS_LOG_*`, `SETTINGS_LOG_*`, `DEVICE_LOG_*`, and `DOCUMENT_IO_LOG_*`
- `XC_SCOPE_LOG(...)` uses `xc_log::ScopeTimer` to emit entry/exit timing logs like `>>> name` and `<<< name (N ms)`.
- `Common/QtFmtSupport.h` provides `fmt::formatter<>` support for selected Qt types currently including `QString`, `QByteArray`, `QPointF`, and `QRectF`, enabling `spdlog`/`fmt`-style formatting with `{}` placeholders.
- Early migration examples include:
  - `Common/AppSettings.cpp` for settings/config path logging
  - `Import/DXF/DXFTranslator.cpp` for structured DXF import tracing
  - `Serialization/DocumentIO.cpp` for document IO diagnostics
- When adding new logs, prefer module macros and fmt-style formatting over stream-style Qt logging.

## Repository map

- `Canvas/` — core document model types, including `xcanvas::Canvas` and shape ownership/undo integration.
- `Shape/` — shape primitives, editing commands, geometry, and drawing behavior.
- `DrawingTool/` — interactive drawing/editing tools coordinated by the tool manager.
- `Controller/` — view, interaction, selection, document session, and document IO controllers.
- `Layer/` — layer data structures, ordering, parameters, and layer panel UI.
- `Import/` — importer interfaces, registry, and concrete DXF/image/PDF importers.
- `Serialization/` — `.xcanvas` document schema constants and load/save serialization code.
- `Common/` — shared application utilities, reusable UI infrastructure, and cross-cutting logging infrastructure such as `AppLogging`, `LogMacros`, and `QtFmtSupport`.
