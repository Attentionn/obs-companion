# OBS Companion 1.1.0

A free Windows x64 plugin for OBS Studio, created by **Nula / Attentionn**.

## Features

- Audio confirmation when OBS saves a replay, using your existing hotkey.
- Replay Buffer start and stop notifications.
- Microphone mute/unmute sounds and timed muted reminders.
- Optional microphone and desktop no-signal warnings with separate thresholds, delays and repeat intervals.
- Built-in notification tones, per-event volume and optional custom WAV files.
- English, Spanish, French and Simplified Chinese interface translations.
- An optional notification dock inside OBS, without desktop popups or a game overlay.

## Installation

1. Download `obs-companion-windows-x64.zip` from Releases. GitHub's source-code ZIP is for developers.
2. Close OBS Studio.
3. Copy **only `obs-companion.dll`** directly into your OBS installation's **`obs-plugins/64bit/`** folder. Replace the previous DLL if upgrading.
4. Start OBS and open **Tools → OBS Companion**.

Example paths:

```text
C:\Program Files\obs-studio\obs-plugins\64bit\obs-companion.dll
D:\SteamLibrary\steamapps\common\OBS Studio\obs-plugins\64bit\obs-companion.dll
```

Do not place the DLL inside an extra `obs-companion` subfolder. A separate `data` folder is not required for installation: default tones and translations are embedded in the DLL.

The build targets OBS Studio 31.1.1 on Windows x64. Compatibility with other versions requires testing.

## Settings

**Apply** saves all tabs without closing the window. **Save and close** saves and exits. **Cancel** discards edits since the last Apply; it does not undo settings already applied.

Choose your microphone under **Microphone source**. **None** disables microphone tracking. Muted/unmuted reports the selected source's mute state in OBS; it does not prove that the device is receiving sound. Hardware and Discord mute states may differ.

Muted reminders only enable alerts: they never mute or disable your microphone. No-signal checks are optional and independent. Muted, disabled and missing audio sources are excluded from no-signal alerts. Intentional silence can still trigger a reminder on an unmuted source.

Duration-based checks run only while Replay Buffer or an unpaused recording is active. **Pause all monitoring for 5 minutes** shows a countdown; **Resume all monitoring** resumes immediately. Replay and microphone mute-change event notifications remain available during this pause.

Advanced settings and sound controls are collapsed to keep each tab readable. Default volumes are 100%; existing saved volume choices are preserved. Use the reset-all button if you want every sound at 100%.

## Sound presets and custom WAVs

In General → Sound presets, choose **Minimalist tones** or **Voices**. Voices can follow the interface language or use English, Spanish, French or Chinese independently. Unsupported automatic languages fall back to English. Click Apply to save without closing.

Tones and all four voice packs are embedded in the DLL. No external audio/data folder is needed for installation. Voice recordings by **Nula / Attentionn**, generated with ElevenLabs and edited in FL Studio.

A per-event custom WAV overrides the preset. Changing presets preserves custom paths, volumes and disabled sounds. **Use preset sound** clears that event’s custom path. Test previews the currently selected preset and language before Apply. If an embedded voice resource is unavailable, the embedded minimalist tone is used.

Custom files may have any filename. They must be PCM16 WAV, mono/stereo, at most 4 MiB per file, and remain at their selected location. Sounds play through Windows’s default output and may enter Desktop Audio.

**Restore sound defaults** selects minimalist tones, enables sounds, resets volumes to 100% and clears custom paths. **Reset all settings** also restores sources to None, disables optional signal monitors, resets intervals, selects English and disables the dock. Both require confirmation and then Apply/Save; Cancel discards staged resets. No WAV files are deleted. Applying a full reset also clears pause/history.

## OBS notification dock

Enable General → Show notification panel inside OBS and click Apply. Show/hide it under **Docks → OBS Companion**. Drag the panel to dock or float it. It shows up to 30 timestamped events, monitoring state and pause/resume controls.

## Video feature retirement

Black-screen and unchanged-image monitoring have been removed from the normal build. Old video settings are ignored and removed; no video sampling runs. Audio/replay monitoring is unchanged. The supplied black_screen/frozen_screen voice files are reserved for future experimental work and do not produce alerts in this version.

## Feedback and contact

Please include your OBS, Windows and plugin versions, steps to reproduce, expected and actual results, and relevant logs with private information removed. For video issues, include SDR/HDR, scene contents, overlays and the live diagnostic values.

- **Discord:** `lvnula`
- **Email:** `lvnula@proton.me`

## Support & Donations

Donations are optional and never required to use the plugin. If you would like to support development:

- **PayPal:** [paypal.me/darioh07](https://www.paypal.com/paypalme/darioh07)
- **Other platforms or cryptocurrency:** contact me using Discord or email above.

No cryptocurrency payment address is currently published. The plugin is free to use.

## Build from source

See [docs/BUILDING.md](docs/BUILDING.md) for GitHub Actions and local Windows build instructions.

The source repository must include `data`: those resources are needed at compile time even though users install only the DLL. All four `audio` language folders are embedded during compilation. Supply your recordings before building.

## License and credits

GPL-2.0-or-later; see [LICENSE](LICENSE). Created by Nula / Attentionn with AI assistance. Based on the official OBS plugin template; see [TEMPLATE-PROVENANCE.md](TEMPLATE-PROVENANCE.md). Default tones are included in the project; optional voice recordings are credited above. Voice-pack distribution must comply with the applicable ElevenLabs license and voice permissions.
