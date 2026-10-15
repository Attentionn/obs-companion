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
// Mute state changes precede duration-based audio reminders.
inline std::optional<std::size_t> chooseAlert(const std::array<bool,10> &pending,const std::array<bool,10> &audible) {
 for(std::size_t i:std::array<std::size_t,5>{1,2,3,6,7}) if(pending[i] && audible[i]) return i;
 return std::nullopt;
}
