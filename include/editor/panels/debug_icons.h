// DebugIconWindow.h
#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include <string>

inline void drawDebugIconWindow(bool* open) {
  // if (!ImGui::Begin("Icon Browser", open)) {
  //   ImGui::End();
  //   return;
  // }

  // struct IconEntry {
  //   const char* name;
  //   const char* icon;
  // };

  // static constexpr IconEntry fa_icons[] = {
  //     // Scene / hierarchy
  //     {"ICON_FA_GLOBE", ICON_FA_GLOBE},
  //     {"ICON_FA_CUBE", ICON_FA_CUBE},
  //     {"ICON_FA_CUBES", ICON_FA_CUBES},
  //     {"ICON_FA_CAMERA", ICON_FA_CAMERA},
  //     {"ICON_FA_LIGHTBULB", ICON_FA_LIGHTBULB},
  //     {"ICON_FA_TAG", ICON_FA_TAG},
  //     {"ICON_FA_TAGS", ICON_FA_TAGS},
  //     {"ICON_FA_CIRCLE", ICON_FA_CIRCLE},
  //     {"ICON_FA_CIRCLE_DOT", ICON_FA_CIRCLE_DOT},
  //     // {"ICON_FA_PAINT_BRUSH", ICON_FA_PAINT_BRUSH},
  //     {"ICON_FA_EYE", ICON_FA_EYE},
  //     {"ICON_FA_EYE_SLASH", ICON_FA_EYE_SLASH},
  //     {"ICON_FA_LOCK", ICON_FA_LOCK},
  //     {"ICON_FA_UNLOCK", ICON_FA_UNLOCK},
  //     {"ICON_FA_FOLDER", ICON_FA_FOLDER},
  //     {"ICON_FA_FOLDER_OPEN", ICON_FA_FOLDER_OPEN},
  //     {"ICON_FA_FILE", ICON_FA_FILE},
  //     {"ICON_FA_PLAY", ICON_FA_PLAY},
  //     {"ICON_FA_PAUSE", ICON_FA_PAUSE},
  //     {"ICON_FA_STOP", ICON_FA_STOP},
  //     {"ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT", ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT},
  //     {"ICON_FA_ROTATE", ICON_FA_ROTATE},
  //     {"ICON_FA_EXPAND", ICON_FA_EXPAND},
  //     {"ICON_FA_TRASH", ICON_FA_TRASH},
  //     {"ICON_FA_COPY", ICON_FA_COPY},
  //     {"ICON_FA_PLUS", ICON_FA_PLUS},
  //     {"ICON_FA_MINUS", ICON_FA_MINUS},
  //     {"ICON_FA_XMARK", ICON_FA_XMARK},
  //     {"ICON_FA_CHECK", ICON_FA_CHECK},
  //     {"ICON_FA_GEAR", ICON_FA_GEAR},
  //     {"ICON_FA_GEARS", ICON_FA_GEARS},
  //     {"ICON_FA_WRENCH", ICON_FA_WRENCH},
  //     {"ICON_FA_BEZIER_CURVE", ICON_FA_BEZIER_CURVE},
  //     {"ICON_FA_DIAGRAM_PROJECT", ICON_FA_DIAGRAM_PROJECT},
  // };

  // static constexpr IconEntry md_icons[] = {
  //     {"ICON_MD_PUBLIC", ICON_MD_PUBLIC},
  //     {"ICON_MD_VIEW_IN_AR", ICON_MD_VIEW_IN_AR},
  //     {"ICON_MD_GRID_ON", ICON_MD_GRID_ON},
  //     {"ICON_MD_CAMERA_ALT", ICON_MD_CAMERA_ALT},
  //     {"ICON_MD_LIGHT_MODE", ICON_MD_LIGHT_MODE},
  //     {"ICON_MD_PALETTE", ICON_MD_PALETTE},
  //     {"ICON_MD_TEXTURE", ICON_MD_TEXTURE},
  //     {"ICON_MD_OPEN_WITH", ICON_MD_OPEN_WITH},
  //     {"ICON_MD_360", ICON_MD_360},
  //     {"ICON_MD_STRAIGHTEN", ICON_MD_STRAIGHTEN},
  //     {"ICON_MD_TIMELINE", ICON_MD_TIMELINE},
  //     {"ICON_MD_ANIMATION", ICON_MD_ANIMATION},
  //     {"ICON_MD_TUNE", ICON_MD_TUNE},
  //     {"ICON_MD_VISIBILITY", ICON_MD_VISIBILITY},
  //     {"ICON_MD_VISIBILITY_OFF", ICON_MD_VISIBILITY_OFF},
  //     {"ICON_MD_FOLDER", ICON_MD_FOLDER},
  //     {"ICON_MD_FOLDER_OPEN", ICON_MD_FOLDER_OPEN},
  //     {"ICON_MD_ADD", ICON_MD_ADD},
  //     {"ICON_MD_REMOVE", ICON_MD_REMOVE},
  //     {"ICON_MD_CLOSE", ICON_MD_CLOSE},
  //     {"ICON_MD_SETTINGS", ICON_MD_SETTINGS},
  //     {"ICON_MD_BUILD", ICON_MD_BUILD},
  //     {"ICON_MD_PLAY_ARROW", ICON_MD_PLAY_ARROW},
  //     {"ICON_MD_PAUSE", ICON_MD_PAUSE},
  //     {"ICON_MD_STOP", ICON_MD_STOP},
  //     {"ICON_MD_DELETE", ICON_MD_DELETE},
  //     {"ICON_MD_CONTENT_COPY", ICON_MD_CONTENT_COPY},
  //     {"ICON_MD_SAVE", ICON_MD_SAVE},
  //     {"ICON_MD_UNDO", ICON_MD_UNDO},
  //     {"ICON_MD_REDO", ICON_MD_REDO},
  //     {"ICON_MD_VIEW_GRID", "\uf570"},
  //     {"ICON_MD_VIEW_AGENDA", ICON_MD_VIEW_AGENDA},
  //     {"ICON_MD_VIEW_ARRAY", ICON_MD_VIEW_ARRAY},
  //     {"ICON_MD_VIEW_CAROUSEL", ICON_MD_VIEW_CAROUSEL},
  //     {"ICON_MD_VIEW_CAROUSEL_custom", "\U000f056c"},

  // };

  // static constexpr IconEntry mdi_icons[] = {
  //     // {"ICON_MDI_PUBLIC", ICON_MDI_PUBLIC},
  //     // {"ICON_MDI_VIEW_IN_AR", ICON_MDI_VIEW_IN_AR},
  //     // {"ICON_MDI_GRID_ON", ICON_MDI_GRID_ON},
  //     // {"ICON_MDI_CAMERA_ALT", ICON_MDI_CAMERA_ALT},
  //     // {"ICON_MDI_LIGHT_MODE", ICON_MDI_LIGHT_MODE},
  //     {"ICON_MDI_PALETTE", ICON_MDI_PALETTE},
  //     {"ICON_MDI_TEXTURE", ICON_MDI_TEXTURE},
  //     // {"ICON_MDI_OPEN_WITH", ICON_MDI_OPEN_WITH},
  //     // {"ICON_MDI_360", ICON_MDI_360},
  //     // {"ICON_MDI_STRAIGHTEN", ICON_MDI_STRAIGHTEN},
  //     {"ICON_MDI_TIMELINE", ICON_MDI_TIMELINE},
  //     {"ICON_MDI_ANIMATION", ICON_MDI_ANIMATION},
  //     {"ICON_MDI_TUNE", ICON_MDI_TUNE},
  //     // {"ICON_MDI_VISIBILITY", ICON_MDI_VISIBILITY},
  //     // {"ICON_MDI_VISIBILITY_OFF", ICON_MDI_VISIBILITY_OFF},
  //     {"ICON_MDI_FOLDER", ICON_MDI_FOLDER},
  //     {"ICON_MDI_FOLDER_OPEN", ICON_MDI_FOLDER_OPEN},
  //     // {"ICON_MDI_ADD", ICON_MDI_ADD},
  //     // {"ICON_MDI_REMOVE", ICON_MDI_REMOVE},
  //     {"ICON_MDI_CLOSE", ICON_MDI_CLOSE},
  //     // {"ICON_MDI_SETTINGS", ICON_MDI_SETTINGS},
  //     // {"ICON_MDI_BUILD", ICON_MDI_BUILD},
  //     // {"ICON_MDI_PLAY_ARROW", ICON_MDI_PLAY_ARROW},
  //     {"ICON_MDI_PAUSE", ICON_MDI_PAUSE},
  //     {"ICON_MDI_STOP", ICON_MDI_STOP},
  //     {"ICON_MDI_DELETE", ICON_MDI_DELETE},
  //     {"ICON_MDI_CONTENT_COPY", ICON_MDI_CONTENT_COPY},
  //     // {"ICON_MDI_SAVE", ICON_MDI_SAVE},
  //     {"ICON_MDI_UNDO", ICON_MDI_UNDO},
  //     {"ICON_MDI_REDO", ICON_MDI_REDO},
  // };

  // float icon_size = ImGui::GetTextLineHeight();
  // float cell_width = icon_size + ImGui::CalcTextSize("ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT").x + 24.0f;
  // int cols = ImMax(1, (int)(ImGui::GetContentRegionAvail().x / cell_width));

  // auto draw_section = [&](const char* label, const IconEntry* icons, int count) {
  //   if (!ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) return;

  //   int col = 0;
  //   for (int i = 0; i < count; ++i) {
  //     // filter
  //     // if (filter[0] != '\0' && !ImGui::IsTextFilterMatchingEntry(filter, icons[i].name)) continue;

  //     if (col > 0 && col % cols != 0) ImGui::SameLine(col * cell_width);

  //     // highlight on hover
  //     ImVec2 pos = ImGui::GetCursorScreenPos();
  //     ImGui::PushID(i);
  //     ImGui::InvisibleButton("##icon", ImVec2(cell_width, icon_size + 4));
  //     if (ImGui::IsItemHovered()) {
  //       ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + cell_width, pos.y + icon_size + 4),
  //                                                 IM_COL32(255, 255, 255, 20), 3.0f);
  //       ImGui::SetTooltip("%s", icons[i].name);
  //     }
  //     if (ImGui::IsItemClicked()) ImGui::SetClipboardText(icons[i].name);

  //     ImGui::SetCursorScreenPos(ImVec2(pos.x + 4, pos.y + 2));
  //     ImGui::Text("%s %s", icons[i].icon, icons[i].name);
  //     ImGui::PopID();
  //     ++col;
  //   }
  //   ImGui::Spacing();
  // };

  // static auto codepoint_to_utf8 = [](uint32_t cp) -> std::string {
  //   char buf[5] = {};
  //   if (cp < 0x80) {
  //     buf[0] = cp;
  //   } else if (cp < 0x800) {
  //     buf[0] = 0xC0 | (cp >> 6);
  //     buf[1] = 0x80 | (cp & 0x3F);
  //   } else if (cp < 0x10000) {
  //     buf[0] = 0xE0 | (cp >> 12);
  //     buf[1] = 0x80 | ((cp >> 6) & 0x3F);
  //     buf[2] = 0x80 | (cp & 0x3F);
  //   } else {
  //     buf[0] = 0xF0 | (cp >> 18);
  //     buf[1] = 0x80 | ((cp >> 12) & 0x3F);
  //     buf[2] = 0x80 | ((cp >> 6) & 0x3F);
  //     buf[3] = 0x80 | (cp & 0x3F);
  //   }
  //   return buf;
  // };

  // // --- Codepoint tester ---
  // struct CustomIcon {
  //   uint32_t codepoint;
  //   std::string utf8;
  // };
  // static std::vector<CustomIcon> custom_icons;
  // static char cp_input[16] = "";
  // static char cp_error[64] = "";

  // ImGui::SeparatorText("Codepoint Tester");
  // ImGui::SetNextItemWidth(120.0f);
  // ImGui::InputText("##cp", cp_input, sizeof(cp_input));
  // ImGui::SameLine();
  // if (ImGui::Button("Add")) {
  //   char* end;
  //   uint32_t cp = (uint32_t)strtoul(cp_input, &end, 16);
  //   if (end == cp_input || *end != '\0') {
  //     snprintf(cp_error, sizeof(cp_error), "Invalid hex: '%s'", cp_input);
  //   } else {
  //     custom_icons.push_back({cp, codepoint_to_utf8(cp)});
  //     cp_input[0] = '\0';
  //     cp_error[0] = '\0';
  //   }
  // }
  // ImGui::SameLine();
  // if (ImGui::Button("Clear All")) custom_icons.clear();

  // if (cp_error[0] != '\0') {
  //   ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", cp_error);
  // }

  // if (!custom_icons.empty()) {
  //   ImGui::Spacing();
  //   for (int i = (int)custom_icons.size() - 1; i >= 0; --i) {
  //     auto& ic = custom_icons[i];
  //     ImGui::Text("%s  U+%05X", ic.utf8.c_str(), ic.codepoint);
  //     ImGui::SameLine();
  //     ImGui::PushID(i);
  //     if (ImGui::SmallButton("copy##utf8")) ImGui::SetClipboardText(ic.utf8.c_str());
  //     ImGui::SameLine();
  //     if (ImGui::SmallButton("x")) custom_icons.erase(custom_icons.begin() + i);
  //     ImGui::PopID();
  //   }
  // }

  // ImGui::Spacing();

  // draw_section("Font Awesome 6", fa_icons, IM_ARRAYSIZE(fa_icons));
  // draw_section("Material Design", md_icons, IM_ARRAYSIZE(md_icons));
  // draw_section("Material Design Icons", mdi_icons, IM_ARRAYSIZE(mdi_icons));

  // ImGui::End();
}

