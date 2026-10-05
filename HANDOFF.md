# HANDOFF — feature/vr-scope (live cursor)

**Next action:** wait for the user's Phase 1 headset gate (§3.1.4). On pass: tick the `[verified]` lines and timings in `docs/design/vr-scope-checklist.md`, open a PR into fork `main`, then start §3.2. On failure: debug inline.

## State
- Repo: `D:\ab\argent`, branch `feature/vr-scope`. Phase 1 committed and pushed (see `git log main..feature/vr-scope`).
- Phase 1 is `[coded]`, not `[verified]`.
- Untracked, intentionally (gitignored): `releases/`, `third-party/bhaptics/bhaptics_library.dll`.
- `third-party/spirv-cross` shows as modified. It is a line-ending artifact; don't commit it.

## Phase 1 facts (what bites)
- View count: `src/sfs/ViewCount.h` (`kViews=3`, `kScopeView=2`, `supportedViews`, `querySlot`, `occlusionSum`).
- The runtime view count is per device: `Configuration::views`, set in `ArgentLayer.cpp` from `maxMultiviewViewCount`/`maxImageArrayLayers`. `ARGENT_SFS_VIEWS=2` forces the 2-view baseline.
- Uniform buffer always holds `kViews` entries (448 bytes). The GLSL block comes from `eyeProjectionBlock()` (`StereoSource.h`).
- `mirrorScopeView()` copies view 0 into view 2 right before `sfs::prepare` (`ArgentLayer.cpp`). Phase 2 replaces this with `ScopeProjection`.
- Occlusion queries sum **all** views: Vulkan may distribute multiview counts across view slots arbitrarily. So the scope view inflates game occlusion counts (e.g. 24 instead of 16).
- DLSS: `EternalDlssHook.cpp` creates one NGX history per view (3), so layer 2 gets an upscaled image.
- The debug mirror: `ARGENT_SCOPE_DEBUG=1` blits the raw source image's layer 2 to the desktop window (`DesktopMirror::present` `sourceLayer`).

## Build / deploy
- Build: `D:\ab\build_argent.cmd` → `D:\ab\ab\Release`. Tests: build all, then `ctest -C Release` in `D:\ab\ab`.
- Dev install: `C:\Games\KHARVOX-ARGENT\ARGENT-scope-dev` (created). To deploy, copy `D:\ab\ab\Release\ArgentLayer.dll` there.
- **Never touch `C:\Games\KHARVOX-ARGENT\ARGENT-1.01a`.**

## Verification
- `[coded]` means it builds and the unit tests are green.
- `[verified]` means the user confirmed it in the headset.
