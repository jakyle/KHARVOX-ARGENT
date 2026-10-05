# VR Optical Scope — Spec (contract)

**Fork:** https://github.com/jakyle/KHARVOX-ARGENT
**Branch:** `feature/vr-scope`
**Date:** 2026-10-05

This spec is read-only during the build. Track progress in `vr-scope-checklist.md`, with the cursor in `/HANDOFF.md`. Every section has a stable ID (§N.N).

## §0 Goal

Doom Eternal's Heavy Cannon **Precision Bolt** gets a real VR scope: an isolated zoom window on the weapon that shows a magnified, live, separately rasterized view of the world. Everything outside the lens stays at normal view.

- **§0.1** The lens image is a real render: scene geometry rasterized at the zoom FOV, not an upscaled crop of the eye image.
- **§0.2** One scope image per frame, shown to both eyes. The scope is monocular by nature.
- **§0.3** Magnification is the game's own zoom FOV for the weapon (`decl+0x1700`, also returned by `originalZoomFov`).
- **§0.4** The scope is active only while the Precision Bolt zoom is engaged. Other weapons and zoom mods behave exactly as today.
- **§0.5** Performance tricks are part of the feature, not optional extras (§3.5).

## §1 Engine/projection contract (hard constraint, from code)

- **§1.1** SFS stereo works by recompiling every game shader. The game's mono clip position is mapped per view:
  `gl_Position = clipFromCenter[gl_ViewIndex] * clip + eyeTranslation[gl_ViewIndex]`
  - Source: `src/sfs/StereoSource.h:90-102`.
  - Volume/reconstruction shaders use the same terms: `src/sfs/EternalVolumes.h:172-184`.
- **§1.2** In use, these terms are produced only by `parallelEyeProjection` (`src/sfs/EyeProjection.h:32`, called at `src/ArgentLayer.cpp:437`), through `kharvox::sfs::frameProjection` (`src/sfs/FrameProjection.h:19`).
  - Every view must be **parallel** to the game camera: orientation dot ≥ 0.99999.
  - A view may have a **lateral/vertical offset**, but no longitudinal one (|z| ≤ 0.5 mm).
  - A view may have an **asymmetric FOV**.
- **§1.3** Rotated or longitudinally offset views are out of contract. `EyeProjection.h:29-31` says: "rotating only selected clip-space geometry disagrees with Eternal's culling, reconstruction and lighting."
- **§1.4** Therefore the scope camera is defined as follows:
  - parallel to the game camera (head orientation);
  - placed at the scope's lateral/vertical offset from the eye center, with longitudinal offset dropped;
  - using an **off-axis asymmetric frustum** of the zoom FOV, centered on the barrel's aim direction projected into that parallel camera.

  When aiming down the scope, the barrel is within a few degrees of head-forward, so the off-axis skew is negligible.
- **§1.5** The engine culls to the game camera frustum (the headset envelope). Scope content outside it is not drawn. This is acceptable because aiming keeps the scope inside the view. Document it; don't fight it.

## §2 Architecture

```
game frame (mono) ──SFS──► multiview N=3 layers
        layer 0 = left eye
        layer 1 = right eye
        layer 2 = scope view
                    │
OpenXR submit ◄── layers 0/1 (unchanged path)
hand/weapon renderer ──► draws the lens quad on the weapon into each eye,
                         sampling final layer 2 (post-processed)
```

- **§2.1 View count.** Introduce one constant, the SFS view count `kViews=3`. The scope view index is `kScopeView=2`. Every two-view assumption becomes `kViews`-driven.
- **§2.2 Scope uniforms.** `EyeUniforms` / `FrameUniforms` arrays grow from 2 to 3. Update the std140 GLSL block in the shader rewrite and the `static_assert` sizes together.
- **§2.3 Scope camera policy.** A pure function in `src/sfs/ScopeProjection.h`:
  - Inputs: the game source projection, the eye-center pose, the scope pose (tracking space), the zoom FOV in degrees, units per meter, and the sub-rect (§3.5.1).
  - Output: `clip[2]` and `translation[2]`.
  - Returns false (scope view disabled for that frame) on any invalid input.
