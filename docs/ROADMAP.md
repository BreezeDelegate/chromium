# Roadmap

## Browser defaults

Keep Chrome-like behavior while removing prompts and background features that do not help browsing.

## Blocking

Use a native request/cosmetic filtering engine with maintained filter lists. Keep SponsorBlock isolated as its own licensed component.

## Search

Hide sponsored Search modules without hiding Gemini. Restore a direct Google Maps action from the current query instead of spoofing region settings.

## Privacy

Audit telemetry, variations, prediction, background services, tracking parameters, bounce tracking, WebRTC and site-data isolation. Keep security services that materially protect the user.

## Media

Build with the broadest codecs each target can legally ship. Support Widevine through legitimate distribution only. Test real streaming services because some providers apply their own browser certification rules.

## Performance

Measure startup, memory, tab switching, page load and background CPU before changing defaults. Prefer PGO/LTO and removing unnecessary work over benchmark-only flags.

## Updates

Keep Chromium security updates easy to rebase. Small patches win over a deep fork.
