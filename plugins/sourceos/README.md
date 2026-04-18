# SourceOS Albert plugin

This plugin adds **keyboard-first SourceOS workstation actions** to Albert.

Design goals:

- Treat Albert as an **action bus** (command palette), not as a second filesystem index.
- Keep actions auditable and compatible with SourceOS execution boundaries.

## Trigger

Default trigger: `sourceos `

## Actions (v0)

- `doctor` → runs `sourceos doctor workstation-v0`
- `profile apply` → runs `sourceos profile apply workstation-v0`
- `k9s` → launches `k9s`
- `lazygit` → launches `lazygit`

## Notes

- This plugin assumes the `sourceos` CLI is installed on PATH (installed by workstation profile tooling).
- A future iteration will add richer parameterization and optional GNOME portal gating.