- **§2.4 Lens renderer.** Extends `src/hands/HandRenderer` with a lens draw:
  - a circular quad at the scope eyepiece, in weapon space;
  - samples layer 2 of the final scene color;
  - adds a procedural reticle (crosshair plus mil dots) and an edge vignette;
  - depth-tested against the private hand depth;
  - never writes engine depth.
- **§2.5 Zoom integration.** `src/WeaponZoomPolicy.h` gains a pure `scopeActive(...)` decision. `EternalPlayerHooks.cpp` exposes the zoom state and the native zoom FOV for the Precision Bolt to the render thread through atomics.

## §3 Phases (each ends with a user in-headset check, then merge gate)

### §3.1 Phase 1 — three-view SFS, inert scope

- **§3.1.1** Generalize every two-view assumption to `kViews`:
  - `StereoResources.h`: `arrayLayers=2`, view `layerCount=2`, `masks_` value 3 becomes 7, `dispatchDepth` multiplier.
  - `SourceRing.h:82`, `QueryResolve.h`, `TimestampQueries.inc`, `WaterGpuCapture.h:240`.
  - `NativeSfs.cpp:738` and the remaining roughly 40 sites. Find them with `rg -n "layers\(.*\)==2|arrayLayers=2|layerCount=2|\[2\]|<2;|\*2\b|\b2u\b" src/sfs`.
  - The correlation mask stays 3: the eyes are correlated, the scope is not.
- **§3.1.2** View 2 uses the **left-eye** projection (`clip[2]=clip[0]`), so the game looks identical. Add a dev-only env flag `ARGENT_SCOPE_DEBUG=1` that copies layer 2 into the desktop mirror (`DesktopMirror.h`) so it can be inspected.
- **§3.1.3** Run the device capability check (`DeviceCapabilities.h:23`) with `maxViews>=3`. If fewer than 3 views are supported, fall back to 2 views with the scope disabled and a log line.
- **§3.1.4** Gate:
  - Existing unit tests pass.
  - New tests cover the `kViews` math (dispatch depth, query slots).
  - In headset: both eyes are unchanged, there are no new artifacts, and the debug mirror shows layer 2.
  - Frame time is logged with and without the third view.

### §3.2 Phase 2 — scope camera

- **§3.2.1** Scope pose source: the weapon laser/muzzle pose already published per rendered frame (`camera::renderedLaser`, used in `src/HandsRuntime.inc`).
  - Fallback is the calibrated weapon pose (`sample.weapon`, `XrActions.inc:231`) plus a per-profile scope offset in `weapon_pose_calibration_default.cfg`.
- **§3.2.2** `ScopeProjection.h` implements §1.4:
  - Project the barrel direction into the parallel scope camera to get the frustum center (tan space).
  - Take the half-angle from the zoom FOV.
  - Fold the sub-rect transform in (§3.5.1).
  - Unit tests: a centered barrel gives a symmetric frustum; a 2° yaw shifts the center by tan(2°); zoom FOV halving doubles the scale; invalid inputs return false.
- **§3.2.3** Gate: the debug mirror shows a magnified view that tracks the barrel.

### §3.3 Phase 3 — lens on the weapon

- **§3.3.1** Lens geometry: a disc at the eyepiece, defined per weapon profile as offset plus radius in `HandWeaponProfile.h` / cfg.
  - Draw it in both eyes, sampling layer 2. Draw it after hands, depth-tested.
- **§3.3.2** The lens samples the **post-tonemap** layer 2. Identify the final color image the SFS submit path copies to the OpenXR swapchain (`StereoXr.inc`, `XrWorker.h`) and keep its layer 2.
- **§3.3.3** Reticle and vignette are procedural in the lens fragment shader (no assets). Add an eye-relief fade: the lens dims to black when the eye is more than about 12 cm from the eyepiece or off-axis, like a real scope.
- **§3.3.4** Gate: the lens is visible on the Precision Bolt with a live magnified image and no judder.

### §3.4 Phase 4 — zoom integration

