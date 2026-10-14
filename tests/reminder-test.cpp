#include "../src/reminder.hpp"
#include <cassert>
#include <iostream>
int main() {
 Reminder r;
 assert(!r.step(100,true,true,60,60));
 assert(!r.step(159,true,true,60,60));
 assert(r.step(160,true,true,60,60));
 assert(!r.step(160,true,true,60,60));
 assert(r.step(220,true,true,60,60));
 assert(!r.step(221,false,true,60,60));
 assert(!r.step(300,true,true,60,60));
 assert(!r.step(301,true,false,60,60));
 assert(!r.step(400,true,true,60,60));
 assert(!r.step(459,true,true,60,60));
 assert(r.step(460,true,true,60,60));
 r.reset(); assert(!r.step(500,true,true,30,10));
 assert(r.step(530,true,true,30,10));
 assert(!r.step(539,true,true,30,10)); assert(r.step(540,true,true,30,10));
 std::cout << "Reminder behavior passed\n";
}
