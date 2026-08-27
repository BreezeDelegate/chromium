# Performance

The target is lower overhead without making normal browsing feel slower. Memory reductions that trade away tab-switch latency, page-load latency, process isolation or reliability are not considered free wins.

## Current rules

- Keep Chromium's multi-process and site-isolation model.
- Keep BFCache and normal page preloading until a browser-level benchmark proves a better trade-off.
- Do not force aggressive tab discarding. Chromium already has Memory Saver, memory-pressure handling and platform working-set trimming.
- Prefer removing duplicate work, shortening object lifetimes and bounded one-shot work over permanent page observers.
- Benchmark a full release build before changing allocator, renderer-process or cache defaults.

## Adblock overhead

The adblock engine uses the serialized EasyList/EasyPrivacy engine and Chromium's own registrable-domain calculation, avoiding a second public-suffix resolver. The serialized bytes are released after deserialization.

A standalone release benchmark on the current VPS measured:

- serialized engine: 6,044,437 bytes;
- process RSS before loading: about 2.9 MiB;
- RSS with the deserialized engine after releasing the serialized bytes: about 9.7 MiB;
- steady engine overhead in that harness: about 6.8 MiB;
- 1,000 cosmetic lookups did not produce meaningful RSS growth;
- representative URL-specific cosmetic lookups took about 1.5-2.0 ms and produced roughly 0.3-26 KiB of CSS.

These numbers measure the engine in isolation, not total browser memory. A full browser build is required for meaningful Chrome-vs-fork comparisons.

## Cosmetic filtering

URL-specific CSS is injected once at DOMContentLoaded. Generic class/id rules use one bounded DOM scan with a TreeWalker, cap class/id tokens at 4,096 each and ignore tokens longer than 256 characters. No MutationObserver is kept alive after the scan.

## Full-build benchmark gate

Before calling the browser performance-ready, compare the fork with the matching Chromium revision using the same profile and workload:

- cold and warm startup;
- idle browser RSS/PSS;
- 1, 10 and 30-tab working sets;
- tab-switch latency;
- page-load metrics on a fixed local corpus;
- background CPU after settling;
- memory pressure and tab-discard behavior.
