# Project rules

Read `BREEZE.md`, `docs/STATUS.md` and `docs/ROADMAP.md` before changing the fork.

Keep patches small and easy to rebase on Chromium. Do not weaken sandboxing, site isolation, TLS validation or exploit mitigations for convenience.

Privacy changes should remove unnecessary collection, not break websites. Regional UX fixes must not fake the user's country or disable legal/privacy protections.

Keep proprietary codecs and CDMs behind their real licensing requirements. Third-party blockers and filter lists keep their own licenses.

Public commits, PRs and comments should be short and project-focused.

<!-- CANONICAL_VPS_CONTEXT:START -->
## Canonical VPS context — mandatory

Before any development, deployment, Git/GitHub write, migration, DNS/network change or operational action in this repository, read and follow **both** canonical references:

- `https://github.com/BreezeDelegate/workspace-reference` — global working rules, Git/GitHub authorship, security and cross-project conventions. Local canonical clone: `/opt/repos/workspace-reference`.
- `https://github.com/BreezeDelegate/vps-infrastructure` — source of truth for this VPS: architecture, services, paths, ports, reverse proxy/DNS, deployment and safety constraints. Local canonical clone: `/opt/repos/vps-infrastructure`.

Inspect the real VPS state before changing it. If repository-local documentation conflicts with either canonical reference or with the live state, stop, reconcile the mismatch, and update the documentation instead of guessing. Keep the project handoff/status documentation current before ending substantial work.
<!-- CANONICAL_VPS_CONTEXT:END -->
