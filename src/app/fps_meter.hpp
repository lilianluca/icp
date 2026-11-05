#pragma once

#include <chrono>

// Měří počet snímků za sekundu (FPS), hodnota se přepočítá jednou za interval
class FpsMeter {
public:
  explicit FpsMeter(
      std::chrono::duration<double> update_interval = std::chrono::seconds(1));

  void update();                // volat jednou za snímek (na konci smyčky)
  bool is_updated() const;      // byla právě spočítaná nová hodnota?
  double get() const;           // poslední spočítané FPS
  double frame_time_ms() const; // průměrná doba jednoho snímku v milisekundách

private:
  using Clock = std::chrono::steady_clock;

  std::chrono::duration<double> interval;
  Clock::time_point last_time = Clock::now();
  int frame_count = 0;
  double fps = 0.0;
  bool updated = false;
};