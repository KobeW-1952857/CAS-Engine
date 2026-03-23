#include "core/simulation_clock.h"

void SimulationClock::advanceFrame(float wall_dt) {
  m_pending_ticks = 0;
  if (m_state != State::Playing) return;

  if (!use_fixed) {
    m_tick_dt = wall_dt * speed;
    m_sim_time += m_tick_dt;
    m_pending_ticks = 1;
    return;
  }

  m_accumulator += wall_dt * speed;
  while (m_accumulator >= fixed_dt) {
    m_accumulator -= fixed_dt;
    m_sim_time += fixed_dt;
    m_pending_ticks++;
  }
  m_tick_dt = fixed_dt;
}

void SimulationClock::step() {
  m_sim_time += fixed_dt;
  m_tick_dt = fixed_dt;
  m_pending_ticks = 1;
}