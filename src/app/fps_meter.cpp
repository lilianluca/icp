#include "fps_meter.hpp"
#include <chrono>

FpsMeter::FpsMeter(std::chrono::duration<double> update_interval)
    : interval(update_interval) {}

void FpsMeter::update() {
  frame_count++;

  Clock::time_point now = Clock::now();
  std::chrono::duration<double> elapsed = now - last_time;
  if (elapsed >= interval) {
    fps = static_cast<double>(frame_count) / elapsed.count();
    frame_count = 0;
    updated = true;
    last_time = now;
  } else {
    updated = false;
  }
}

bool FpsMeter::is_updated() const { return updated; }

double FpsMeter::get() const { return fps; }

double FpsMeter::frame_time_ms() const {
  if (fps == 0) {
    return 0.0;
  }
  return 1000 / fps;
}