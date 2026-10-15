"""Fail early when files, embedded resources, translations or git tracking are missing."""
from pathlib import Path
import argparse, json, subprocess, wave, xml.etree.ElementTree as ET
parser=argparse.ArgumentParser()
parser.add_argument("--allow-missing-voices", action="store_true", help="Validate source-only handoff without user recordings; never use for a release build")
args=parser.parse_args()
missing=[]
root = Path(__file__).resolve().parents[1]
required = ['INSTALL.md','README.md','LICENSE','CHANGELOG.md','docs/BUILDING.md','docs/RELEASE-NOTES.md','audio/README.md',
            'tests/reminder-test.cpp','tests/watchdog-test.cpp','tests/sound-preset-test.cpp','src/resources.qrc',
            'src/plugin-main.cpp','src/audio-watch.hpp','src/sound-preset.hpp','src/watchdog.hpp']
resources = ET.parse(root/'src/resources.qrc').getroot()
for item in resources.iter('file'):
    file = (root/'src'/item.text).resolve()
    assert file.is_relative_to(root), f'Resource outside project: {file}'
    if not file.is_file() and file.is_relative_to(root/'audio'):
        missing.append(file.relative_to(root).as_posix())
        continue
    assert file.is_file(), f'Missing resource: {file}' 
    required.append(file.relative_to(root).as_posix())
    if file.suffix == '.wav':
        with wave.open(str(file)) as sound:
            assert sound.getcomptype()=="NONE" and sound.getsampwidth()==2 and sound.getnchannels() in (1,2)
        assert file.stat().st_size<=4*1024*1024
translations = [json.loads((root/'data/i18n'/f'{lang}.json').read_text(encoding='utf-8')) for lang in ['en','es','zh','fr']]
assert all(set(t)==set(translations[0]) and all(t.values()) for t in translations)
for name in required:
    assert (root/name).is_file(), f'Missing project file: {name}'
    result=subprocess.run(['git','check-ignore','--no-index','-q',name],cwd=root)
    assert result.returncode==1, f'gitignore excludes required source: {name}'
print(f'Package preflight passed: {len(required)} files, four complete language catalogs, embedded tone/voice resources')

if missing:
    print("Missing voice recordings (add your folders before building):\n"+"\n".join(missing))
    if not args.allow_missing_voices:
        raise SystemExit("Build requires all 40 voice WAVs under audio/en, audio/esp, audio/fr and audio/cn.")
    print("SOURCE-ONLY validation: Windows resource compilation still requires the missing recordings.")
else:
    print("All 40 embedded voice recordings validated")
