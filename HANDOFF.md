# HANDOFF — feature/vr-scope (live cursor)

**Next action:** implement spec **§3.1** (Phase 1, three-view SFS). Work it line by line from `docs/design/vr-scope-checklist.md`. Read only §1, §2 and §3.1 of `docs/design/vr-scope-spec.md` before starting.

## State
- Repo: `D:\ab\argent`, on branch `feature/vr-scope` (off fork `main` @ `df1dc29`).
- Remote `origin`: https://github.com/jakyle/KHARVOX-ARGENT.
- Nothing has been implemented yet. Only the docs exist: the spec, the checklist, this file, and `.claude/napkin.md`.
- Untracked, intentionally (gitignored):
  - `releases/`
  - `third-party/bhaptics/bhaptics_library.dll`, a proprietary copy taken from the official 1.01a install.
- `third-party/spirv-cross` shows as modified. It is a working-tree/line-ending artifact; don't commit it.

## Build / deploy
- Build: run `D:\ab\build_argent.cmd`. It reuses the CMake tree at `D:\ab\ab` and glslang at `D:\ab\glslang-install`, and outputs to `D:\ab\ab\Release`.
- Unit tests: compile them directly with cl, following the pattern in `.claude/napkin.md`. Alternatively, add `--target <name>_tests` to the build.
- Dev install: `C:\Games\KHARVOX-ARGENT\ARGENT-scope-dev`. Create it from `D:\ab\argent\releases\ARGENT-1.01.1` on first deploy, then copy the user's customized `assets\*.cfg` from `C:\Games\KHARVOX-ARGENT\ARGENT-1.01a\assets\`. After that, deploy by copying `D:\ab\ab\Release\ArgentLayer.dll` (plus `ArgentLauncher.exe` if it changed).
- **Never touch `C:\Games\KHARVOX-ARGENT\ARGENT-1.01a`.** It is the user's working install.

## Load-bearing facts
- The projection contract allows only parallel views, with x/y offset and asymmetric FOV (`src/sfs/FrameProjection.h:19-50`). Don't rotate views.
- Stereo works by shader rewrite through `clipFromCenter` / `eyeTranslation` (`src/sfs/StereoSource.h:90-102`).
- Uniforms are computed at `src/ArgentLayer.cpp:437`.
- Zoom hooks and the Precision Bolt detection live in `src/EternalPlayerHooks.cpp:201-271`.
- The pure policy is in `src/WeaponZoomPolicy.h`.

## Verification
- `[coded]` means it builds and the unit tests are green.
- `[verified]` means the user confirmed it in the headset (Steam Frame over SteamVR, RTX 5090). Ask the user to test at each phase gate.
