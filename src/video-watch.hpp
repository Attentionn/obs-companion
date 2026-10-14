// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <obs.h>
#include <obs-frontend-api.h>
#include <util/platform.h>
#include <mutex>
#include "watchdog.hpp"
struct VideoWatch {
 bool blackEnabled=false, freezeEnabled=false;
 int blackDelay=10, freezeDelay=30, cooldown=60, darkLevel=16, darkPercent=98;
 double stillThreshold=0.5;
 enum class State { Disabled, Waiting, NoOutput, InvalidInfo, ConnectFailed, Connected, Stale };
 State state=State::Disabled; bool connected=false; video_t *output=nullptr;
 uint64_t retryAfter=0;
 std::mutex mutex; FrameSample previous{}; bool hasPrevious=false;
 double black=0,difference=255; uint64_t sampledAt=0, generation=0, consumed=0;
 uint64_t connectedAt=0, lastDiagnostic=0;
 SustainedAlert blackAlert, freezeAlert;
 ~VideoWatch() { stop(); }
 static void receive(void *param,video_data *frame) {
  auto *self=static_cast<VideoWatch *>(param);
  if(!frame || !frame->data[0]) return;
  std::lock_guard<std::mutex> lock(self->mutex);
  FrameSample current{};
  auto metrics=analyzeFrame(frame->data[0],frame->linesize[0],current,
   self->hasPrevious ? &self->previous : nullptr,static_cast<unsigned>(self->darkLevel));
  self->previous=current; self->hasPrevious=true;
  self->black=metrics.darkPercent; self->difference=metrics.difference;
  self->sampledAt=os_gettime_ns(); ++self->generation;
 }
 void stop() {
  if(connected && output) video_output_disconnect(output,receive,this);
  connected=false; output=nullptr; connectedAt=0; lastDiagnostic=0; retryAfter=0; state=State::Disabled;
  std::lock_guard<std::mutex> lock(mutex);
  hasPrevious=false; sampledAt=0; generation=0; consumed=0;
  blackAlert.reset(); freezeAlert.reset();
 }
 struct Snapshot { State state=State::Disabled; bool connected=false,fresh=false; double dark=0,change=255; std::int64_t blackSeconds=0,staticSeconds=0; };
 Snapshot snapshot() {
  std::lock_guard<std::mutex> lock(mutex);
  const auto clock=static_cast<std::int64_t>(os_gettime_ns()/1000000000ULL);
  return {state,connected,sampledAt && os_gettime_ns()-sampledAt<=3000000000ULL,black,difference,
   blackAlert.since ? clock-*blackAlert.since : 0,freezeAlert.since ? clock-*freezeAlert.since : 0};
 }
 void reset() { stop(); }
 std::array<bool,2> step(std::int64_t now,bool capture) {
  if(!capture || (!blackEnabled && !freezeEnabled)) { if(connected) stop(); state=State::Disabled; blackAlert.reset(); freezeAlert.reset(); return {false,false}; }
  if(!connected) {
   state=State::Waiting;
   const auto clock=os_gettime_ns(); if(clock<retryAfter) return {false,false};
   output=obs_get_video(); if(!output) { state=State::NoOutput; retryAfter=clock+1000000000ULL; return {false,false}; }
   auto *info=video_output_get_info(output); if(!info || !info->fps_num || !info->fps_den) { state=State::InvalidInfo; retryAfter=clock+1000000000ULL; return {false,false}; }
   const uint32_t divisor=std::max(1U,(info->fps_num+info->fps_den-1)/info->fps_den);
   video_scale_info conversion{}; conversion.format=VIDEO_FORMAT_BGRA;
   conversion.width=frameWidth; conversion.height=frameHeight;
   conversion.range=VIDEO_RANGE_FULL; conversion.colorspace=VIDEO_CS_709;
   connected=video_output_connect2(output,&conversion,divisor,receive,this);
   connectedAt=os_gettime_ns();
   if(!connected) { state=State::ConnectFailed; retryAfter=clock+2000000000ULL;
    if(!lastDiagnostic || clock-lastDiagnostic>10000000000ULL) { blog(LOG_WARNING,"[OBS Companion] Could not connect to OBS video output"); lastDiagnostic=clock; }
    return {false,false};
   }
   state=State::Connected; blog(LOG_INFO,"[OBS Companion] Video monitor connected to OBS final output at %ux%u; sampling %ux%u",info->width,info->height,frameWidth,frameHeight);
  }
  std::lock_guard<std::mutex> lock(mutex);
  if(!sampledAt || os_gettime_ns()-sampledAt>3000000000ULL) {
   const auto clock=os_gettime_ns();
   if(connectedAt && clock-connectedAt>5000000000ULL && (!lastDiagnostic || clock-lastDiagnostic>60000000000ULL)) {
    blog(LOG_WARNING,"[OBS Companion] Video monitor connected but no recent samples; verify active output and video format"); lastDiagnostic=clock;
   }
   state=State::Stale; blackAlert.reset(); freezeAlert.reset(); return {false,false};
  }
  // Each sample is consumed once; duration uses wall clock, never duplicate frames.
  state=State::Connected;
  if(consumed==generation) return {false,false};
  consumed=generation;
  const bool isBlack=black>=darkPercent;
  return {blackAlert.step(now,blackEnabled,isBlack,blackDelay,cooldown),
   freezeAlert.step(now,freezeEnabled,!isBlack && difference<=stillThreshold,freezeDelay,cooldown)};
 }
};
