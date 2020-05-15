
//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h> 

// OLED
// https://github.com/olikraus/u8g2
//#include "U8glib.h"
#include <U8g2lib.h>

// RTC
// https://github.com/NeiroNx/RTCLib
#include <RTClib.h>

// Use this lib as it has easy way to handle date time component set

// close_time open_time
#include <Dusk2Dawn.h>

// Menu
// https://github.com/jonblack/arduino-menusystem
#include <MenuSystem.h>
#include "MenuRenderer.h"

// EEprom to store settings
#include <EEPROM.h>

// Store strings in progmem
#include <avr/pgmspace.h>

// IOs

#define BTN_ENTER 8
#define BTN_UP    9
#define BTN_DOWN 10

#define MOTOR_A   5
#define MOTOR_B   6
#define MOTOR_EN  7

#define DEBOUNCE_DELAY 50

#define RTC_IC DS1307

#define GEO_LAT 50.59833
#define GEO_LONG 4.32848
#define GEO_TZ +2

#define MOTOR_TIMEOUT    20 /* seconds */

// State definitions

#define MOTOR_OFF   0
#define MOTOR_OPEN  1
#define MOTOR_CLOSE 2

#define GATE_OPEN  1
#define GATE_CLOSE 2

#define UI_STATE_DEFAULT 0
#define UI_STATE_MENU    1
#define UI_STATE_ACTION  2

#define NIGHT 1
#define DAY   0

#define EE_ADDR_CFG       0
#define EE_ADDR_OPEN_DEL  2
#define EE_ADDR_CLOSE_DEL 4
#define EE_ADDR_GATE      6
#define EE_ADDR_LONG     10 // Float : 4 bytes
#define EE_ADDR_LAT      14 // Float : 4 bytes
#define EE_ADDR_TZ       18 // Float : 4 bytes
/*
 * Type definitions
 */

typedef struct {
  unsigned long debounce_time;
  byte id;
  byte state;  
} btn_t;



typedef struct {
  byte disp_state;
  unsigned long  state_time;
  unsigned long  last_update;
  unsigned long  last_duskdawn_update;
  // Buttons
  struct  {
    btn_t up;
    btn_t down;
    btn_t enter;
  } buttons;
  // Time info
  DateTime currentTime;
  int close_time;   // Minutes since midnight
  int open_time;  // Minutes since midnight
  byte day_state; // Keep state (day or night)
  // Motor
  byte motor_state;
  unsigned long motor_start_time;
  // Door
  byte gate;
  int cfg_gate_open_delay;
  int cfg_gate_close_delay;
} state_t;

/*
 *  Globals 
 */

RTC_IC rtc;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins) at address 0x3C
U8X8_SSD1306_128X64_NONAME_HW_I2C disp(/* reset=*/ U8X8_PIN_NONE);

//Dusk2Dawn localTimeData(GEO_LAT,GEO_LONG, GEO_TZ);

state_t State;


