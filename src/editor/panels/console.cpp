#include "editor/panels/console.h"

#include <imgui.h>

std::vector<ConsoleMessage> ConsolePanel::s_messages;
std::mutex ConsolePanel::s_mutex;
uint32_t ConsolePanel::s_message_buffer_capacity = 1000;

void ConsolePanel::pushMessage(ConsoleMessage::Level level, const std::string& message) {
  std::lock_guard<std::mutex> lock(s_mutex);
  s_messages.push_back({level, message});
  if (s_messages.size() > s_message_buffer_capacity) {
    s_messages.erase(s_messages.begin(), s_messages.begin() + 100);
  }
}

void ConsolePanel::clear() {
  std::lock_guard<std::mutex> lock(s_mutex);
  s_messages.clear();
}

void ConsolePanel::onImGuiRender() {
  ImGui::Begin("Console");
  const char* levelStrings[] = {"Trace", "Debug", "Info", "Warn", "Error", "Fatal"};
  if (ImGui::Button("Clear")) clear();
  ImGui::SameLine();
  ImGui::Checkbox("Auto-Scroll", &m_auto_scroll);
  ImGui::SameLine();

  static float combo_width = 0.0f;
  if (combo_width == 0.0f) {
    float max_text_width = 0.0f;
    for (int i = 0; i < IM_ARRAYSIZE(levelStrings); i++) {
      float w = ImGui::CalcTextSize(levelStrings[i]).x;
      if (w > max_text_width) max_text_width = w;
    }
    combo_width = max_text_width + ImGui::GetFrameHeight() + (ImGui::GetStyle().ItemInnerSpacing.x * 2.0f);
  }

  ImGui::SetNextItemWidth(combo_width);
  if (ImGui::BeginCombo("##Level", levelStrings[static_cast<int>(m_filter_level)])) {
    for (int i = 0; i < IM_ARRAYSIZE(levelStrings); i++) {
      bool isSelected = (static_cast<int>(m_filter_level) == i);

      if (ImGui::Selectable(levelStrings[i], isSelected)) {
        m_filter_level = static_cast<ConsoleMessage::Level>(i);
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

  ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

  std::lock_guard<std::mutex> lock(s_mutex);

  std::vector<const ConsoleMessage*> filtered_messages;
  filtered_messages.reserve(s_messages.size());

  for (const auto& msg : s_messages) {
    if (msg.level >= m_filter_level) {
      filtered_messages.push_back(&msg);
    }
  }

  ImGuiListClipper clipper;
  clipper.Begin(filtered_messages.size());
  while (clipper.Step()) {
    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
      const auto& msg = filtered_messages[i];
      ImVec4 color;
      switch (filtered_messages[i]->level) {
        case ConsoleMessage::Level::Trace:
          color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
          break;
        case ConsoleMessage::Level::Debug:
          color = ImVec4(0.4f, 0.7f, 0.7f, 1.0f);
          break;
        case ConsoleMessage::Level::Info:
          color = ImVec4(0.0f, 0.7f, 0.0f, 1.0f);
          break;
        case ConsoleMessage::Level::Warn:
          color = ImVec4(0.7f, 0.7f, 0.0f, 1.0f);
          break;
        case ConsoleMessage::Level::Error:
          color = ImVec4(0.7f, 0.3f, 0.3f, 1.0f);
          break;
        case ConsoleMessage::Level::Fatal:
          color = ImVec4(0.7f, 0.1f, 0.1f, 1.0f);
          break;
      }
      ImGui::PushStyleColor(ImGuiCol_Text, color);
      ImGui::TextUnformatted(msg->message.c_str());
      ImGui::PopStyleColor();
    }
  }

  if (m_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);

  ImGui::EndChild();
  ImGui::End();
}