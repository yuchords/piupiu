# Changelog Writing Guide

Read this file only when writing or updating a milestone-level changelog.

## Purpose

This file is for human-readable release notes, not raw commit history.
Write changelogs in plain language so a teammate can quickly understand what changed.

## When To Update

- Major version updates
- Milestone completions
- Important prototype validation progress
- Backend or API changes with visible impact

## Writing Rules

- Focus on user-visible or project-visible changes, not every internal edit.
- Use direct language and describe what was actually achieved.
- Do not copy commit messages line by line.
- Group related changes together.
- Mention unresolved risks when they matter.

## Recommended Sections

### 1. Functional Changes

Use plain language to describe what was implemented.

Examples:
- Added ESP32 prototype support for LED strip control.
- Added initial server API for device status reporting.
- Added WeChat Mini Program login and device binding flow.

### 2. Bug Fixes

Describe what problem was fixed and what behavior improved.

Examples:
- Fixed repeated reconnect failures after ESP32 network recovery.
- Fixed incorrect API error handling in device registration.

### 3. Known Issues / Potential Problems

List current limitations, unstable areas, or items still under verification.

Examples:
- Audio pipeline is still experimental and may have latency spikes.
- Camera-related features are not yet stable on the current prototype board.

### 4. Notes

Use this for compatibility notes, migration reminders, scope changes, or milestone remarks.

Examples:
- This phase only covers prototype validation and server integration.
- Mobile app support is not part of the current milestone.

## Suggested Template

```md
# vX.Y.Z - YYYY-MM-DD

## Functional Changes

- 

## Bug Fixes

- 

## Known Issues

- 

## Notes

- 
```