/* ========================================================================
 * Menu handler
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

// Forward declarations

void on_open_selected(MenuComponent* p_menu_component);
void on_close_selected(MenuComponent* p_menu_component);

void on_back_selected(MenuComponent* p_menu_component) {
}

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

//
void menu_ack(int val) {
    disp.clear();
    disp.setCursor(0,1);
    disp.print(F("Done! "));
    disp.print(val);
    delay(1000); // so we can look the result on the LCD
    // Leave menu
    State.disp_state = UI_STATE_DEFAULT;
}

void menu_ack_0() {
    disp.clear();
    disp.setCursor(0,1);
    disp.print(F("Done! "));
    delay(1000); // so we can look the result on the LCD
    // Leave menu
    State.disp_state = UI_STATE_DEFAULT;
}

// Menu callback function

void on_open_selected(MenuComponent* p_menu_component) {
    ms.reset();
    disp.clear();
    disp.draw1x2String(0, 2, "Opening door");  
    State.disp_state = UI_STATE_ACTION;
    gate_move(GATE_OPEN);   
}

void on_close_selected(MenuComponent* p_menu_component) {
    ms.reset();
    disp.clear();
    disp.draw1x2String(0, 2, "Closing door");  
    State.disp_state = UI_STATE_ACTION;
    gate_move(GATE_CLOSE);
}


// Date & time menu entries

void on_set_day_selected(NumericMenuItem* p_menu_component) {
    int val = p_menu_component->get_value();
    
    DateTime now = rtc.now();
    now.setday(val);
    rtc.adjust(now);
    
    menu_ack(rtc.now().day());
}

void on_set_month_selected(NumericMenuItem* p_menu_component) {
    int val = p_menu_component->get_value();
    
    DateTime now = rtc.now();
    now.setmonth(val);
    rtc.adjust(now);
    
    menu_ack(rtc.now().month());
}

void on_set_year_selected(NumericMenuItem* p_menu_component) {
    int val = p_menu_component->get_value();

    DateTime now = rtc.now();
    now.setyear(val);
    rtc.adjust(now);
    
    menu_ack(rtc.now().year());
}

void on_set_hour_selected(NumericMenuItem* p_menu_component) {
    int val = p_menu_component->get_value();

    DateTime now = rtc.now();
    now.sethour(val);
    rtc.adjust(now);
    
    menu_ack(rtc.now().hour());
}

void on_set_minute_selected(NumericMenuItem* p_menu_component) {
    int val = p_menu_component->get_value();

    DateTime now = rtc.now();
    now.setminute(val);
    rtc.adjust(now);
    
    menu_ack(rtc.now().minute());
}

void on_set_open_delay_selected(NumericMenuItem* p_menu_component) {
   int val = p_menu_component->get_value();
   EEPROM.put(EE_ADDR_OPEN_DEL, val);
   State.cfg_gate_open_delay = val;
   update_sun_times();
   menu_ack(val);
}
void on_set_close_delay_selected(NumericMenuItem* p_menu_component) {
  int val = p_menu_component->get_value();
  EEPROM.put(EE_ADDR_CLOSE_DEL, val);
  State.cfg_gate_close_delay = val;
  update_sun_times();
  menu_ack(val);
}

void on_set_geo_long_selected(NumericMenuItem* p_menu_component) {
   float val = p_menu_component->get_value();
   EEPROM.put(EE_ADDR_LONG, val);
   update_sun_times();
   menu_ack_0();
}
void on_set_geo_lat_selected(NumericMenuItem* p_menu_component) {
  float val = p_menu_component->get_value();
  EEPROM.put(EE_ADDR_LAT, val);
  update_sun_times();
   menu_ack_0();
}

void on_set_geo_tz_selected(NumericMenuItem* p_menu_component) {
  float val = p_menu_component->get_value();
  EEPROM.put(EE_ADDR_TZ, val);
  update_sun_times();
  menu_ack_0();
}

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


/* ========================================================================
 * Setup
\* ======================================================================== */

void btn_setup(btn_t * btn, int id) {

  pinMode(id, INPUT_PULLUP);
  btn->id = id;
  btn->state = 0;
  btn->debounce_time = millis();  
}

void motor_setup() {

  pinMode(MOTOR_A, OUTPUT);
  pinMode(MOTOR_B, OUTPUT);
  pinMode(MOTOR_EN, OUTPUT);

  gate_stop();
}

void config_setup() {
  // Init eeprom variables
  EEPROM.get(EE_ADDR_OPEN_DEL, State.cfg_gate_open_delay);
  if (State.cfg_gate_open_delay == -1) {
        State.cfg_gate_open_delay = 60;
        EEPROM.put(EE_ADDR_OPEN_DEL, State.cfg_gate_open_delay);
  }
  EEPROM.get(EE_ADDR_CLOSE_DEL, State.cfg_gate_close_delay);
  if (State.cfg_gate_close_delay == -1) {
        State.cfg_gate_close_delay = -30;
        EEPROM.put(EE_ADDR_CLOSE_DEL, State.cfg_gate_close_delay);
  }
  // Initialize GEO eeprom (float values)
  float fval; 
  EEPROM.get(EE_ADDR_LONG, fval);
  if (fval != fval) {
       fval = GEO_LONG;
       EEPROM.put(EE_ADDR_LONG, fval);
  }
  EEPROM.get(EE_ADDR_LAT, fval);
  if (fval != fval) {
       fval = GEO_LAT;
       EEPROM.put(EE_ADDR_LAT, fval);
  }
  EEPROM.get(EE_ADDR_TZ, fval);
  if (fval != fval) {
       fval = GEO_TZ;
       EEPROM.put(EE_ADDR_TZ, fval);
  }
}

