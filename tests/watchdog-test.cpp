#include "../src/watchdog.hpp"
#include <cassert>
#include <vector>
#include <iostream>
int main() {
 std::array<bool,10> pending{},soundOn{}; soundOn.fill(true);
 pending[3]=pending[6]=pending[7]=true;
 assert(chooseAlert(pending,soundOn)==3);
 pending[3]=false; assert(chooseAlert(pending,soundOn)==6);
 soundOn[6]=false; assert(chooseAlert(pending,soundOn)==7);
 soundOn[7]=false; assert(!chooseAlert(pending,soundOn));
 SustainedAlert a;
 assert(!a.step(0,true,true,30,60)); assert(!a.step(29,true,true,30,60));
 assert(a.step(30,true,true,30,60)); assert(!a.step(31,true,true,30,60));
 assert(a.step(90,true,true,30,60));
 assert(!a.step(91,true,false,30,60)); assert(!a.step(100,true,true,30,60));
 assert(!a.step(129,true,true,30,60)); assert(a.step(130,true,true,30,60));
 assert(!a.step(131,false,true,30,60)); assert(!a.step(300,true,true,30,60));
 a.reset(); assert(!a.step(500,true,true,10,20)); assert(a.step(510,true,true,10,20));
 std::cout<<"Audio duration/cooldown checks passed\n";
}
