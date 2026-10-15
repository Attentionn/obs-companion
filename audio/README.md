# Embedded voice recordings

Recordings by Nula / Attentionn, generated with ElevenLabs and edited in FL Studio.

The source folders are `audio/en`, `audio/esp`, `audio/fr`, and `audio/cn`. All four complete language packs are required when compiling. End users select the embedded presets and do not need external audio folders.

Each folder must contain these PCM16 mono/stereo WAV files (maximum 4 MiB each):

- `replay.wav`
- `mute.wav`
- `unmute.wav`
- `reminder.wav`
- `replay_start.wav`
- `replay_stop.wav`
- `mic_silent.wav`
- `desktop_silent.wav`
- `black_screen.wav`
- `frozen_screen.wav`

All four packs are compiled into the DLL. Users select Minimalist tones or Voices in General → Sound presets. Voice language can follow the interface or be selected independently. Custom per-event WAVs with any filename remain supported and take priority.

black_screen and frozen_screen are reserved: video monitoring is retired from the normal build, so these two events are not exposed or played.
