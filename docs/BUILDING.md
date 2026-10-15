# Building and publishing 1.1.0

## GitHub Desktop and GitHub Actions

1. Preserve earlier development versions on your existing branch or repository.
2. Put this source package's contents directly in the new repository root. Do not nest them inside another project folder.
3. Keep `.github`, `cmake`, `build-aux`, `src`, `data`, `scripts`, `tests`, `docs`, `audio`, the CMake files, `buildspec.json`, `.gitignore`, license and public documentation.
4. Add all 40 PCM16 WAV recordings under `audio/en`, `audio/esp`, `audio/fr` and `audio/cn` before building. Filenames are listed in audio/README.md. All recordings are embedded in the DLL.
5. In GitHub Desktop, review Changes, commit and push to `main`.
6. In GitHub, open Actions → Windows build → Run workflow → choose `main`. Wait for the Windows build and automated tests to succeed.
7. Download the Windows x64 artifact. Test its DLL in OBS, including Apply, Cancel after a staged reset, both reset controls, pause/resume, replay events, audio checks, preset language selection and custom overrides.
8. Create and push tag `v1.1.0` on the tested commit. The tag workflow builds again and prepares a draft Release with the compiled ZIP and the release notes.
9. Review the draft, attach any additional permitted audio files if needed, and publish. The draft is not published automatically.

The binary ZIP contains the DLL, license, installation instructions, README, changelog and release notes. No external voice folder is needed. GitHub also supplies source-code downloads separately.

Use the clean package as the new public branch contents. Keep earlier development files in your archive branch. Do not copy old release ZIPs, build folders, local notes or state files into the new source root. Preserve build infrastructure and source resources.

## Local Windows build

Dependencies: Visual Studio 2022 C++ build tools, Windows SDK 10.0.22621, Git and CMake 3.30.5 or later. The template pins OBS 31.1.1 and its prebuilt dependencies.

```powershell
python scripts/check-package.py
cmake --preset windows-x64
cmake --build --preset windows-x64 --config Release
cmake --install build_x64 --config Release --prefix release/Release
```

The staged DLL is `release/Release/obs-companion/bin/64bit/obs-companion.dll`.

Automated tests cover reminder timing, cooldown/reset and preset language/resource resolution. Syntax checks and core tests do not replace a Windows link/build or OBS runtime test.

Source-only checks without recordings: `python scripts/check-package.py --allow-missing-voices`. This does not compile Qt resources; normal CI requires all 40 real WAVs. No placeholder speech is included.

When updating an existing checkout, remove the retired `src/video-watch.hpp` and preserve your own `audio` recordings.
