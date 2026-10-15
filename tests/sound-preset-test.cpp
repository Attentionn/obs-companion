#include "../src/sound-preset.hpp"
#include <cassert>
#include <iostream>
int main() {
 assert(voiceFolder("auto","es","en-US")=="esp");
 assert(voiceFolder("auto","auto","zh-CN")=="cn");
 assert(voiceFolder("fr","en","en-US")=="fr");
 assert(voiceFolder("auto","auto","de-DE")=="en");
 assert(presetResource("minimal","es","en","en-US","replay")==":/companion/sounds/replay.wav");
 assert(presetResource("voice","es","en","en-US","mute")==":/companion/voices/esp/mute.wav");
 assert(presetResource("voice","auto","fr","en-US","reminder")==":/companion/voices/fr/reminder.wav");
 std::cout<<"Preset/language resolution passed\n";
}
