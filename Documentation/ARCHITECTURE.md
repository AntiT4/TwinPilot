# TwinPilot Architecture

## Modules

- `TwinPilot` runtime module

## Main Types

- `ADigitalTwinOperatorPawn`
  - Free-fly operator pawn with move/look/sprint APIs
  - Focus helper for fast camera positioning around a target
- `UDigitalTwinPawnLibrary`
  - Blueprint callable utility helpers for control and targeting

## Integration Boundary

- Depends only on Unreal Engine modules (`Core`, `CoreUObject`, `Engine`, `InputCore`)
- No dependency on project-specific game modules

## Intended Repository Model

- Standalone plugin Git repository
- Included in project via `Plugins/TwinPilot`
