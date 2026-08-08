# XCanvas 仓库协作指南

## 项目概览

XCanvas 是使用 C++17、Qt Widgets 和 CMake 构建的桌面绘图应用，原生文档扩展名为 `.xcanvas`。程序主链路为 `App/main.cpp` → `MainWindow` → `EditorPageManager` → 每个标签页的 `MyCanvasView` → `xcanvas::CanvasView`。

核心目录职责：

- `App/`：程序入口、主窗口、编辑器页面管理和画布页面组合根。
- `Widgets/`：工具栏、悬浮菜单、颜色选择和选择状态等可复用界面控件。
- `Canvas/`：文档模型，以及自有画布视口、相机和标尺渲染设施。
- `Shape/`：图形、几何、编辑命令和撤销/重做语义。
- `DrawingTool/`：交互式绘图、选择和编辑工具。
- `Controller/`：渲染、交互、布局、文档会话与文档 IO 的协调逻辑。
- `Layer/`：图层数据、顺序、可见性、加工参数和面板 UI。
- `Import/`、`Serialization/`：外部文件导入和 `.xcanvas` 持久化。
- `Common/`：日志、设置和通用 UI/基础设施；`tests/`：Qt Test 测试。

## 架构边界

- `xcanvas::Canvas` 是文档模型和 `ShapeManager`、`LayerManager`、`QUndoStack` 的所有者。视图层不得复制或绕开这些业务状态。
- `MyCanvasView` 是每个文档页面的画布组件，也是 Controller、工具、渲染、剪贴板和文档操作的页面组合根；它直接作为 `EditorPageManager` 中间分栏的视图，继承基于 `QAbstractScrollArea` 的 `xcanvas::CanvasView`，而不是 `QGraphicsView`。
- `xcanvas::CanvasView` 持有 `CanvasCamera`，负责 viewport 绘制调度、滚动条同步和世界/视口坐标映射；`RulerRenderer` 是 viewport 内最后绘制的半透明覆盖层，不是独立控件。
- 图形由模型数据经 `QPainter` 手工渲染；不要把业务图形迁移为 `QGraphicsItem` 场景图。
- 坐标映射、缩放和平移应通过 `ICanvasViewport`/`ICanvasNavigation` 与 `CanvasCamera` 等项目接口表达。Controller 和工具不应新增对具体视图类或 Qt Graphics View API 的依赖。
- 文档提示、当前路径、dirty 状态、标题更新和保存/打开决策属于 `DocumentSessionController`；异步或阻塞文件读写和导入执行属于 `DocumentIoController`；`.xcanvas` 格式和读写语义留在 `Serialization/`。
- 外部导入通过 `IFileImporter` 和 `ImportManager` 扩展；`MyCanvasView` 启动时注册 DXF、图片和 PDF importer，不要把解析逻辑塞入页面或文档会话 Controller。
- `LayerManager` 除视觉顺序外还保存可见/输出标记、Cut/Scan/Image 模式、速度和功率等加工参数；视图重构不得丢失这些语义。

## 构建与测试

首次检出先初始化子模块：

```powershell
git submodule update --init --recursive
```

重点子模块包括 `Qt-Fluent-Widgets`、`spdlog`、`Clipper2`、`zstd` 和 `libdxfrw`；CMake 还会构建仓库内的 `pdfium`。不要直接修改或清理第三方目录中的本地状态。

Windows **只接受 MSVC 编译器，明确拒绝 MinGW 及其他非 MSVC 编译器**。根 CMake 的配置检查执行这一编译器家族限制，但不单独强制具体 MSVC 版本或 CPU 架构。MSVC 2022 x64 是当前推荐且经过验证的配置；仓库 preset 使用 Ninja 生成器，需从 Visual Studio Developer PowerShell/Command Prompt 运行，或由 Qt Creator 的 MSVC Kit 提供编译环境。`QT_SDK_DIR` 必须指向与编译器匹配的 MSVC Qt SDK：

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-msvc-debug
cmake --build --preset windows-msvc-tests-debug
ctest --preset windows-msvc-tests-debug

