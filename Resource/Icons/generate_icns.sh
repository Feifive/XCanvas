#!/bin/bash
# ==============================================
# 自动生成 macOS 图标 .icns 文件（适用于 Qt 项目）
# 用法: ./generate_icns.sh app_icon.png
# 输出: icons/app.icns
# ==============================================

set -e

# 输入参数检查
if [ $# -lt 1 ]; then
    echo "❌ 用法: $0 <源PNG文件>"
    exit 1
fi

SRC_IMAGE="$1"
ICONSET_DIR="app.iconset"
OUTPUT_DIR="icons"
OUTPUT_ICNS="$OUTPUT_DIR/XCanvasLogo.icns"

# 检查依赖
command -v sips >/dev/null 2>&1 || { echo "❌ sips 未找到，请确保你在 macOS 环境下运行"; exit 1; }
command -v iconutil >/dev/null 2>&1 || { echo "❌ iconutil 未找到"; exit 1; }

# 创建输出目录
mkdir -p "$ICONSET_DIR"
mkdir -p "$OUTPUT_DIR"

# 生成各尺寸图标
echo "🔧 生成 iconset..."
sips -z 16 16     "$SRC_IMAGE" --out "$ICONSET_DIR/icon_16x16.png"
sips -z 32 32     "$SRC_IMAGE" --out "$ICONSET_DIR/icon_16x16@2x.png"
sips -z 32 32     "$SRC_IMAGE" --out "$ICONSET_DIR/icon_32x32.png"
sips -z 64 64     "$SRC_IMAGE" --out "$ICONSET_DIR/icon_32x32@2x.png"
sips -z 128 128   "$SRC_IMAGE" --out "$ICONSET_DIR/icon_128x128.png"
sips -z 256 256   "$SRC_IMAGE" --out "$ICONSET_DIR/icon_128x128@2x.png"
sips -z 256 256   "$SRC_IMAGE" --out "$ICONSET_DIR/icon_256x256.png"
sips -z 512 512   "$SRC_IMAGE" --out "$ICONSET_DIR/icon_256x256@2x.png"
sips -z 512 512   "$SRC_IMAGE" --out "$ICONSET_DIR/icon_512x512.png"
cp "$SRC_IMAGE" "$ICONSET_DIR/icon_512x512@2x.png"

# 生成 icns 文件
echo "🎨 打包为 XCanvasLogo.icns..."
iconutil -c icns "$ICONSET_DIR" -o "$OUTPUT_ICNS"

# 清理中间文件
rm -rf "$ICONSET_DIR"

echo "✅ 图标生成完成: $OUTPUT_ICNS"