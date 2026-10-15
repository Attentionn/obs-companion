// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <string>
#include <string_view>
inline std::string voiceFolder(std::string_view voice, std::string_view ui, std::string_view obsLocale) {
    auto code=voice=="auto" ? (ui=="auto" ? obsLocale : ui) : voice;
    code=code.substr(0,2);
    if(code=="es") return "esp";
    if(code=="fr") return "fr";
    if(code=="zh") return "cn";
    return "en";
}
inline std::string presetResource(std::string_view style,std::string_view voice,std::string_view ui,std::string_view obsLocale,std::string_view event) {
    return (style=="voice" ? ":/companion/voices/"+voiceFolder(voice,ui,obsLocale)+"/" : ":/companion/sounds/")+std::string(event)+".wav";
}