void setup()
{

  Serial.begin(9600);
  
  Wire.begin();

  // RTC
  rtc.begin();

  if (!rtc.isrunning()) {
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  //rtc.adjust(DateTime(__DATE__, __TIME__));

  // Display
  disp.begin();
  disp.setPowerSave(0);
  disp.setFont(u8x8_font_chroma48medium8_r);
  disp.clear();
  //disp.setFont(u8x8_font_8x13B_1x2_f);

  // Buttons
  btn_setup(&State.buttons.up, BTN_UP);
  btn_setup(&State.buttons.down, BTN_DOWN);
  btn_setup(&State.buttons.enter, BTN_ENTER);

  // Config setup
  config_setup();

  // Motor
  motor_setup();
  
  updateState(1);

  // Menu
  menu_setup();

  Serial.println(F("Setup done!")); 
}

/* ========================================================================
 * State update
\* ======================================================================== */

/* Update internal menu values */

void menu_update() {

  // Update
  mu1_dt_d.set_value(State.currentTime.day());
  mu1_dt_m.set_value(State.currentTime.month());
  mu1_dt_y.set_value(State.currentTime.year());
  mu1_dt_h.set_value(State.currentTime.hour());
  mu1_dt_mn.set_value(State.currentTime.minute());   
  mu2_open_del.set_value(State.cfg_gate_open_delay); 
  mu2_close_del.set_value(State.cfg_gate_close_delay); 

  // Geo values are read directly from EEPROM
  float val;
  EEPROM.get(EE_ADDR_LONG, val);
  mu3_geo_long.set_value(val); 
  EEPROM.get(EE_ADDR_LAT, val);
  mu3_geo_lat.set_value(val); 
  EEPROM.get(EE_ADDR_TZ, val);
  mu3_geo_tz.set_value(val); 
}



/* ========================================================================
 * UI Handler
\* ======================================================================== */

// Button read and debounce


int btn_read(btn_t * btn) {
    unsigned int btn_value = !digitalRead(btn->id);
    int ret_val = 0;
    
    if (btn_value == 0) {
      // Btn has changed
      btn->debounce_time = millis();
      btn->state = 0;
    }
    if ((millis() - btn->debounce_time) > DEBOUNCE_DELAY) {
      // Debounced value
      // Update the button state if it has changed
      if (btn_value == 1 && btn->state == 0) {
          btn->state = 1;
          ret_val = 1; 
      }
    }
    return ret_val;
}

void ui_handler() {
    if ((millis() - State.state_time) > 10000) {
        // Leave menu state
        if (State.disp_state == UI_STATE_MENU) {
          ms.reset();
          disp.clear();
          State.disp_state = UI_STATE_DEFAULT;
        }

    }
    if ((millis() - State.last_update) > 1000) {
      State.last_update = millis();

      //updateState(0); // No dusk dawn update
      if (State.disp_state == UI_STATE_DEFAULT) {
        // Display info
        display_info();
      }
    }

    if (State.disp_state == UI_STATE_MENU) {
      display_time(); // Only display time on 1st line
      // Menu state
      if (btn_read(&State.buttons.enter)) {
          State.state_time = millis();
          ms.select();
          if (State.disp_state == UI_STATE_MENU) {
              // Display menu if still in menu state
              ms.display();
          }
      } 
      else if (btn_read(&State.buttons.up)) {
          State.state_time = millis();
          ms.prev();
          ms.display();
      }
      else if (btn_read(&State.buttons.down)) {
          State.state_time = millis();
          ms.next();
          ms.display();
      }  
    } else if (State.disp_state == UI_STATE_DEFAULT) {
      // Menu not selected
      if (btn_read(&State.buttons.enter)) {
          // Enter menu state
          State.state_time = millis();
          menu_update();
          disp.clear();
          ms.display();
          State.disp_state = UI_STATE_MENU;
      } 
    }
}

/* ========================================================================
 * Display functions
\* ======================================================================== */


void display_time(void) {
  // graphic commands to redraw the complete screen should be placed here  
  //u8g.setFont(u8g_font_unifont);
  //disp.setFont(u8x8_font_chroma48medium8_r);

  disp.setCursor(0, 0);
  //disp.clearLine(0);
  char buf[] = "DD/MM/YY  hh:mm";
  State.currentTime.format(buf);
  disp.drawString(0, 0, buf);  
}

// Display full info

void display_info(void) {
  // graphic commands to redraw the complete screen should be placed here  
  char buf[] = "DD/MM/YY  hh:mm";
  State.currentTime.format(buf);
  disp.draw1x2String(0, 0, buf);  

  disp.setCursor(0, 2);
  disp.clearLine(2);
  disp.setCursor(0, 3);
  // Line 3: Open time
  char timeStr[6];
  Dusk2Dawn::min2str(timeStr, State.open_time);
  disp.print(String("Open:  ")+timeStr); // 06:58
  // Line 4: Close time
  disp.setCursor(0, 4);
  Dusk2Dawn::min2str(timeStr, State.close_time);
  disp.print(String("Close: ")+timeStr); // 06:58
  
  // Line 5
  disp.setCursor(0, 5);
  if (State.gate == GATE_OPEN) {
    disp.print(F("Door open   "));
  } else {
    disp.print(F("Door closed "));
  }
  
}

/* ========================================================================
 * Action Handler
\* ======================================================================== */

/*
 * Action handler
 * Check when actions needs to be taken
 */

void gate_move(byte dir) {
  
  State.motor_start_time = millis();
  // IO control
  digitalWrite(MOTOR_A,!dir);
  digitalWrite(MOTOR_B,dir);
  digitalWrite(MOTOR_EN,1);
  State.motor_state = dir;

  Serial.print(F("Gate move"));
  Serial.println(dir);
  
}


void gate_stop() {

  State.disp_state = UI_STATE_DEFAULT;
  State.motor_state = MOTOR_OFF;
  // IO control
  Serial.println(F("Gate stop"));
  digitalWrite(MOTOR_A,0);
  digitalWrite(MOTOR_B,0);
  digitalWrite(MOTOR_EN,0); // Saves power
}

bool is_night() {
  // Define if the time is day or night
  // Take delay into account

  int time_min = State.currentTime.hour() * 60 + State.currentTime.minute(); // Time expressed in minutes

  if (time_min < (State.open_time + State.cfg_gate_close_delay)) 
    return 1; // Night, before open_time
  if (time_min < (State.close_time + State.cfg_gate_open_delay)) 
    return 0; // Day
  return 1; // Night, after close_time
}

void action_handler() {
    // Test gate versus request

    // Gate open
    if ((State.motor_state != MOTOR_OFF) && ((millis() -  State.motor_start_time) > MOTOR_TIMEOUT*1000)) {
       // Gate action completed
       if (State.motor_state == MOTOR_OPEN) 
          State.gate = GATE_OPEN;
       if (State.motor_state == MOTOR_CLOSE) 
          State.gate = GATE_CLOSE;
       gate_stop();
    }

    // Door actions

    if (State.day_state == DAY && is_night()) {
       // Day ends, close the door if needed
       State.day_state = NIGHT;
       if (State.gate == GATE_OPEN) gate_move(GATE_CLOSE);
    }
    
    if (State.day_state == NIGHT && !is_night()) {
       // Day starts, open the door if needed
       State.day_state = DAY;
       if (State.gate == GATE_CLOSE) gate_move(GATE_OPEN);
    }
}

/*
 * Get close_time and open_time time based on geo settings
 */

Dusk2Dawn * localTimeData;

void update_sun_times() {

    const int DAY_IN_MINUTES = 24*60;
    
    float geo_long;
    float geo_lat;
    float geo_tz;
  
    EEPROM.get(EE_ADDR_LONG, geo_long);
    EEPROM.get(EE_ADDR_LAT, geo_lat);
    EEPROM.get(EE_ADDR_TZ, geo_tz);

    Dusk2Dawn localTimeData(geo_lat,geo_long, geo_tz);

    State.open_time     = (localTimeData.sunrise(State.currentTime.year(), State.currentTime.month(), State.currentTime.day(), false) +  State.cfg_gate_open_delay) % DAY_IN_MINUTES;
    State.close_time    = (localTimeData.sunset(State.currentTime.year(), State.currentTime.month(), State.currentTime.day(), false)  + State.cfg_gate_close_delay) % DAY_IN_MINUTES;

    State.last_duskdawn_update = millis();
}

void updateState(int init_state) {
    

    if (init_state) {
        State.disp_state = UI_STATE_DEFAULT;
        State.state_time = millis();
        State.last_update = 0;

        State.day_state = is_night();  
    }

    // Copy to temp var to avoid leakage if assigned directly...
    DateTime dt = rtc.now();
    State.currentTime = DateTime(dt.year(), dt.month(), dt.day(),dt.hour(),dt.minute(), dt.second());
    
    // Update every 12 hours or when init is set
    if (init_state || ((millis() - State.last_duskdawn_update) > 12*60*60*1000)) {
        update_sun_times();
    }
}


/* ========================================================================
 * Main loop
\* ======================================================================== */

void loop()
{
  action_handler();

  ui_handler();
}