# Release application
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
```

个人 Qt SDK 和本机构建树路径只记录在 Git 忽略的 `CMakeUserPresets.json` 中；本机 preset 应继承仓库 preset。也可通过 `QT_ROOT_DIR` 环境变量提供 Qt SDK。日常应用构建使用 `local-windows-msvc-debug`，测试构建和运行使用 `local-windows-msvc-tests-debug`，发布构建使用 `local-windows-msvc-release`。

macOS 可使用对应 Qt SDK：

```bash
cmake --preset macos
cmake --build --preset macos-debug
ctest --preset macos-debug
```

改变生成器、编译器、Qt Kit 或测试结构时使用新的构建目录，不要复用不兼容的 CMake 缓存。Windows Ninja preset 默认使用 12 个并发任务；不要在日常应用构建中附带构建全部测试。

## Qt、CMake 与编码约束

- 保持 C++17、`CMAKE_AUTOUIC`、`CMAKE_AUTOMOC` 和 `CMAKE_AUTORCC` 设置；新增 `QObject` 类、资源、源文件或测试时同步 CMake。
- Windows 配置必须通过根 CMake 的 MSVC 检查；不要弱化、规避或为 MinGW/其他非 MSVC 编译器添加兼容分支。MSVC 2022 x64 是推荐和验收基线，不要把文档写成 CMake 正在校验唯一 MSVC 版本或架构。
- 不要硬编码新的个人路径。Qt 查找优先使用 `QT_SDK_DIR`，其次使用 `QT_ROOT_DIR` 环境变量。
- 遵循根目录 `.clang-format`：4 空格、禁用 Tab、Allman 大括号和现有命名风格。仅格式化本次修改涉及的代码，避免制造全文件噪声。
- 尊重 Qt 对象所有权和信号/槽生命周期；跨异步边界不得捕获可能失效的 QWidget/文档对象。
- 新日志优先使用现有模块宏；不要记录敏感数据或在高频绘制/鼠标移动路径无节制输出日志。

## 日志系统

- `Common/AppLogging.*` 是集中式 `spdlog` 基础设施；`main.cpp` 在事件循环前调用 `AppLogging::initialize()`，退出后调用 `AppLogging::shutdown()`。Qt 消息通过已安装的 message handler 汇入同一日志系统。
- 日志写入 `QStandardPaths::AppConfigLocation/Log/xcanvas.log`，按日轮转并保留 30 个文件；Debug 构建同时输出彩色控制台，文件 sink 初始化失败时回退到 stderr，日志失败不得阻止程序启动。
- 调用面使用 `Common/LogMacros.h` 的 `XC_LOG_*`、`XC_SCOPE_LOG` 和现有模块便捷宏，并使用 fmt `{}` 占位符。`Common/QtFmtSupport.h` 已支持若干 Qt 类型；需要新类型时优先补 formatter，而不是退回流式 `qDebug()`。
- `XCANVAS_LOG_DETAIL` 控制 compact/verbose 细节；禁止记录敏感数据，也不要在绘制、鼠标移动等高频路径无节制输出。

## 已知编译问题与解决方法

- 普通 PowerShell 直接调用 MSVC 时，`cl.exe` 可能找不到 `type_traits` 等标准库头。这是 Visual Studio 开发环境未初始化；改用 Developer PowerShell、VS Developer Command Prompt 或 Qt Creator 的 MSVC Kit。
- MSVC 中文诊断可能被 Qt Creator 错误解码，Ninja 也可能无法过滤本地化的 `/showIncludes` 输出。Windows preset 固定 `VSLANG=1033`，让诊断采用英文并保持依赖扫描输出简洁。
- Visual Studio 生成器构建 Clipper2 时若报 C4530，随后因 `/WX` 转为 C2220，表示未启用 C++ 异常展开语义；Windows MSVC preset 已通过 `CMAKE_CXX_FLAGS=/EHsc` 处理。手工配置时也必须为 C++ 目标启用 `/EHsc`，并在新的构建目录重新 configure。
- MinGW 链接 Qt-Fluent-Widgets 时可能报 `DwmGetWindowAttribute`、`DwmSetWindowAttribute` 等 `undefined reference`。正式解决方法是切换到 MSVC 和与之匹配的 Qt SDK（推荐 MSVC 2022 x64），并新建构建目录；根 CMake 会主动拒绝 MinGW。
- 旧构建树中的 CTest 可能仍引用已经不存在的 `tests/*.cmake`。重新运行 configure；若生成器或测试结构变化，直接新建构建目录。
- CTest 返回 `0xc0000135` 通常表示缺少 Qt DLL。当前 `tests/RunQtTest.cmake` 会注入 Qt runtime，重新 configure 后再运行 CTest；直接运行测试 exe 时，需确保对应 Qt `bin` 在 `PATH`。
- Qt 查找失败或混用 Kit 时，在 `CMakeUserPresets.json` 中将 `QT_SDK_DIR` 指向实际的 MSVC 版 Qt 根目录，或设置 `QT_ROOT_DIR`；修改 Kit 后使用新的构建目录重新 configure。

## 用户改动保护

- 开始前查看 `git status` 和相关 diff。工作树可能有用户或其他代理的未提交改动；它们都应视为需保留内容。
- 只修改任务要求的文件，不回滚、不覆盖、不顺手格式化无关代码；发现重叠时先理解并最小化合并。
- 不使用 `git reset --hard`、`git checkout --` 等破坏性命令，不删除现有构建产物或用户文件，除非用户明确授权且目标已核实。
- 未经要求不要提交、推送或改写历史；第三方子模块中的本地状态也不要擅自清理。

## 验收要求

- 至少完成一次与改动相关的全新目录 configure、目标构建和 `ctest --output-on-failure`；Windows 必须使用 MSVC，推荐并以已验证的 MSVC 2022 x64 配置作为验收基线。无法运行的平台或手工 UI 项目必须如实说明。
- 画布改动除自动测试外，还应按重构文档检查缩放锚点、平移/fit、渲染层次、选择与文本输入、剪贴板/拖放、标尺/HUD、多标签文档、保存提示和高 DPI。
- 验收报告列出修改文件、执行的命令及结果、未验证项和已知风险。不得把仅 configure 成功描述为完整构建通过。
- 交付前再次检查 diff，确认没有无关文件、调试输出、个人绝对路径或意外生成物。
