# Building and publishing on Windows x64

## Build the release ZIP locally

Open a Visual Studio 2022 Developer PowerShell with PowerShell 7.2+, Git, CMake 3.30.5+, the x64 C++ tools and a Windows SDK 10.0.20348 or newer. The first build downloads the OBS 31.1.1 sources, Qt and OBS dependencies pinned in `buildspec.json`.

Run this command from the repository root:

```powershell
./.github/scripts/Build-Windows.ps1 -Target x64 -Configuration Release -Package
```

The output is `release/obs-companion-windows-x64.zip`. Open it to find `obs-companion.dll`, the license, installation instructions, README, changelog and release notes at the top level. The four voice packs and translations are inside the DLL.

## Build in GitHub Actions

Choose **Actions → Windows build → Run workflow**, select the branch and run it. With GitHub CLI, the equivalent from this checkout is:

```powershell
gh workflow run windows.yml --ref main
```

Open that run, check the **build** and **reminder-tests** results, and download **obs-companion-windows-x64.zip** under **Artifacts**. It contains the same files as the local package.

## Publish a tested commit

1. Try the DLL in OBS. Check replay saved, mute/unmute, a timed reminder across scene changes, a voice preset and the notification dock. For the desktop silence check, toggle the microphone during the delay and confirm the desktop warning still fires on time.
2. Push a version tag matching `buildspec.json` on the tested commit (for example, `v1.1.0`).
3. The tag workflow runs the tests, builds the ZIP, checks the downloaded archive and attaches it to a draft release. Open the draft in **Releases**, review its notes and asset, select **Set as the latest release**, and publish.

To check the source resources independently, run `python scripts/check-package.py` with Python 3.9+. It validates all four language catalogs and the embedded WAV files.
