// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
struct SustainedAlert {
 std::optional<std::int64_t> since, next;
 void reset() { since.reset(); next.reset(); }
 bool step(std::int64_t now, bool eligible, bool condition, int delay, int cooldown) {
  if (!eligible || !condition) { reset(); return false; }
  if (!since) since=now;
  if (now-*since < delay || (next && now<*next)) return false;
  next=now+cooldown; return true;
 }
};
constexpr unsigned frameWidth=64, frameHeight=36;
using FrameSample=std::array<unsigned char,frameWidth*frameHeight>;
struct FrameMetrics { double darkPercent=0, difference=255; };
inline FrameMetrics analyzeFrame(const unsigned char *bgra, unsigned stride,
 FrameSample &current, const FrameSample *previous, unsigned darkLevel) {
 unsigned dark=0; double change=0;
 for(unsigned y=0;y<frameHeight;++y) for(unsigned x=0;x<frameWidth;++x) {
  const auto *v=bgra+y*stride+x*4;
  unsigned luma=(unsigned(v[2])*54+unsigned(v[1])*183+unsigned(v[0])*19)/256;
  unsigned i=y*frameWidth+x; current[i]=static_cast<unsigned char>(luma);
  if(luma<=darkLevel) ++dark;
  if(previous) change+=std::abs(int(current[i])-int((*previous)[i]));
 }
 return {100.0*dark/static_cast<double>(current.size()),previous ? change/static_cast<double>(current.size()) : 255.0};
}

// Video safety warnings precede muted, microphone-signal and desktop-signal reminders.
inline std::optional<std::size_t> chooseAlert(const std::array<bool,10> &pending,
 const std::array<bool,10> &audible) {
 for(std::size_t i:std::array<std::size_t,7>{8,9,1,2,3,6,7}) if(pending[i] && audible[i]) return i;
 return std::nullopt;
}
