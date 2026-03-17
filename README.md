# TwinPilot

Operator-focused player pawn library plugin for Unreal Engine digital twin projects.

## Features

- `ADigitalTwinOperatorPawn`
- `UDigitalTwinPawnLibrary` (Blueprint Function Library)

## Installation

1. Place this repository under your project `Plugins` directory as `Plugins/TwinPilot`.
2. Generate project files and build the project.
3. Enable the plugin from `Edit > Plugins` if needed.

## Usage

1. Set your default pawn class to `ADigitalTwinOperatorPawn` or a Blueprint child class.
2. Route input values into these functions:
   - `ApplyMoveInput(Right, Forward)`
   - `ApplyVerticalInput(Up)`
   - `ApplyLookInput(Yaw, Pitch)`
   - `SetSprintEnabled(bEnabled)`
3. Use utility nodes from `UDigitalTwinPawnLibrary`:
   - `SetOperatorPawnMoveSpeed`
   - `FocusOperatorPawnOnActor`
   - `TeleportPawnNearActor`
   - `SetPlayerLookInputEnabled`

## Repository Layout

- `Source/` plugin runtime module source
- `Content/` optional plugin content assets
- `Documentation/` notes and technical docs
- `TwinPilot.uplugin` plugin descriptor

## Notes

- This plugin is intended to be maintained as an independent Git repository.
- Add `Plugins/TwinPilot/` to the parent project's `.gitignore` to isolate plugin changes from the parent repo.
