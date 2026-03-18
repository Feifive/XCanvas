# Third-Party Notices

This project includes or links against third-party software under their respective licenses.
The original license texts remain in each dependency directory. This file is a convenience summary only.

## Third-party components

### Clipper2

- Path: `3rdparty/Clipper2`
- Upstream license file: `3rdparty/Clipper2/LICENSE`
- License: Boost Software License 1.0
- Notes: Used as a git submodule and built from source by CMake.

### zstd

- Path: `3rdparty/zstd`
- Upstream license files:
  - `3rdparty/zstd/LICENSE`
  - `3rdparty/zstd/COPYING`
- License: dual-licensed BSD OR GPLv2
- Notes: This repository's README states zstd is available under BSD or GPLv2. If you distribute this project, you should document which option your distribution relies on.

### libdxfrw

- Path: `3rdparty/libdxfrw`
- Upstream license files:
  - `3rdparty/libdxfrw/COPYING`
  - `3rdparty/libdxfrw/README.md`
- License: GNU General Public License v2 or later
- Notes: The upstream README states libdxfrw is licensed under GPL v2 or, at your option, any later version. This may impose copyleft obligations on distributions of this project.

### spdlog

- Path: `3rdparty/spdlog`
- Upstream license file: `3rdparty/spdlog/LICENSE`
- License: MIT
- Notes: The upstream license file also notes that spdlog depends on fmt, which is also under MIT.

### PDFium

- Path: `3rdparty/pdfium`
- Upstream license file: `3rdparty/pdfium/LICENSE`
- Additional bundled notices directory: `3rdparty/pdfium/licenses`
- License: MIT-style permissive license for PDFium itself, plus additional third-party licenses shipped in the `licenses/` directory
- Notes: If you redistribute PDFium binaries or package this application, include the relevant notices from `3rdparty/pdfium/licenses`.

## Project status

- XCanvas itself is licensed under `GPL-2.0-or-later`. See `LICENSE`.
- This file does not replace legal review.
- When publishing binaries or source distributions, keep the corresponding third-party license texts and PDFium bundled notices.
