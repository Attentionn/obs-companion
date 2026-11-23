# Changelog

## 1.1.0

- Scene switches, overlapping recording/replay sessions and unrelated settings changes preserve reminder timing. Microphone mute changes preserve the desktop silence timer.
- Zero-volume alerts leave other audible notifications available during the same update.
- Windows builds produce a single installable ZIP for local builds, Actions downloads and tagged releases.
- Embedded multilingual voice presets alongside minimalist tones; independent voice/interface language and custom WAV overrides.
- Sound-only and full-settings reset with confirmation and Apply/Cancel semantics.
- Removed video detection, its settings tab and sampling from the normal build; legacy video settings are retired.
- Added explicit notification dock instructions.
- Documentation-only pushes no longer trigger Windows compilation; tags and manual builds remain supported.


## 1.0.0

- Replay saved, Replay Buffer started and stopped notifications.
- Microphone mute/unmute notifications and timed muted reminders.
- Independent optional microphone and desktop no-signal warnings.
- Tabbed settings with Apply, Save and close, and collapsible advanced/sound controls.
- Five-minute monitoring pause with live countdown and resume controls.
- Optional OBS notification dock.
- Embedded tones and four interface languages; support for custom PCM16 WAV recordings.
- Optional language-specific voice-pack folders.
- Experimental black/unchanged output warnings with live diagnostics.

Video detection remains experimental; Windows/OBS runtime confirmation is required. Earlier development history is maintained separately.
