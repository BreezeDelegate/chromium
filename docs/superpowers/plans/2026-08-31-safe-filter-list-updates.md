# Safe Filter-List Updates Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add background EasyList/EasyPrivacy refreshes that preserve the embedded engine as an immutable fallback and never replace a known-good engine with an invalid update.

**Architecture:** Keep active-engine ownership in `breeze_adblock_service.*`; add a focused updater component for refresh cadence, network validation, metadata and transactional cache persistence. Rust compiles the two validated text lists into serialized engine bytes, and C++ validates/deserializes the candidate before atomic activation.

**Tech Stack:** Chromium C++/base/network APIs, Rust `adblock-rust 0.13.3`, cxx bridge, GN/Ninja, gtest.

**Spec:** `docs/superpowers/specs/2026-08-31-safe-filter-list-updates-design.md`

## Global Constraints

- Keep Chromium security boundaries intact.
- No Breeze telemetry, per-user identifier, browsing URL, cookie, or auth data in updater requests.
- EasyList and EasyPrivacy are one atomic update transaction.
- Four days is the normal refresh age.
- Startup and page loads never wait for downloads or compilation.
- Invalid cache/update keeps current engine; embedded engine always remains available.
- Cache lives browser-wide under the user-data directory.
- All heavy I/O, hashing, parsing and compilation stays off the UI thread.

---

### Task 1: Make engine compilation and validation explicit

**Files:**
- Modify: `chrome/browser/breeze_adblock/breeze_adblock_ffi.rs`
- Create: `chrome/browser/breeze_adblock/breeze_adblock_ffi_unittest.cc`
- Modify: `chrome/browser/breeze_adblock/BUILD.gn`

**Interfaces:**
- Produces: `compile_engine(easylist: &str, easyprivacy: &str) -> Vec<u8>` where failure returns empty bytes.
- Produces: `serialized_engine_is_valid(serialized: &[u8]) -> bool` for candidate/cache validation.

- [ ] Add a failing test proving empty/malformed list input is rejected and valid minimal ABP input produces a deserializable serialized engine.
- [ ] Run the focused test and verify failure before production changes.
- [ ] Implement minimal Rust validation and `serialized_engine_is_valid`.
- [ ] Re-run the focused test and existing Breeze adblock tests.
- [ ] Commit the task.

### Task 2: Extract cache/cadence/response policy into testable updater helpers

**Files:**
- Create: `chrome/browser/breeze_adblock/breeze_adblock_updater.h`
- Create: `chrome/browser/breeze_adblock/breeze_adblock_updater.cc`
- Create: `chrome/browser/breeze_adblock/breeze_adblock_updater_unittest.cc`
- Modify: `chrome/browser/breeze_adblock/BUILD.gn`

**Interfaces:**
- Produces: constants for canonical list URLs, max response bytes and four-day refresh interval.
- Produces: helper decisions for freshness, accepted HTTPS final URLs, ABP header/title validation, metadata schema validation and candidate cache paths.

- [ ] Write failing unit tests for four-day freshness, HTTPS downgrade rejection, HTML/error-body rejection, expected EasyList/EasyPrivacy titles and schema/digest mismatch.
- [ ] Run focused tests and verify the expected failures.
- [ ] Implement only the pure helpers needed to pass.
- [ ] Re-run focused tests and `diff --check`.
- [ ] Commit the task.

### Task 3: Add last-known-good cache loading and atomic engine replacement

**Files:**
- Modify: `chrome/browser/breeze_adblock/breeze_adblock_service.h`
- Modify: `chrome/browser/breeze_adblock/breeze_adblock_service.cc`
- Create: `chrome/browser/breeze_adblock/breeze_adblock_service_unittest.cc`
- Modify: `chrome/browser/breeze_adblock/BUILD.gn`

**Interfaces:**
- Produces: service entry point that attempts a serialized candidate and returns success without disturbing current engine on failure.
- Produces: startup helper selecting valid cache first and embedded engine second.

- [ ] Write failing tests proving invalid candidate preserves current engine and valid candidate replaces it.
- [ ] Run focused tests and verify failure.
- [ ] Implement candidate validation plus atomic `shared_ptr` swap and cache-first/embedded-second startup selection.
- [ ] Run focused service/prefs tests and verify all pass.
- [ ] Commit the task.

### Task 4: Wire background fetching, conditional requests and transactional persistence

**Files:**
- Modify: `chrome/browser/breeze_adblock/breeze_adblock_updater.h`
- Modify: `chrome/browser/breeze_adblock/breeze_adblock_updater.cc`
- Modify: `chrome/browser/breeze_adblock/breeze_adblock_updater_unittest.cc`
- Modify: `chrome/browser/breeze_adblock/breeze_adblock_service.cc`
- Modify: `chrome/browser/breeze_adblock/BUILD.gn`
- Modify: `docs/STATUS.md`
- Modify: `docs/ROADMAP.md`

**Interfaces:**
- Consumes: canonical URLs, policy helpers, Rust compiler, service candidate-install entry point.
- Produces: one process-global updater transaction with conditional requests, bounded downloads, off-thread compile/hash/write, metadata-last commit and startup scheduling.

- [ ] Write failing tests for one-list failure aborting the transaction, `304` handling, oversized response rejection and metadata-last commit decisions.
- [ ] Run focused tests and verify failure.
- [ ] Implement fetch orchestration using Chromium `SharedURLLoaderFactory`/`SimpleURLLoader`, disable cookies/auth credentials, cap download size, validate both bodies, compile off-thread, then persist/activate only after validation.
- [ ] Run focused unit tests, GN generation for the Breeze target, targeted Ninja build where capacity allows, Rust/CXX checks and `git diff --check`.
- [ ] Update status/roadmap with evidence and commit the task.

### Task 5: Release/build convergence

**Files:**
- Reuse the existing Windows build/installer branch work as reference; do not duplicate the workflows into product history unless needed.
- Modify product branch only for build fixes proven necessary.

**Interfaces:**
- Consumes: complete `breeze-next` product commits.
- Produces: buildable product branch and a verified installable browser artifact on an approved Windows-capable builder.

- [ ] Rebase/synchronize `breeze-next` against current `origin/main` only if required by build tooling and resolve Breeze patches minimally.
- [ ] Run all checks possible on the canonical VPS first.
- [ ] Verify whether Windows x64 can be built on approved self-hosted infrastructure; do not silently use GitHub-hosted compute.
- [ ] On a Windows-capable approved builder, generate release GN args, build `mini_installer`, install into a disposable profile/machine context and launch `chrome.exe` smoke test.
- [ ] Record SHA-256, browser version, source commit and artifact location in `docs/STATUS.md`.
