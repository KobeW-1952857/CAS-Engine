#pragma once

#include <Nexus/Log.h>
#include <imgui.h>

#include <glm/glm.hpp>
#include <vector>

enum class ArcLengthStrategy : uint8_t {
  ForwardDifferencing,
  AdaptiveDifferencing,
};

struct ArcLengthEntry {
  float t;
  float arc_length;
};

struct ForwardDifferencingStrategy {
  int samples = 100;

  std::vector<ArcLengthEntry> operator()(auto&& evaluate) const {
    std::vector<ArcLengthEntry> table;
    table.reserve(samples + 1);
    table.push_back({0.0f, 0.0f});
    glm::vec3 prev_point = evaluate(0.0f);
    float accumulated_length = 0.0f;
    for (int i = 1; i <= samples; ++i) {
      float t = static_cast<float>(i) / samples;
      glm::vec3 point = evaluate(t);
      accumulated_length += glm::distance(prev_point, point);
      table.push_back({t, accumulated_length});
      prev_point = point;
    }
    return table;
  }

  bool drawUI() {
    ImGui::DragInt("Samples", &samples, 1, 10, 10000);
    return ImGui::IsItemDeactivatedAfterEdit();
  }
};

struct AdaptiveDifferencingStrategy {
  float tolerance = 0.001f;
  int max_depth = 8;

  std::vector<ArcLengthEntry> operator()(auto&& evaluate) const {
    std::vector<ArcLengthEntry> table;
    table.push_back({0.0f, 0.0f});

    subdivide(evaluate, 0.0f, evaluate(0.0f), 1.0f, evaluate(1.0f), 0.0f, 0, table);

    return table;
  }

  void subdivide(const auto& evaluate, float t0, glm::vec3 p0, float t1, glm::vec3 p1, float s0, int depth,
                 std::vector<ArcLengthEntry>& table) const {
    const float tm = (t0 + t1) * 0.5;
    const glm::vec3 pm = evaluate(tm);

    const float chord = glm::distance(p0, p1);
    const float split = glm::distance(p0, pm) + glm::distance(pm, p1);

    if (depth < max_depth && abs(split - chord) > tolerance) {
      subdivide(evaluate, t0, p0, tm, pm, s0, depth + 1, table);
      const float sm = table.back().arc_length;
      subdivide(evaluate, tm, pm, t1, p1, sm, depth + 1, table);
    } else {
      const float sm = s0 + glm::distance(p0, pm);
      table.push_back({tm, sm});
      table.push_back({t1, sm + glm::distance(pm, p1)});
    }
  }

  bool drawUI() {
    bool should_rebuild = false;
    ImGui::DragFloat("Tolerance", &tolerance, 1e-6f, 0.0f, 1.0f, "%.2e");
    should_rebuild |= ImGui::IsItemDeactivatedAfterEdit();

    ImGui::DragInt("Max Depth", &max_depth, 1, 1, 12);
    should_rebuild |= ImGui::IsItemDeactivatedAfterEdit();

    return should_rebuild;
  }
};

struct ArcLengthTable {
  ArcLengthStrategy strategy = ArcLengthStrategy::ForwardDifferencing;

  using Entry = ArcLengthEntry;

  template <typename EvaluateFn, typename StrategyFn>
  void build(const EvaluateFn& evaluate, const StrategyFn& strategy) {
    m_entries = strategy(evaluate);
  }

  void rebuild(auto&& evaluate) {
    switch (strategy) {
      case ArcLengthStrategy::ForwardDifferencing:
        m_entries = m_forward_differencing(evaluate);
        break;
      case ArcLengthStrategy::AdaptiveDifferencing:
        m_entries = m_adaptive_differencing(evaluate);
        break;
    }
  }

  float length() const { return m_entries.empty() ? 0.0f : m_entries.back().arc_length; }
  float tFromArcLength(float s) const {
    if (m_entries.empty()) return 0.0f;
    if (s <= 0.0f) return 0.0f;
    if (s >= m_entries.back().arc_length) return 1.0f;

    // Binary search for the two entries that straddle s
    auto it = std::lower_bound(m_entries.begin(), m_entries.end(), s,
                               [](const Entry& entry, float value) { return entry.arc_length < value; });

    if (it == m_entries.begin()) return 0.0f;
    if (it == m_entries.end()) return 1.0f;

    const Entry& upper = *it;
    const Entry& lower = *std::prev(it);

    // Linearly interpolate t between the two entries
    float range = upper.arc_length - lower.arc_length;
    float alpha = (s - lower.arc_length) / range;
    return alpha * upper.t + (1.0f - alpha) * lower.t;
  }

  void drawUI(auto&& evaluate) {
    const char* strategy_names[] = {"Forward Differencing", "Adaptive Differencing"};
    int current_strategy = static_cast<int>(strategy);
    if (ImGui::Combo("Strategy", &current_strategy, strategy_names, IM_ARRAYSIZE(strategy_names))) {
      strategy = static_cast<ArcLengthStrategy>(current_strategy);
      rebuild(evaluate);
    }
    bool needs_rebuild = false;
    switch (strategy) {
      case ArcLengthStrategy::ForwardDifferencing:
        needs_rebuild = m_forward_differencing.drawUI();
        break;
      case ArcLengthStrategy::AdaptiveDifferencing:
        needs_rebuild = m_adaptive_differencing.drawUI();
        break;
    }

    // ImGui::Text("Entries: %zu", m_entries.size());
    // ImGui::Text("Length: %.4f", length());
    // ImGui::Separator();
    // for (const auto& entry : m_entries) {
    //   ImGui::Text("%.4f %.4f", entry.t, entry.arc_length);
    // }

    if (needs_rebuild) rebuild(evaluate);
  }

 private:
  std::vector<Entry> m_entries;
  ForwardDifferencingStrategy m_forward_differencing;
  AdaptiveDifferencingStrategy m_adaptive_differencing;
};
