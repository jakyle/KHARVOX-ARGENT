# Napkin — ARGENT fork mistakes and lessons

### 2026-10-05 — MSVC fails on long build paths
- **What went wrong:** building glslang/SPIRV-Tools under `%TEMP%\claude\...\scratchpad` failed with `C1083: Cannot open compiler generated file: ''`.
- **Correction:** build under a short path (`D:\ab\...`).
- **Durable lesson:** keep all build trees in `D:\ab`.

### 2026-10-05 — Repo was missing a packaging asset
- **What went wrong:** `tools/package_runtime.ps1` needs `assets/argent_controls.cfg.support`, which wasn't committed upstream.
- **Correction:** copied it from the official 1.01a package (commit `df1dc29`).
- **Durable lesson:** `package_release.ps1` refuses existing release names. Rename a failed attempt's folder instead of deleting it.

### 2026-10-05 — Generator and toolchain
- Only VS 2022 BuildTools is installed: use the `Visual Studio 17 2022` generator with CRT `VC\Redist\MSVC\14.44.35112\x64\Microsoft.VC143.CRT`.
- CMake is VS-bundled: `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`.

### 2026-10-05 — Quick unit-test build
Run under `vcvars64.bat`, from `D:\ab\argent`:
```
cl /nologo /std:c++17 /EHsc /W4 /I third-party\openxr-sdk\include tests\X.cpp /Fe:<out>\X.exe
```
Vulkan-dependent sources also need:
```
/DWIN32_LEAN_AND_MEAN /DNOMINMAX /DVK_NO_PROTOTYPES /I third-party\vulkan-headers\include /I third-party\cgltf /I src
```

### 2026-10-05 — Windows macros
- `far` (and `near`) are Windows macros. Don't use them as variable names in headers.

### 2026-10-05 — Comment hook
- A user hook blocks multi-line `//` comment blocks in code. Keep code comments to single lines.

### 2026-10-05 — CRLF sources defeat PowerShell string Replace
- **What went wrong:** multi-line `.Replace()` edits silently missed in CRLF files.
- **Correction:** use the Edit tool, or normalize to LF, replace, and restore CRLF.
- **Durable lesson:** after any scripted replace, print a MISSING line for each pattern that didn't match.

### 2026-10-05 — Multiview occlusion can't be split per view
- **What went wrong:** summed only the eye slots so the scope wouldn't change occlusion counts. The GPU test then got 24, not 16.
- **Correction:** sum every view slot. Vulkan may distribute the count across view slots arbitrarily.
- **Durable lesson:** an extra multiview view always inflates game occlusion counts.

### 2026-10-05 — Pre-existing test failures
- `captures/` isn't in the repo, so 6 capture-fixture tests fail (eternal_volumes, light_grid, vk3d, amd_light_grid, amd_world, world_variants).
- `eternal_build_profile` and `sfs_portable_ui` also fail on base `d8ceb1b`.
- Direct-compile GPU tests (bindless, sampling, water_robustness) use 2-layer fixtures, so they pin `options.views=kEyeViews`.
