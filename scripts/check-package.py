"""Fail early when files, embedded resources, translations or git tracking are missing."""
from pathlib import Path
import json, subprocess, wave, xml.etree.ElementTree as ET
root = Path(__file__).resolve().parents[1]
required = ['INSTALL.md','README.md','LICENSE','CHANGELOG.md','docs/BUILDING.md','docs/RELEASE-NOTES.md','audio/README.md',
            'tests/reminder-test.cpp','tests/watchdog-test.cpp','src/resources.qrc',
            'src/plugin-main.cpp','src/audio-watch.hpp','src/video-watch.hpp','src/watchdog.hpp']
resources = ET.parse(root/'src/resources.qrc').getroot()
for item in resources.iter('file'):
    file = (root/'src'/item.text).resolve()
    assert file.is_relative_to(root), f'Resource outside project: {file}'
    assert file.is_file(), f'Missing resource: {file}'
    required.append(file.relative_to(root).as_posix())
    if file.suffix == '.wav':
        with wave.open(str(file)) as sound:
            assert sound.getsampwidth()==2 and sound.getnchannels() in (1,2)
        assert file.stat().st_size<=4*1024*1024
translations = [json.loads((root/'data/i18n'/f'{lang}.json').read_text(encoding='utf-8')) for lang in ['en','es','zh','fr']]
assert all(set(t)==set(translations[0]) and all(t.values()) for t in translations)
for name in required:
    assert (root/name).is_file(), f'Missing project file: {name}'
    result=subprocess.run(['git','check-ignore','--no-index','-q',name],cwd=root)
    assert result.returncode==1, f'gitignore excludes required source: {name}'
print(f'Package preflight passed: {len(required)} files, four complete language catalogs, ten WAV resources')

# Voice packs are optional; a supplied language pack must be complete and compatible.
events = ['replay','mute','unmute','reminder','replay_start','replay_stop','mic_silent','desktop_silent','black_screen','frozen_screen']
for lang in ['cn','esp','fr','en']:
    folder=root/'audio'/lang
    supplied=list(folder.glob('*.wav'))
    if not supplied:
        continue
    assert {p.name for p in supplied}=={event+'.wav' for event in events}, f'Incomplete voice pack: {lang}'
    for file in supplied:
        with wave.open(str(file)) as sound:
            assert sound.getcomptype()=='NONE' and sound.getsampwidth()==2 and sound.getnchannels() in (1,2), f'Use PCM16 mono/stereo: {file}'
        assert file.stat().st_size<=4*1024*1024, f'Voice file exceeds 4 MiB: {file}'
print('Optional voice-pack validation passed')
