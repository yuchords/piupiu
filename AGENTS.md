# AGENTS.md

## Project Scope

- This repository is for an ESP32-based embedded desktop assistant prototype.
- The current main target is prototype validation on ESP32 using ESP-IDF.
- Current active software scope only includes:
  - ESP32 firmware prototype
  - server-side services



## Priority Rules

- User requirements take precedence over document conventions.
- Do not follow documents mechanically when the user has already given a clear requirement.
- Prefer the current development request over generic templates or canned process.
- When requirements and documents conflict, follow the user and mention the conflict briefly if it matters.
- When the user mentions words such as `规划`, `计划`, `帮我设计`, or other planning/design intent, this becomes the highest-priority document rule:
  read all documents under `docs/architecture/` first as the primary design requirements, and use information under `docs/memo/` as supporting reference.
- whenever you use the document from `docs/` or `.agent`, you should clearly output which document you use and what infomation you learned.

## Coding Style

- Follow existing project patterns before introducing new abstractions.
- Keep code simple, readable, and easy to debug.
- Avoid premature generalization, and unnecessary framework expansion.
- Make small, scoped changes unless the task clearly requires larger refactoring.
- When changing behavior, update the closest relevant documentation.
- For ESP32 firmware work, align with ESP-IDF conventions and embedded constraints.
- For server-side and API work, optimize for clear contracts and stable integration with ESP32 .
- Focus on future-proofing the code.Before writing the code, briefly explain the software architecture you plan to use to ensure this project remains scalable for future requirements.

## Context Loading

- Do not load all docs by default.
- First determine the task type, then read only the smallest relevant set of files.
- For planning or design-intent requests, load all documents under `docs/architecture/` before responding.
- For planning or design-intent requests, use `docs/memo/` only as auxiliary context, not as the primary source of requirements.
- Read `.agent/` files only when they are directly relevant to the current task.
- Do not read `.agent/DESIGN.md` unless the task is about design, architecture, hardware selection, module boundaries, or feature planning.
- Do not read `.agent/changelog.md` unless the task is about version summaries, milestone updates, release notes, or changelog writing.

## Task Routing

- For API, backend, and mini-program integration tasks, read `docs/api/` first.
- For architecture, hardware selection, and feature planning tasks, read `docs/architecture/` first.
- For requests containing `规划`, `计划`, `帮我设计`, or similar design/planning language, read all files under `docs/architecture/` first and treat them as the highest-priority design input; then read `docs/memo/` as supporting material if needed.
- For project milestone history and release summaries, read `docs/changelog/` first.
- For agent-specific operating references, read only the matching file under `.agent/` when needed.

## Documentation Rules

- Put interface definitions and integration notes in `docs/api/`.
- Put hardware selection, system boundaries, and software planning in `docs/architecture/`.
- Put version-level summaries and milestone updates in `docs/changelog/`.
- Keep documents concise, direct, and task-oriented.

## Change Management

- If a change affects interfaces, update `docs/api/`.
- If a change affects architecture, hardware selection, or feature planning, update `docs/architecture/`.
- If a change is a milestone-scale update, update `docs/changelog/` and read `.agent/changelog.md` if needed.

## Working Method

- Prefer direct implementation over long planning unless the task is unclear or risky.
- If a large or risky change is needed, state the assumption and proceed carefully.
- Raise questions only when a reasonable assumption would be too risky.
