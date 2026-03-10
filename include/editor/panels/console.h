#pragma once

#include <mutex>
#include <string>
#include <vector>

struct ConsoleMessage {
  enum class Level : uint8_t { Trace, Debug, Info, Warn, Error, Fatal };
  Level level;
  std::string message;
};

class ConsolePanel {
 public:
  ConsolePanel() = default;

  void onImGuiRender();

  static void pushMessage(ConsoleMessage::Level level, const std::string& message);

  static void clear();

 private:
  static std::vector<ConsoleMessage> s_messages;
  static std::mutex s_mutex;
  static uint32_t s_message_buffer_capacity;

  ConsoleMessage::Level m_filter_level = ConsoleMessage::Level::Info;
  bool m_auto_scroll = true;
};