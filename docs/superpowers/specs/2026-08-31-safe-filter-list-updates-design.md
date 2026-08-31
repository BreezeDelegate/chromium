# Safe Filter-List Updates Design

## Goal

Keep Breeze native ad/tracker blocking current without making browser startup, browsing, or privacy depend on a successful network update. EasyList and EasyPrivacy remain the only list inputs for this first updater lot. The embedded serialized adblock engine remains the permanent recovery baseline.

## Product constraints

- Keep the fork close to Chromium and keep the patch easy to rebase.
- Never weaken Chromium sandboxing, site isolation, TLS validation, Safe Browsing, or exploit mitigations.
- Do not introduce a Breeze telemetry endpoint or a user identifier.
- Do not make page loading wait for a filter-list download or compilation.
- Do not replace a working engine until a replacement has downloaded, parsed, serialized, deserialized, and passed validation.
- A failed update must leave the current in-memory engine and last known-good cache untouched.
- A missing or corrupt cache must fall back to the engine embedded in the browser binary.
- Keep the current per-site allowlist behavior shared by network and cosmetic filtering.
- Use Chromium networking and filesystem primitives rather than a custom updater runtime.

## Inputs and cadence

The updater fetches the canonical EasyList and EasyPrivacy text lists over HTTPS:

- `https://easylist.to/easylist/easylist.txt`
- `https://easylist.to/easylist/easyprivacy.txt`

Both upstream lists currently advertise a four-day expiry interval. Breeze therefore treats four days as the normal refresh age. On browser startup, an update is scheduled only when the last successful refresh is older than four days or no successful refresh metadata exists. The check is delayed until after startup so it does not compete with the critical browser-launch path.

A failed refresh does not enter a tight retry loop. The same browser session may retry once after a bounded delay; otherwise the next normal browser launch can try again. This keeps network noise and failure amplification low.

Where the server provides `ETag` or `Last-Modified`, Breeze persists those validators and uses conditional requests. `304 Not Modified` updates the successful-check timestamp without recompiling the engine.

## Storage model

The updater stores browser-wide state below the Chromium user-data directory, not inside an individual profile, because the native engine is process-global and shared by all profiles.

The cache contains:

- the serialized adblock engine (`engine.dat`);
- update metadata (`metadata.json`) containing schema version, successful-check timestamp, successful-content timestamp, source URLs, source validators, source byte counts, and a SHA-256 digest of the serialized engine.

Raw downloaded filter lists are temporary inputs only. They are not retained after a successful compile and are removed after a failed attempt.

The cache is written transactionally: write candidate files to temporary paths, flush/close them, validate the serialized candidate by constructing an `adblock::Engine`, then atomically replace the previous cache. Metadata is committed last so it never claims an engine exists before that engine is durable.

## Startup and fallback order

Engine startup uses this order:

1. Read the embedded serialized engine from `IDR_BREEZE_ADBLOCK_ENGINE`.
2. Attempt to read and validate the cached serialized engine.
3. If the cache is valid and its metadata/digest match, install the cached engine.
4. Otherwise install the embedded engine.
5. Schedule a background update only after a usable engine is installed.

The embedded engine is never deleted or overwritten because it is part of the browser resources. It is therefore always available after a broken download, corrupt disk cache, incompatible cache format, or interrupted write.

A cache schema version is required. Schema mismatch means "ignore cache and rebuild from upstream later", not browser failure.

## Download validation

Each list download must satisfy all of the following before compilation:

- HTTPS final URL;
- successful HTTP status or `304`;
- no authentication or cookies required;
- bounded response size;
- non-empty body for a changed response;
- recognizable Adblock Plus list header;
- title matches the expected list (`EasyList` or `EasyPrivacy`);
- content remains text rather than HTML/error output.

Redirects are allowed only when Chromium considers the target HTTPS URL valid; a downgrade to HTTP is rejected.

The two lists form one update transaction. Breeze never compiles a mixed generation where only one changed list is available and the other required source is missing. If one response fails, the whole refresh fails and the existing engine stays active.

## Compilation and engine validation

