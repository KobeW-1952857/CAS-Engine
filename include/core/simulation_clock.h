#pragma once

#include <cstdint>

class SimulationClock {
 public:
  enum class State : uint8_t { Stopped, Playing, Paused };

  void advanceFrame(float wall_dt);
  void step();
  void play() { m_state = State::Playing; }
  void pause() { m_state = State::Paused; }
  void stop() {
    m_state = State::Stopped;
    reset();
  }

  uint64_t pendingTicks() const { return m_pending_ticks; }
  float simTime() const { return m_sim_time; }
  float tickDt() const { return m_tick_dt; }
  bool isPlaying() const { return m_state == State::Playing; }
  bool isPaused() const { return m_state == State::Paused; }
  bool isStopped() const { return m_state == State::Stopped; }
  State state() const { return m_state; }

  float speed = 1.0f;
  float fixed_dt = 1.0f / 60.0f;
  float use_fixed = false;

 private:
  void reset() {
    m_accumulator = 0.0f;
    m_sim_time = 0.0f;
    m_pending_ticks = 0;
  }

  State m_state = State::Stopped;
  float m_accumulator = 0.0f;
  float m_sim_time = 0.0f;
  uint64_t m_pending_ticks = 0;
  float m_tick_dt = 0.0f;
};