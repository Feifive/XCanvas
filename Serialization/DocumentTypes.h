#ifndef XCANVAS_SERIALIZATION_DOCUMENTTYPES_H
#define XCANVAS_SERIALIZATION_DOCUMENTTYPES_H

#include <QString>

namespace xcanvas::serialization
{
// File extension used by XCanvas document files.
inline constexpr const char* kDocumentExtension = ".xcanvas";
inline constexpr const char* kCompressedHeaderZstd  = "XCANVASZSTD1\n";
inline constexpr const char* kPlainSaveEnvKey   = "XCANVAS_SAVE_PLAIN";
inline constexpr const char* kZstdLevelEnvKey   = "XCANVAS_ZSTD_LEVEL";
inline constexpr const char* kZstdThreadsEnvKey = "XCANVAS_ZSTD_THREADS";

// Top-level schema version for JSON document format.
inline constexpr int kDocumentVersion = 1;

// Common top-level keys.
inline constexpr const char* kKeyVersion = "version";
inline constexpr const char* kKeyCanvas  = "canvas";
inline constexpr const char* kKeyLayers  = "layers";
inline constexpr const char* kKeyShapes  = "shapes";

// Optional metadata keys.
inline constexpr const char* kKeyMeta = "meta";
inline constexpr const char* kKeyApp  = "app";
inline constexpr const char* kKeyTime = "savedAt";
}// namespace xcanvas::serialization

#endif// XCANVAS_SERIALIZATION_DOCUMENTTYPES_H
