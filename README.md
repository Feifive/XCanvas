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
- `SDK/lib/...` 中的 `libdxfrw`

## 本地构建

### Qt 路径

项目会按以下优先级查找 Qt：

1. CMake 变量 `QT_SDK_DIR`
2. 环境变量 `QT_ROOT_DIR`
3. `CMakeLists.txt` 中的平台默认路径

如果你的 Qt 安装目录和仓库默认值不同，推荐显式传入：

```bash
cmake -S . -B build -DQT_SDK_DIR=/path/to/Qt/6.x.x/clang_64
```

### macOS

配置 Debug：

```bash
cmake -S . -B cmake-build-debug
```

构建 Debug：

```bash
cmake --build cmake-build-debug --target XCanvas -j 24
```

配置 Release：

```bash
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
```

构建 Release：

```bash
cmake --build cmake-build-release --target XCanvas --config Release -j 24
```

运行 Release 版本：

```bash
open Bin/mac/Release/XCanvas.app
```

### Windows

推荐使用 Qt 6 + MSVC 2022 工具链，通过 CMake 配置并构建 Release：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DQT_SDK_DIR=D:/Qt/6.x.x/msvc2022_64
cmake --build build --config Release --target XCanvas
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

[docs/CI-CD.md](/Users/ze/Desktop/QtProjects/XCanvas/docs/CI-CD.md)

## 日志系统

项目正在逐步从 `qDebug()` 迁移到基于 `spdlog` 的统一日志系统。

- 日志初始化入口：`Common/AppLogging.*`
- 常用日志宏：`Common/LogMacros.h`
- 默认日志文件位置：`QStandardPaths::AppConfigLocation/Log/xcanvas.log`

环境变量 `XCANVAS_LOG_DETAIL` 可切换日志粒度：

- `compact`
- `verbose`

## 当前状态

- 当前仓库没有项目级自动化测试
- 当前 CI 主要覆盖“可配置、可编译、可打包”
- 仓库中部分平台依赖仍以二进制形式随项目保存

## 许可证

如果后续需要对外开源或补充许可证信息，建议在根目录补充单独的 `LICENSE` 文件。
