# XCanvas

XCanvas 是一个基于 Qt Widgets 和 CMake 构建的桌面绘图应用，使用 C++17 开发，当前面向 macOS 和 Windows。

项目支持多种图形绘制与编辑流程，并集成了 DXF、图片、PDF 等导入能力。原生工程文件格式为 `.xcanvas`。

## 技术栈

- C++17
- Qt 6 / Qt Widgets
- CMake
- PDFium
- Clipper2
- zstd
- spdlog

## 目录结构

- `Canvas/`：画布与文档模型
- `Shape/`：图元、命令与几何逻辑
- `DrawingTool/`：绘图工具与交互工具
- `Controller/`：视图、交互、文档生命周期控制器
- `Layer/`：图层系统与图层面板
- `Import/`：DXF、图片、PDF 等导入逻辑
- `Serialization/`：`.xcanvas` 序列化与反序列化
- `Common/`：通用基础设施、设置、日志等
- `3rdparty/`：项目依赖的第三方组件

## 依赖说明

首次拉取仓库后请初始化子模块：

```bash
git submodule update --init --recursive
```

项目当前使用的第三方依赖包括：

- `3rdparty/Qt-Fluent-Widgets`
- `3rdparty/spdlog`
- `3rdparty/pdfium`
- `3rdparty/Clipper2`
- `3rdparty/zstd`
- `3rdparty/libdxfrw`

## 本地构建

### Qt 路径

项目会按以下优先级查找 Qt：

1. CMake 变量 `QT_SDK_DIR`
2. 环境变量 `QT_ROOT_DIR`

仓库不保存个人 Qt 安装路径。可以在环境变量中设置 `QT_ROOT_DIR` 后使用仓库 preset；也可以在已由 Git 忽略的 `CMakeUserPresets.json` 中创建继承仓库 preset 的本机配置，并通过 `QT_SDK_DIR` 记录本机 Qt SDK。

### Windows

Windows 只支持 MSVC，推荐使用 MSVC 2022 x64。仓库 preset 使用 Ninja 生成器；请从 Visual Studio Developer PowerShell/Command Prompt 运行，或在 Qt Creator 中选择 MSVC Kit。日常构建只编译 XCanvas，测试单独构建和运行：

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-msvc-debug
cmake --build --preset windows-msvc-tests-debug
ctest --preset windows-msvc-tests-debug

# Release
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
```

如果使用仓库提供的本机 user preset，则命令为：

```powershell
cmake --preset local-windows-msvc
cmake --build --preset local-windows-msvc-debug
cmake --build --preset local-windows-msvc-tests-debug
ctest --preset local-windows-msvc-tests-debug

# Release
cmake --preset local-windows-msvc-release
cmake --build --preset local-windows-msvc-release
```

### macOS

设置 `QT_ROOT_DIR` 后，可使用通用 macOS preset：

```bash
cmake --preset macos
cmake --build --preset macos-debug
ctest --preset macos-debug
```

构建输出目录默认位于：

- macOS：`Bin/mac`
- Windows：`Bin/win64`

## 运行与打包

项目在构建后会处理部分运行时依赖：

- Windows 下会拷贝 `pdfium.dll`
- macOS 下会将 `libpdfium.dylib` 放入 App Bundle 的 `Frameworks` 目录

CI/CD 中额外使用：

- `macdeployqt` 打包 macOS 应用
- `windeployqt` 打包 Windows 应用

## CI/CD

仓库已经接入 GitHub Actions：

- `push` 到 `main` 自动构建
- `pull_request` 到 `main` 自动构建
- 推送 `v*` 标签自动创建 GitHub Release 并上传产物

详细说明见：

[docs/CI-CD.md](docs/CI-CD.md)

## 日志系统

项目正在逐步从 `qDebug()` 迁移到基于 `spdlog` 的统一日志系统。

- 日志初始化入口：`Common/AppLogging.*`
- 常用日志宏：`Common/LogMacros.h`
- 默认日志文件位置：`QStandardPaths::AppConfigLocation/Log/xcanvas.log`

环境变量 `XCANVAS_LOG_DETAIL` 可切换日志粒度：

- `compact`
- `verbose`

## 当前状态

- 当前仓库包含 `CanvasCamera`、`CanvasViewportContract`、`CanvasView` 和 `TextEditCommit` 四项项目级 CTest
- 当前 CI 仍主要覆盖“可配置、可编译、可打包”；上述 CTest 可通过仓库 test preset 运行
- 仓库中部分平台依赖仍以二进制形式随项目保存

## 许可证

- XCanvas 采用 `GPL-2.0-or-later`
- 根许可证文件见 [`LICENSE`](LICENSE)
- 第三方许可证说明见 [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md)
- 项目当前链接并分发 `libdxfrw`，其上游声明为 `GPL-2.0-or-later`，因此项目整体按 GPL 兼容方式发布