- **§3.4.1** The scope turns on only for `precisionBoltActive` with zoom engaged (the native zoom state the hooks already observe). It turns off on weapon switch, menus, cinematics, glory kills and Revenant, following `HandVisibilityPolicy` presentation rules.
- **§3.4.2** Keep the flat scope GUI and reticle suppressed and hands visible (current `prepareZoomDecl` behavior). The world view itself does not zoom.
- **§3.4.3** Magnification = native zoom FOV (§0.3). Clamp it to the range 1°–60°.
- **§3.4.4** Gate: scoping in or out toggles the lens instantly, and nothing else about the weapon changes. Firing, ammo and zoom timing are untouched.

### §3.5 Phase 5 — performance

- **§3.5.1** **Sub-rect render.** Map the scope frustum into a corner region of layer 2 (default 1024 px square, configurable) through the clip transform. Reject geometry outside it with `gl_ClipDistance` for view 2 in the vertex-shader rewrite, which requires the `shaderClipDistance` feature. The lens samples only that region.
- **§3.5.2** **Off when idle.** With the scope inactive, view 2 gets a degenerate transform (all geometry clipped), and per-view compute dispatches skip layer 2, using a dispatch Z of 2 instead of 3 where the layer index is the Z slice.
- **§3.5.3** **Cheaper effects in the scope view.** Identify the expensive per-view screen-space passes (SSR, AO, volumetrics, clustered light grid: `EternalLightGrid.h`, `EternalVolumes.h`) and give each a policy: run, run at reduced work, or skip for view 2. Default: skip SSR and volumetric fog, keep lighting.
- **§3.5.4** **Half-rate option.** `scope_rate half` renders view 2 on alternate frames and shows the previous lens image in between.
- **§3.5.5** **Measure.** Use the existing GPU timestamps (`TimestampQueries.inc`, `FrameTiming.h`) to report frame time for: baseline 2-view, scope idle, scope active full-rate, and scope active half-rate. Results go in the checklist.

### §3.6 Phase 6 — config, AMD, release

- **§3.6.1** `assets/argent_controls.cfg` keys, parsed in `WeaponConfig.h` with tests:

  | Key | Values | Default |
  |---|---|---|
  | `scope_enabled` | 0 / 1 | 1 |
  | `scope_resolution` | 512 / 768 / 1024 / 1536 | 1024 |
  | `scope_rate` | full / half | full |
  | `scope_effects` | full / reduced | reduced |

  Add a launcher checkbox for `scope_enabled`.
- **§3.6.2** AMD path parity: the `EternalAmd*` rules get the same three-view handling. Without AMD hardware, gate it with logs and the fallback (§3.1.3).
- **§3.6.3** Release: `tools/package_release.ps1` with a new version, and a GitHub release on the fork.

## §4 Risks / unknowns (resolve in the phase noted)

- **§4.1 (P1)** Some passes may hard-assume two layers in ways regex won't find, such as descriptor indexing or shader constants. Watch for corruption in the scope layer and in the eyes.
- **§4.2 (P1)** Memory: every stereo image becomes 3 layers (+50% render-target memory). Check VRAM headroom. The 32 GB RTX 5090 is fine; flag it for smaller GPUs.
- **§4.3 (P3)** Which image is the final post-tonemap stereo color, and when layer 2 can be sampled without a hazard. A barrier may be needed.
- **§4.4 (P5)** Desktop GPUs have no per-view viewports, so the sub-rect relies on clip distances. If `shaderClipDistance` is unavailable, render the full layer (slower, still correct).
- **§4.5 (P2)** The laser pose may only exist for laser-equipped weapons. The Heavy Cannon is laser-allowed (`laserWeaponAllowed` test list), but confirm it.

## §5 Verification rules

- Pure math and policy (`ScopeProjection.h`, `scopeActive`, `kViews` helpers, cfg parsing) gets unit tests in `tests/`, registered in `CMakeLists.txt` like `player_mechanics_tests`.
- `[verified]` means the user confirmed it in the headset: Steam Frame over SteamVR, RTX 5090, deployed dev build. Unit-green is only `[coded]`.
