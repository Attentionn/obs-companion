# Installing OBS Companion 1.0.0

1. Download the Windows x64 plugin ZIP from Releases, then close OBS.
2. Copy `obs-companion.dll` directly to your OBS installation's `obs-plugins/64bit/` folder. Do not copy a containing folder.
3. Open OBS → Tools → OBS Companion. Choose sources, test sounds and click Apply.

Example: `D:\SteamLibrary\steamapps\common\OBS Studio\obs-plugins\64bit\obs-companion.dll`

No external data folder is required. Built-in sounds and translations are embedded.

Optional voices: keep `audio/en`, `audio/esp`, `audio/fr` or `audio/cn` in a permanent location. Select each WAV under Customize sounds → Browse → Test → Apply. This is manual; changing interface language does not change sounds. Keep custom files at their selected path.

Supported custom format: 16-bit PCM WAV, mono/stereo, up to 4 MiB per file.

Apply saves without closing. Save and close saves and exits. Pause monitoring shows a five-minute countdown; Resume all monitoring resumes immediately. Event-based replay and mute-change notifications remain active during the pause.

Black/unchanged output monitoring is experimental and disabled by default. Consult README.md for limits and testing instructions.

Contact: Discord `lvnula` · Email `lvnula@proton.me`
Optional donations: https://www.paypal.com/paypalme/darioh07
