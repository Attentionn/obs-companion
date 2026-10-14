// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <obs.h>
#include <obs-audio-controls.h>
#include <atomic>
#include <algorithm>
#include <cmath>
#include <QString>
#include "watchdog.hpp"
struct AudioWatch {
 QString name; bool enabled=false; int threshold=-50, delay=30, cooldown=60;
 obs_volmeter_t *meter=nullptr; obs_weak_source_t *weak=nullptr;
 std::atomic<float> peak{-1000.0f}; SustainedAlert alert;
 AudioWatch() { meter=obs_volmeter_create(OBS_FADER_LOG); if(meter) obs_volmeter_add_callback(meter, measured, this); }
 ~AudioWatch() { clear(); if(meter) { obs_volmeter_remove_callback(meter,measured,this); obs_volmeter_destroy(meter); } }
 static void measured(void *param,const float[],const float values[],const float[]) {
  auto *self=static_cast<AudioWatch *>(param); float largest=-1000;
  for(size_t i=0;i<MAX_AUDIO_CHANNELS;++i) if(std::isfinite(values[i])) largest=std::max(largest,values[i]);
  float old=self->peak.load(); while(largest>old && !self->peak.compare_exchange_weak(old,largest)) {}
 }
 void clear() {
  if(meter) obs_volmeter_detach_source(meter);
  if(weak) obs_weak_source_release(weak);
  weak=nullptr;
  peak.store(-1000); alert.reset();
 }
 // Called only on OBS's UI thread; callbacks only update the atomic peak.
 bool step(std::int64_t now,bool capture) {
  if(!enabled || !capture || name.isEmpty()) { clear(); return false; }
  obs_source_t *source=obs_get_source_by_name(name.toUtf8().constData());
  obs_source_t *attached=weak ? obs_weak_source_get_source(weak) : nullptr;
  bool same=source && source==attached; if(attached) obs_source_release(attached);
  if(!same) {
   clear();
   if(source && meter && obs_volmeter_attach_source(meter,source)) weak=obs_source_get_weak_source(source);
  }
  bool muted=source && obs_source_muted(source);
  bool available=source && weak && obs_source_enabled(source);
  if(source) obs_source_release(source);
  float level=peak.exchange(-1000);
  // Only attached, enabled, unmuted sources are monitored. Missing devices are not diagnosed.
  return alert.step(now,!muted && available,level<static_cast<float>(threshold),delay,cooldown);
 }
};