Compilation happens on a background `ThreadPool` sequence. The Rust boundary owns list parsing and serialization through `adblock-rust 0.13.3`.

The Rust API returns an explicit success/failure result rather than treating an empty byte vector as a fully expressive error channel. The C++ side validates the resulting bytes by constructing a candidate `BreezeAdblockEngine` and requiring `is_loaded() == true` before persistence or activation.

The candidate engine is installed with the already-prepared atomic `shared_ptr` swap. Readers continue using the previous shared engine while the new engine is compiled and validated, so concurrent requests cannot observe freed memory or a half-initialized engine.

## Update controller boundaries

The implementation is split by responsibility instead of growing the request-matching service into an updater monolith:

- `breeze_adblock_service.*`: owns the currently active engine, fallback loading, validation/install entry points, and read-side blocking/cosmetic APIs.
- `breeze_adblock_updater.*`: owns scheduling, network fetches, response validation, conditional-request metadata, transactional persistence, and calls into the service only with a fully validated serialized candidate.
- `breeze_adblock_ffi.rs`: owns Rust list compilation and serialized-engine construction checks.
- updater tests: exercise cache fallback, response rejection, atomic persistence decisions, cadence decisions, and successful install handoff without performing live Internet requests.

Network fetching and update state stay out of request interception (`breeze_adblock_throttle.*`) and DOM cosmetic code.

## Threading and lifetime

The active engine remains readable from arbitrary browser request paths via atomic `shared_ptr` load/store.

Disk I/O, hashing, and Rust compilation run off the UI thread. Network callbacks return to a sequence-owned updater controller. The updater must not capture an unretained object across shutdown. Cancellation/destruction at browser shutdown is normal and must leave the last committed cache intact.

Only one update transaction may be active at a time.

## Privacy behavior

The updater contacts only the two declared EasyList HTTPS endpoints and sends no Breeze account, profile, browsing, installation, or site history data. Conditional request headers may reveal only the previously observed upstream validator for that same public list.

No page URL or browsing event triggers a list download. Refresh eligibility is based solely on updater timestamps.

## Failure handling

All failures are fail-open with respect to browser usability and fail-stable with respect to filtering state:

- no embedded resource: browser continues without blocking until a valid engine becomes available;
- invalid cache: ignore it and use embedded engine;
- network failure: keep current engine;
- oversized or malformed list: reject update;
- compile/deserialize failure: reject update;
- cache write/rename failure: keep current engine and previous cache;
- shutdown during update: cancel outstanding work and keep previous committed state.

No failure path should crash the browser or replace a known-good engine with an invalid one.

## Testing strategy

Tests are written before production changes.

Rust-level tests cover list compilation success, empty/malformed input rejection, and deserialization of compiled output.

C++ unit tests cover:

- valid cache preferred over embedded engine;
- corrupt cache rejected;
- digest/schema mismatch rejected;
- four-day freshness decision;
- conditional metadata handling and `304` behavior;
- one-list failure aborting the transaction;
- oversized/non-list/HTTP-downgrade responses rejected;
- candidate validation before install;
- previous engine preserved when install validation fails;
- successful candidate atomically replacing the active engine;
- temporary-file/metadata commit ordering helpers.

The final validation for this lot includes targeted Rust/C++ tests, GN/Ninja dependency checks, and the existing Breeze static checks. A full Chromium browser build is a separate release gate and must be performed on appropriate builder capacity because project status already records that the canonical VPS is insufficient for a full Chromium build.

## Out of scope for this lot

- additional regional/custom filter lists;
- user-managed filter subscriptions;
- a UI for list selection or update timing;
- remote Breeze configuration or telemetry;
- changing the existing per-site disable control;
- Gemini preference work;
- telemetry/privacy-default review outside updater behavior;
- Widevine/media licensing changes;
- performance tuning unrelated to updater overhead;
- release packaging itself.

## Success criteria

The lot is complete when a current EasyList/EasyPrivacy pair can be fetched, compiled, validated, cached, and activated without blocking startup; a browser with no network continues to use the latest known-good engine; every invalid/interrupted update preserves the previous engine; and the embedded engine remains a guaranteed fallback.
