#pragma once
#include <cstdint>
#include <optional>
struct Reminder {
    std::optional<std::int64_t> due;
    bool step(std::int64_t now, bool active, bool muted, int first, int repeat) {
        if (!active || !muted) { due.reset(); return false; }
        if (!due) due = now + first;
        if (now < *due) return false;
        due = now + repeat; return true;
    }
    void reset() { due.reset(); }
};
