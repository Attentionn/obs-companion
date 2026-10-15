# OBS Companion 1.1.0 installation

Close OBS. Copy only `obs-companion.dll` directly into OBS’s `obs-plugins/64bit/` folder, replacing the previous DLL. Do not add a containing subfolder. Restart OBS and open Tools → OBS Companion.

No external data or audio folder is needed: minimalist tones, four voice packs and translations are embedded.

General → Sound presets: choose Minimalist tones or Voices, then choose a voice language or Follow interface language. Custom WAVs override the preset per event. Use preset sound removes the override. Apply saves without closing.

Custom files: PCM16 WAV, mono/stereo, up to 4 MiB each. Keep them at their selected location.

Restore sound defaults and Reset all settings require confirmation, then Apply to save. They never delete recordings. Cancel discards staged resets.

For the dock, enable Show notification panel inside OBS → Apply, then Docks → OBS Companion.

Video detection is retired in this build. Replay and audio monitoring remain available.

Discord: `lvnula` · Email: `lvnula@proton.me`
Optional donations: https://www.paypal.com/paypalme/darioh07
