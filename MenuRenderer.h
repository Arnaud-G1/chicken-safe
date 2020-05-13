
#ifndef _MENU_RENDERER_H
#define _MENU_RENDERER_H

#include <MenuSystem.h>

#define MENU_LINES 5

#define MENU_START_LINE 2

class MenuRenderer : public MenuComponentRenderer {
public:
    void render(Menu const& menu) const;
    void render_menu_item(MenuItem const& menu_item) const;
    void render_back_menu_item(BackMenuItem const& menu_item) const;
    void render_numeric_menu_item(NumericMenuItem const& menu_item) const;
    //void render_custom_numeric_menu_item(CustomNumericMenuItem const& menu_item) const;
    void render_menu(Menu const& menu) const;
};

#endif
