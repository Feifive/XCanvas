# XCanvas CI/CD

本项目使用 GitHub Actions 做持续集成和持续交付，工作流文件位于：

- `.github/workflows/ci-cd.yml`

## 当前行为

- 向 `main` 分支提交代码时自动构建
- 对 `main` 发起 Pull Request 时自动构建
- 手动触发 `workflow_dispatch` 时可手工执行流水线
- 推送 `v*` 标签时自动创建 GitHub Release

## 构建平台

- macOS
- Windows

## CI 阶段

每次 PR / Push 到 `main` 时会执行：

1. 拉取仓库及子模块
2. 安装 Qt 6.8.3
3. 使用 CMake 配置 Release 构建
4. 编译 `XCanvas`
5. 自动打包构建产物
6. 上传 Actions Artifacts

## 打包方式

### macOS

- 使用 `macdeployqt` 收集 Qt 运行时依赖
- 输出 `XCanvas.app`
- 再打成 zip 包上传

### Windows

- 使用 `windeployqt` 收集 Qt 运行时依赖
- 将可执行文件和依赖目录打成 zip 包上传

## Release 阶段

当推送标签，例如：

```bash
git tag v0.1.0
git push origin v0.1.0
```

流水线会：

1. 先完成 macOS / Windows 构建
2. 下载两个平台的打包产物
3. 自动创建 GitHub Release
4. 自动上传 zip 附件
5. 自动生成 Release Notes

## 产物命名

当前产物命名格式：

- `XCanvas-macOS-<ref>.zip`
- `XCanvas-windows-<ref>.zip`

示例：

- `XCanvas-macOS-v0.1.0.zip`
- `XCanvas-windows-v0.1.0.zip`

如果是普通分支构建，`<ref>` 会是分支名；如果是发版，则会是标签名。

## CMake / Qt 约定

为了同时兼容本地开发和 CI，`CMakeLists.txt` 现在按以下优先级查找 Qt：

1. CMake 变量 `QT_SDK_DIR`
2. 环境变量 `QT_ROOT_DIR`
3. 项目里原有的本地默认路径

也就是说：

- 本地开发可以继续使用你现有 Qt 安装目录
- CI 会自动把 `install-qt-action` 提供的 `QT_ROOT_DIR` 传给 CMake

## 注意事项

- 当前仓库没有项目级测试，所以这套 CI 目前主要覆盖“能否成功构建和打包”
- `3rdparty/Qt-Fluent-Widgets` 和 `3rdparty/spdlog` 通过 git submodule 拉取，因此 checkout 必须启用 `submodules: recursive`
- `pdfium` 和 `libdxfrw` 当前仍依赖仓库内已有的二进制文件

## 后续建议

后续可以继续增强：

- 增加单元测试并接入 CI
- 增加代码格式检查
- 增加草稿版 Release / 预发布 Release
- 为不同平台补充更正式的安装包格式，例如 `dmg` 或安装程序