inline void drawNerdFontRanges(bool* open) {
  if (!ImGui::Begin("Nerd Font Range Browser", open)) {
    ImGui::End();
    return;
  }

  static auto codepoint_to_utf8 = [](uint32_t cp) -> std::string {
    char buf[5] = {};
    if (cp < 0x80) {
      buf[0] = cp;
    } else if (cp < 0x800) {
      buf[0] = 0xC0 | (cp >> 6);
      buf[1] = 0x80 | (cp & 0x3F);
    } else if (cp < 0x10000) {
      buf[0] = 0xE0 | (cp >> 12);
      buf[1] = 0x80 | ((cp >> 6) & 0x3F);
      buf[2] = 0x80 | (cp & 0x3F);
    } else {
      buf[0] = 0xF0 | (cp >> 18);
      buf[1] = 0x80 | ((cp >> 12) & 0x3F);
      buf[2] = 0x80 | ((cp >> 6) & 0x3F);
      buf[3] = 0x80 | (cp & 0x3F);
    }
    return buf;
  };

  struct Range {
    const char* label;
    uint32_t from;
    uint32_t to;
  };
  static constexpr Range ranges[] = {{"IEC Power Symbols", 0x23FB, 0x23FE},
                                     {"Box Drawing", 0x2500, 0x257F},
                                     {"Powerline Extra 1", 0x2630, 0x2630},
                                     {"Octiocons ...", 0x2665, 0x2665},
                                     {"Octicons Alert", 0x26A1, 0x26A1},
                                     {"Heavy Angle Brackets", 0x276C, 0x2771},
                                     {"IEC Power Symbols 2", 0x2B58, 0x2B58},
                                     {"Pomicons", 0xE000, 0xE00A},
                                     {"Powerline", 0xE0A0, 0xE0A2},
                                     {"Powerline Extra 2", 0xE0A3, 0xE0A3},
                                     {"Powerline 2", 0xE0B0, 0xE0B3},
                                     {"Powerline Extra 3", 0xE0B4, 0xE0C8},
                                     {"Powerline Extra 4", 0xE0CA, 0xE0CA},
                                     {"Powerline Extra 5", 0xE0CC, 0xE0D7},
                                     {"Font Awesome Extension", 0xE200, 0xE2A9},
                                     {"Weather Icons", 0xE300, 0xE3E3},
                                     {"Seti-UI + Custom", 0xE5FA, 0xE6B7},
                                     {"Devicons", 0xE700, 0xE8EF},
                                     {"Codicons", 0xEA60, 0xEC1E},
                                     {"Font Awesome", 0xED00, 0xEFCE},
                                     {"Progress", 0xEE00, 0xEE0B},
                                     {"Font Awesome 2", 0xF000, 0xF2FF},
                                     {"Font Logos", 0xF300, 0xF381},
                                     {"Octicons", 0xF400, 0xF533},
                                     {"Material Design", 0xF0001, 0xF1AF0}};

  static int cols = 16;
  static float icon_size = ImGui::GetTextLineHeight() + 8.0f;

  ImGui::SliderInt("Columns", &cols, 4, 32);
  ImGui::SliderFloat("Icon Size", &icon_size, 16.0f, 64.0f, "%.0f", 1.0f);

  ImGui::Spacing();

  ImFont* font = ImGui::GetFont();  // or a specific font from io.Fonts->Fonts[i];

  for (const auto& range : ranges) {
    if (!ImGui::CollapsingHeader(range.label)) continue;

    ImGui::PushID(range.label);
    uint32_t count = range.to - range.from + 1;
    int rows = (int)((count + cols - 1) / cols);

    // use a clipper so huge ranges (like Nerd Fonts Supplemental) don't kill framerate
    ImGuiListClipper clipper;
    clipper.Begin(rows);
    while (clipper.Step()) {
      for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
        for (int col = 0; col < cols; ++col) {
          uint32_t cp = range.from + row * cols + col;
          if (cp > range.to) break;

          if (col > 0) ImGui::SameLine();

          ImGui::PushID((int)cp);
          ImVec2 pos = ImGui::GetCursorScreenPos();
          ImGui::InvisibleButton("##g", ImVec2(icon_size, icon_size));

          bool hovered = ImGui::IsItemHovered();
          bool clicked = ImGui::IsItemClicked();

          if (hovered) {
            ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + icon_size, pos.y + icon_size),
                                                      IM_COL32(255, 255, 255, 30), 3.0f);
            ImGui::SetTooltip("U+%05X\n%s", cp, codepoint_to_utf8(cp).c_str());
          }
          if (clicked) ImGui::SetClipboardText(codepoint_to_utf8(cp).c_str());

          // center the glyph in the cell
          std::string glyph = codepoint_to_utf8(cp);
          ImVec2 glyph_size = ImGui::CalcTextSize(glyph.c_str());
          ImGui::GetWindowDrawList()->AddText(font, icon_size, ImVec2(pos.x, pos.y), IM_COL32(0, 0, 0, 255),
                                              glyph.c_str());

          ImGui::PopID();
        }
      }
    }
    clipper.End();
    ImGui::PopID();
    ImGui::Spacing();
  }

  ImGui::End();
}