#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

namespace ImGui {
static inline bool CurveEditBezier(ImVec2 from, ImVec2 to, ImVec2* control_points, int control_point_count) {
  ImGui::BeginChild("CurveEditor", ImVec2(0, 200), ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders,
                    ImGuiWindowFlags_NoScrollbar);
  ImVec2 origin = ImGui::GetWindowPos();
  ImVec2 size = ImGui::GetWindowSize();
  auto* dl = ImGui::GetWindowDrawList();
  ImVec2 padding = ImGui::GetStyle().WindowPadding;
  bool changed = false;

  auto transform = [&](const ImVec2& v) -> ImVec2 {
    return ImVec2(origin.x + padding.x + v.x * (size.x - 2 * padding.x),
                  origin.y + size.y - padding.y - v.y * (size.y - 2 * padding.y));
  };
  auto invTransform = [&](const ImVec2& v) -> ImVec2 {
    return ImVec2((v.x - origin.x - padding.x) / (size.x - 2 * padding.x),
                  (origin.y + size.y - padding.y - v.y) / (size.y - 2 * padding.y));
  };

  auto AddDashedLine = [](ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thickness = 1.0f, float dash_len = 6.0f,
                          float gap_len = 4.0f) {
    ImVec2 dir = ImVec2(b.x - a.x, b.y - a.y);
    float total = sqrtf(dir.x * dir.x + dir.y * dir.y);
    dir = ImVec2(dir.x / total, dir.y / total);  // normalize

    float t = 0.0f;
    bool drawing = true;
    while (t < total) {
      float seg_len = drawing ? dash_len : gap_len;
      float t_end = ImMin(t + seg_len, total);
      if (drawing) {
        ImVec2 p0 = ImVec2(a.x + dir.x * t, a.y + dir.y * t);
        ImVec2 p1 = ImVec2(a.x + dir.x * t_end, a.y + dir.y * t_end);
        dl->AddLine(p0, p1, col, thickness);
      }
      t = t_end;
      drawing = !drawing;
    }
  };

  for (int i = 0; i < control_point_count; ++i) {
    float radius = 6.0f;
    auto point = transform(control_points[i]);

    ImGui::SetCursorScreenPos(ImVec2(point.x - radius, point.y - radius));
    ImGui::PushID(i);
    ImGui::InvisibleButton("pt", ImVec2(radius * 2, radius * 2));
    ImGui::PopID();

    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    if (active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.0f);
      ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);

      ImVec2 new_screen = invTransform(ImVec2(point.x + delta.x, point.y + delta.y));

      new_screen.x = ImClamp(new_screen.x, 0.0f, 1.0f);
      new_screen.y = ImClamp(new_screen.y, 0.0f, 1.0f);

      control_points[i] = new_screen;
      changed = true;
    }

    ImU32 col = active    ? IM_COL32(255, 200, 0, 255)
                : hovered ? IM_COL32(255, 255, 255, 255)
                          : IM_COL32(150, 150, 150, 255);
    dl->AddCircleFilled(point, radius, col);
  }

  dl->AddBezierCubic(transform(from), transform(control_points[0]), transform(control_points[1]), transform(to),
                     IM_COL32(0, 0, 0, 255), 2.0f);
  dl->AddCircle(transform(control_points[0]), 10.0f, IM_COL32(0, 0, 0, 100), 0, 2.0f);
  dl->AddCircle(transform(control_points[1]), 10.0f, IM_COL32(0, 0, 0, 100), 0, 2.0f);
  AddDashedLine(dl, transform(from), transform(control_points[0]), IM_COL32(0, 0, 0, 100));
  AddDashedLine(dl, transform(control_points[1]), transform(to), IM_COL32(0, 0, 0, 100));
  AddDashedLine(dl, transform(from), transform(to), IM_COL32(0, 0, 0, 50));

  // Draw grid
  for (int i = 0; i <= 10; ++i) {
    float t = i / 10.0f;
    dl->AddLine(ImVec2(origin.x + padding.x + t * (size.x - 2 * padding.x), origin.y + padding.y),
                ImVec2(origin.x + padding.x + t * (size.x - 2 * padding.x), origin.y + size.y - padding.y),
                IM_COL32(0, 0, 0, 100));
    dl->AddLine(ImVec2(origin.x + padding.x, origin.y + padding.y + t * (size.y - 2 * padding.y)),
                ImVec2(origin.x + size.x - padding.x, origin.y + padding.y + t * (size.y - 2 * padding.y)),
                IM_COL32(0, 0, 0, 100));
  }
  ImGui::EndChild();
  return changed;
}
}  // namespace ImGui