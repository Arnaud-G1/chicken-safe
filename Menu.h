
#ifndef _MENU_H
#define _MENU_H

#include <MenuSystem.h>
#include "MenuRenderer.h"

// Formatters

const String format_int(const float value);

const String format_float(const float value);

const String format_geo_long(const float value);

const String format_geo_lat(const float value);

// Forward declarations

void on_move_selected(MenuComponent* p_menu_component);

void on_back_selected(MenuComponent* p_menu_component);

void on_door_state_selected(MenuComponent* p_menu_component);

void on_set_day_selected(NumericMenuItem* p_menu_component);
void on_set_month_selected(NumericMenuItem* p_menu_component);
void on_set_year_selected(NumericMenuItem* p_menu_component);
void on_set_hour_selected(NumericMenuItem* p_menu_component);
void on_set_minute_selected(NumericMenuItem* p_menu_component);

void on_set_open_delay_selected(NumericMenuItem* p_menu_component);
void on_set_close_delay_selected(NumericMenuItem* p_menu_component);

void on_set_geo_lat_selected(NumericMenuItem* p_menu_component);
void on_set_geo_long_selected(NumericMenuItem* p_menu_component);
void on_set_geo_tz_selected(NumericMenuItem* p_menu_component);

void menu_setup();

extern MenuSystem ms;

// Declare Numeric menu items as extern to allow setting the values from system (RTC or Eeprom)
// when menu loads

extern NumericMenuItem mu1_dt_d;
extern NumericMenuItem mu1_dt_m;
extern NumericMenuItem mu1_dt_y;
extern NumericMenuItem mu1_dt_h;
extern NumericMenuItem mu1_dt_mn;

extern NumericMenuItem mu2_open_del;
extern NumericMenuItem mu2_close_del;

extern NumericMenuItem mu3_geo_long;
extern NumericMenuItem mu3_geo_lat;
extern NumericMenuItem mu3_geo_tz;

// Declare menu items as extern to allow setting name when menu loads

extern MenuItem mm_move_door; // Name is set dynamically
extern MenuItem mu2_set_door_state;

#endif
