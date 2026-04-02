# TwinPilot

TwinPilot is an Unreal Engine plugin for operator-style camera and interaction control in digital twin scenes.

## Scope

- Runtime module: `TwinPilot`
- Primary types:
  - `ADigitalTwinOperatorPawn`
  - `ADigitalTwinOperatorController`
  - `UDigitalTwinPawnLibrary` (Blueprint Function Library)

## Key Capabilities

- Orbit/pan/zoom operator camera controls.
- Cursor-based actor hover and selection.
- Custom depth/stencil tagging for interaction highlight workflows.
- Blueprint-friendly utility functions for movement, focus, and look-control toggles.

## Quick Start

1. Place plugin in `Plugins/TwinPilot`.
2. Regenerate project files and build.
3. Set:
   - Player Pawn: `ADigitalTwinOperatorPawn` (or child BP)
   - Player Controller: `ADigitalTwinOperatorController` (or child BP)
4. Bind input actions/axes to your project mappings.

## Interaction Notes

- `SelectActorUnderCursor(AActor*& HitActor)` returns selection success and exposes the hit actor.
- `OnActorSelected` and `OnActorHovered` delegates are provided for Blueprint event wiring.
- Actors tagged with `Background` are ignored by cursor selection by default via `SelectionBlockedTag`.
- `ClearConfirmedActor()` clears both the confirmed selection and current hover/preselection state.

## Repository Layout

- `Source/`: runtime source
- `Content/`: plugin assets
- `Documentation/`: design/architecture notes
- `TwinPilot.uplugin`: plugin descriptor

## License

Unless otherwise noted, the original source code and original repository-authored non-code files in this repository are licensed under the Apache License 2.0.
See `LICENSE` for the full text.

### Licensing Scope and Exceptions

- This repository does not relicense Unreal Engine code, headers, or content.
- Use with Unreal Engine is subject to Epic's Unreal Engine EULA.
- Third-party dependency/license metadata is tracked in `ThirdParty.json`.
