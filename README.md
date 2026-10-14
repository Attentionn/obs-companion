# OBS Companion 1.0.0

A free Windows x64 plugin for OBS Studio, created by **Nula / Attentionn**.

## Features

- Audio confirmation when OBS saves a replay, using your existing hotkey.
- Replay Buffer start and stop notifications.
- Microphone mute/unmute sounds and timed muted reminders.
- Optional microphone and desktop no-signal warnings with separate thresholds, delays and repeat intervals.
- Experimental black-screen and unchanged-image monitoring.
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

## Optional voice packs and custom sounds

Optional recordings created by **Nula / Attentionn using ElevenLabs**, with editing in FL Studio, can be kept in:

| Folder | Language |
| --- | --- |
| `audio/en/` | English |
| `audio/esp/` | Spanish |
| `audio/fr/` | French |
| `audio/cn/` | Chinese |

These folder names are independent of the interface's translation codes. See [audio/README.md](audio/README.md) for exact filenames.

Extract `audio` to a permanent location of your choice. In **Tools → OBS Companion**, open the relevant tab → **Customize sounds**, choose an event, click **Browse**, select its WAV, then **Test** and **Apply**. Changing the interface language does not automatically select a voice pack. Leave the path blank to use the built-in tone. You can also use your own recordings.

Custom sounds must be **16-bit PCM WAV**, mono or stereo, at most **4 MiB per file**. MP3 and 24/32-bit WAV are unsupported. Do not move files after selecting them. Sounds play through Windows's default output and may be captured by Desktop Audio; simultaneous alerts can interrupt one another.

## Experimental video monitoring

Black-screen and unchanged-image checks are **off by default and experimental**. They sample the final OBS output, not individual game sources. Dark scenes or static menus may trigger warnings, while moving overlays may hide a frozen game. These checks cannot confirm a capture failure.

Default black detection requires 98% of sampled pixels at brightness 16/255 or below for 10 seconds. Unchanged-image detection uses an average brightness difference of 0.5/255 or below for 30 seconds. The default repeat interval is 60 seconds.

To test: enable a video warning, click **Apply**, start Replay Buffer or recording, and show a full-frame black scene for the selected delay. The Video tab shows connection status, dark-pixel percentage and elapsed times. Missing frames are reported as a sampling problem, not automatically classified as a frozen image. Check **Help → Log Files → View Current Log** for `OBS Companion` diagnostics. Windows runtime confirmation and HDR testing remain outstanding.

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

The source repository must include `data`: those resources are needed at compile time even though users install only the DLL. The optional `audio` folder is distributed alongside the plugin ZIP and is not embedded in the DLL.

## License and credits

GPL-2.0-or-later; see [LICENSE](LICENSE). Created by Nula / Attentionn with AI assistance. Based on the official OBS plugin template; see [TEMPLATE-PROVENANCE.md](TEMPLATE-PROVENANCE.md). Default tones are included in the project; optional voice recordings are credited above. Voice-pack distribution must comply with the applicable ElevenLabs license and voice permissions.
