# VR Scope — Checklist

States: `[ ]` → `[coded]` (built, unit tests green) → `[verified]` (the user confirmed in headset).

The spec is the contract: `vr-scope-spec.md`. Done means every line is `[verified]`, or carries a user-approved note.

## Phase 1: three-view SFS (§3.1)
- [ ] §2.1 `kViews` / `kScopeView` constants introduced; no stray literal-2 view counts in `src/sfs`.
- [ ] §2.2 `EyeUniforms` / `FrameUniforms` resized to 3 views; std140 block and `static_assert` updated.
- [ ] §3.1.1 Images use 3 layers; views use `layerCount` 3; render-pass mask is 7, correlation mask stays 3.
- [ ] §3.1.1 Dispatch depth, query slots, timestamps, `SourceRing`, and water capture all generalized.
- [ ] §3.1.2 View 2 = left-eye projection.
- [ ] §3.1.2 `ARGENT_SCOPE_DEBUG=1` mirrors layer 2 to the desktop.
- [ ] §3.1.3 `maxViews>=3` check, with a fallback to 2 views (scope off) and a log line.
- [ ] §3.1.4 Unit tests added and passing.
- [ ] §3.1.4 Headset: eyes unchanged.
- [ ] §3.1.4 Headset: debug mirror shows layer 2.
- [ ] §3.1.4 Frame time recorded:
  - 2-view: ___ ms
  - 3-view inert: ___ ms

## Phase 2: scope camera (§3.2)
- [ ] §3.2.1 Scope pose from `renderedLaser`, with the calibrated-weapon fallback.
- [ ] §3.2.2 `ScopeProjection.h` written (off-axis parallel frustum), with unit tests.
- [ ] §3.2.3 Headset: debug mirror shows a magnified view that tracks the barrel.

## Phase 3: lens (§3.3)
- [ ] §3.3.1 Per-profile lens geometry (offset + radius) in cfg.
- [ ] §3.3.1 Lens drawn in both eyes, depth-tested.
- [ ] §3.3.2 Lens samples the final post-tonemap layer 2, hazard-free.
- [ ] §3.3.3 Procedural reticle, vignette, and eye-relief fade.
- [ ] §3.3.4 Headset: live magnified lens on the Precision Bolt, no judder.

## Phase 4: zoom integration (§3.4)
- [ ] §3.4.1 `scopeActive` policy written, with tests.
- [ ] §3.4.1 Off for weapon switch, menus, cinematics, glory kills, and Revenant.
- [ ] §3.4.2 Flat GUI stays suppressed; hands stay visible; the world doesn't zoom.
- [ ] §3.4.3 Magnification uses the native zoom FOV, clamped to 1–60°.
- [ ] §3.4.4 Headset: instant toggle; fire, ammo, and timing unchanged.

## Phase 5: performance (§3.5)
- [ ] §3.5.1 Sub-rect render with `gl_ClipDistance`, plus a fallback when it's unsupported.
- [ ] §3.5.2 View 2 inert when idle; compute skips layer 2.
- [ ] §3.5.3 Per-pass effect policy for view 2 (SSR and volumetric fog skipped by default).
- [ ] §3.5.4 `scope_rate half` alternates frames.
- [ ] §3.5.5 Timings recorded:
  - baseline: ___
  - idle: ___
  - active full: ___
  - active half: ___

## Phase 6: config, AMD, release (§3.6)
- [ ] §3.6.1 cfg keys `scope_enabled`, `scope_resolution`, `scope_rate`, and `scope_effects` parsed and tested.
- [ ] §3.6.1 Launcher checkbox.
- [ ] §3.6.2 AMD rules handle 3 views (or log the fallback).
- [ ] §3.6.3 Packaged release published on the fork.
