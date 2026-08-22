# Status

Product branch: `breeze`.

Done:
- freed Ctrl+Shift+M from the profile menu on Windows, Linux and macOS so DevTools can use it for device mode;
- disabled Chromium password saving and auto sign-in by default;
- disabled address, payment and form-history Autofill by default;
- cleaned sponsored modules from Google Search without touching Gemini and restored a direct Maps action when it is absent;
- bundled SponsorBlock 6.1.7 as an isolated GPL component with the matching upstream source archive;
- added native network ad/tracker blocking with adblock-rust 0.13.3, EasyList and EasyPrivacy, loaded from a reproducible serialized engine;
- added native cosmetic filtering for HTTP(S) frames, including bounded one-shot class/id discovery for generic EasyList selectors without persistent DOM observers.
- added a Page Info per-site ad/tracker switch backed by a registrable-domain allowlist shared by network and cosmetic filtering.

Next:
- safe filter-list updates with the embedded engine kept as fallback;
- Gemini temporary-chat preference where the official web flow supports it;
- privacy defaults and telemetry review;
- media/DRM build matrix;
- performance build profile, memory/page-load benchmarks and release pipeline.

Validation: targeted Rust/CXX tests and JS syntax checks pass; Page Info/pref unit-test coverage is wired into Chromium's `unit_tests`, and static workspace checks pass. A full Chromium build needs a larger builder than this VPS.
