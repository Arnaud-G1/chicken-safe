

#include "MenuRenderer.h"

#include <U8g2lib.h>


/*
 * Display functions
 */

extern U8X8_SSD1306_128X64_NONAME_HW_I2C disp;

void MenuRenderer::render(Menu const& menu) const {

    int StartIdx = 0;
    int EndIdx = MENU_LINES-1;
    if (menu.get_num_components() <= MENU_LINES) {
      EndIdx = menu.get_num_components()-1;
    } else {
      // More menu than lines
      if (menu.get_current_component_num() > menu.get_previous_component_num()) {
        // Navigate down  
        EndIdx = max(menu.get_current_component_num(),MENU_LINES-1);
        StartIdx = EndIdx - MENU_LINES + 1;
      } else {
        // Navigate up
        StartIdx = min(menu.get_current_component_num(),menu.get_num_components()-MENU_LINES);
        EndIdx = StartIdx + MENU_LINES - 1;
      }
    }
    
    for (int i = StartIdx, line = MENU_START_LINE; line < MENU_LINES+MENU_START_LINE; i++, line++) {
      if (i <= EndIdx) {
        MenuComponent const* cp_m_comp = menu.get_menu_component(i);
        disp.clearLine(line);
        disp.setCursor(1,line);
        cp_m_comp->render(*this);
        if (cp_m_comp->is_current()) {
          disp.drawGlyph(0,line,'>');
        }
      } else {
        disp.clearLine(line);
      }
    }
}

void MenuRenderer::render_menu_item(MenuItem const& menu_item) const {
    disp.print(menu_item.get_name());
}

void MenuRenderer::render_back_menu_item(BackMenuItem const& menu_item) const {
    disp.print(menu_item.get_name());
}

void MenuRenderer::render_numeric_menu_item(NumericMenuItem const& menu_item) const {
    disp.print(String("") + menu_item.get_name() + (menu_item.has_focus() ? '>' : '=') + menu_item.get_formatted_value() );
}

void MenuRenderer::render_menu(Menu const& menu) const {
    disp.print(menu.get_name()); 
}
