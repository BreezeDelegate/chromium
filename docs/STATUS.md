# Status

Product branch: `breeze`.

Done:
- freed Ctrl+Shift+M from the profile menu on Windows, Linux and macOS so DevTools can use it for device mode;
- disabled Chromium password saving and auto sign-in by default;
- disabled address, payment and form-history Autofill by default;
- cleaned sponsored modules from Google Search without touching Gemini and restored a direct Maps action when it is absent;
- bundled SponsorBlock 6.1.7 as an isolated GPL component with the matching upstream source archive;
- added native network ad/tracker blocking with adblock-rust 0.13.3, EasyList and EasyPrivacy, loaded from a reproducible serialized engine.

Next:
- cosmetic filtering, per-site controls and filter-list updates;
- Gemini temporary-chat preference where the official web flow supports it;
- privacy defaults and telemetry review;
- media/DRM build matrix;
- performance build profile and release pipeline.

Validation: static checks on the workspace. A full Chromium build needs a larger builder than this VPS.
