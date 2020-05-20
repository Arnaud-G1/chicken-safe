#include "Menu.h"
#include "System.h"

/* ========================================================================
 * Menu formatters
\* ======================================================================== */

// writes the (int) value of a float into a char buffer.
const String format_int(const float value) {
    return String((int) value);
}

const String format_float(const float value) {
    return String(value);
}

const String format_geo_long(const float value) {
    if (value > 0)    
        return String((float) value)+String(" E");
    else
        return String((float) value)+String(" W");
}

const String format_geo_lat(const float value) {
    if (value > 0)    
        return String((float) value)+String(" N");
    else
        return String((float) value)+String(" S");
}

/* ========================================================================
 * Menu Variables
\* ======================================================================== */


// Menu variables

MenuRenderer menu_renderer;

MenuSystem ms(menu_renderer);
MenuItem mm_mi1("1.Open door", &on_open_selected);
MenuItem mm_mi2("2.Close door", &on_close_selected);

Menu mu1("3.Date & Time");

NumericMenuItem mu1_dt_d("Day   ",    &on_set_day_selected,     0,    1,   31, 1, format_int);
NumericMenuItem mu1_dt_m("Month ",    &on_set_month_selected,   1,    1,   12, 1, format_int);
NumericMenuItem mu1_dt_y("Year  ",    &on_set_year_selected, 2020, 1979, 2050, 1, format_int);
NumericMenuItem mu1_dt_h("Hour  ",    &on_set_hour_selected,    0,    1,   23, 1, format_int);
NumericMenuItem mu1_dt_mn("Minute",   &on_set_minute_selected,  0,    1,   59, 1, format_int);

Menu mu2("4.Door params"); // Gate
NumericMenuItem mu2_open_del( "Open delay ",     &on_set_open_delay_selected,    60,    -120,   120, 10, format_int);
NumericMenuItem mu2_close_del("Close delay",   &on_set_close_delay_selected,  -30,    -120,   120, 10, format_int);

Menu mu3("5.Position"); // Position
NumericMenuItem mu3_geo_long("Long.  ",    &on_set_geo_long_selected,    GEO_LONG,    -180,   180, 0.5, format_geo_long);
NumericMenuItem mu3_geo_lat( "Lat.   ",   &on_set_geo_lat_selected,       GEO_LAT,     -180,   180, 0.5, format_geo_lat);
NumericMenuItem mu3_geo_tz(  "Time z.",   &on_set_geo_tz_selected,         GEO_TZ,      -12,   +12,  0.5, format_float);

BackMenuItem mu_back("<= Back",&on_back_selected, &ms);

// Setup the menu structure

void menu_setup() {
  ms.get_root_menu().add_item(&mm_mi1);
  ms.get_root_menu().add_item(&mm_mi2);
  ms.get_root_menu().add_menu(&mu1);
  ms.get_root_menu().add_menu(&mu2);
  ms.get_root_menu().add_menu(&mu3);
  mu1.add_item(&mu1_dt_h);
  mu1.add_item(&mu1_dt_mn);
  mu1.add_item(&mu1_dt_d);
  mu1.add_item(&mu1_dt_m);
  mu1.add_item(&mu1_dt_y);
  mu1.add_item(&mu_back);
  mu2.add_item(&mu2_open_del);
  mu2.add_item(&mu2_close_del);
  mu2.add_item(&mu_back);
  mu3.add_item(&mu3_geo_long);
  mu3.add_item(&mu3_geo_lat);
  mu3.add_item(&mu3_geo_tz);
  mu3.add_item(&mu_back);
}
