#include "../src/watchdog.hpp"
#include <cassert>
#include <vector>
#include <iostream>
int main() {
 std::array<bool,10> pending{},soundOn{}; soundOn.fill(true);
 pending[3]=pending[6]=pending[7]=pending[8]=true;
 assert(chooseAlert(pending,soundOn)==8);
 soundOn[8]=false; assert(chooseAlert(pending,soundOn)==3);
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
 const unsigned stride=frameWidth*4+16; std::vector<unsigned char> pixels(stride*frameHeight,0);
 FrameSample old{},current{};
 auto m=analyzeFrame(pixels.data(),stride,current,nullptr,8); assert(m.darkPercent==100 && m.difference==255);
 old=current; m=analyzeFrame(pixels.data(),stride,current,&old,8); assert(m.difference==0);
 for(unsigned y=0;y<frameHeight;++y) for(unsigned x=0;x<frameWidth;++x) {
  auto *v=pixels.data()+y*stride+x*4; v[0]=v[1]=v[2]=255; v[3]=255;
 }
 m=analyzeFrame(pixels.data(),stride,current,&old,8); assert(m.darkPercent==0 && m.difference==255);
 old=current; m=analyzeFrame(pixels.data(),stride,current,&old,8); assert(m.difference==0);
 // Limited-range black near luma 16 must count as black at the default threshold.
 for(unsigned y=0;y<frameHeight;++y) for(unsigned x=0;x<frameWidth;++x) {
  auto *v=pixels.data()+y*stride+x*4; v[0]=v[1]=v[2]=16; v[3]=255;
 }
 m=analyzeFrame(pixels.data(),stride,current,nullptr,16); assert(m.darkPercent==100);
 // Transparent black is still visually black; alpha is not treated as luminance.
 for(auto &v:pixels) v=0;
 m=analyzeFrame(pixels.data(),stride,current,&old,8); assert(m.darkPercent==100);
 std::cout<<"Silence duration/cooldown and padded BGRA frame checks passed\n";
}
