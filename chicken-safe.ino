
//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h> 

// Low power
// https://github.com/rocketscream/Low-Power/
// https://thewanderingengineer.com/2014/08/11/arduino-pin-change-interrupts/
#include <LowPower.h>

// OLED
// https://github.com/olikraus/u8g2
#include <U8g2lib.h>

// RTC
// https://github.com/NeiroNx/RTCLib
#include <RTClib.h>

// Use this lib as it has easy way to handle date time component set

// close_time open_time
#include <Dusk2Dawn.h>

// Menu
// https://github.com/jonblack/arduino-menusystem
#include "Menu.h"

// EEprom to store settings
#include <EEPROM.h>


#include "System.h"

typedef struct {
  byte disp_state;
  unsigned long  state_time;
  unsigned long  last_update;
  unsigned long  idle_time; // for sleep entry
  // Buttons
  struct  {
    btn_t up;
    btn_t down;
    btn_t enter;
  } buttons;
  // Time info
  DateTime currentTime;
  int close_time;    // Minutes since midnight
  int open_time;     // Minutes since midnight
  byte day_state;    // Keep state (day or night)
  byte last_duskdawn_day;  // Day when duskdawn was computed (once per day is enough)
  // Motor
  byte motor_state;
  unsigned long motor_start_time;
  // Door
  byte gate;
  // Battery
  float vbat;
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


 
void menu_ack() {
    disp.clear();
    disp.setCursor(0,1);
    disp.print(F("Saved! "));
    delay(1000); // so we can look the result on the LCD
    // Leave menu
    ms.reset();
    State.disp_state = UI_STATE_DEFAULT;
}

// Menu callback function

void on_back_selected(MenuComponent* p_menu_component) {
}

void on_move_selected(MenuComponent* p_menu_component) {
    ms.reset();
    disp.clear();
    if (State.gate == GATE_CLOSE) {
      disp.draw1x2String(0, 2, "Opening door");    
    } else {
      disp.draw1x2String(0, 2, "Closing door");  
    }
    gate_move(State.gate ^ 0x1); 
    State.disp_state = UI_STATE_ACTION;
}


// Date & time menu entries

void on_set_day_selected(NumericMenuItem* p_menu_component) {
    int val = p_menu_component->get_value();
    
    DateTime now = rtc.now();
    now.setday(val);
    rtc.adjust(now);    
    menu_ack();
}

void on_set_month_selected(NumericMenuItem* p_menu_component) {
    int val = p_menu_component->get_value();
    
    DateTime now = rtc.now();
    now.setmonth(val);
    rtc.adjust(now);    
    menu_ack();
}

void on_set_year_selected(NumericMenuItem* p_menu_component) {
    int val = p_menu_component->get_value();

    DateTime now = rtc.now();
    now.setyear(val);
    rtc.adjust(now);
    menu_ack();
}

void on_set_hour_selected(NumericMenuItem* p_menu_component) {
    int val = p_menu_component->get_value();

    DateTime now = rtc.now();
    now.sethour(val);
    rtc.adjust(now);    
    menu_ack();
}

void on_set_minute_selected(NumericMenuItem* p_menu_component) {
    int val = p_menu_component->get_value();

    DateTime now = rtc.now();
    now.setminute(val);
    rtc.adjust(now);    
    menu_ack();
}

void on_set_open_delay_selected(NumericMenuItem* p_menu_component) {
   update_sun_times();
   menu_ack();
}
void on_set_close_delay_selected(NumericMenuItem* p_menu_component) {
  update_sun_times();
  menu_ack();
}

void on_door_state_selected(MenuComponent* p_menu_component) {
    State.gate ^=  1;
    EEPROM.put(EE_ADDR_GATE, State.gate);
}

/* Geo menu */

void on_set_geo_long_selected(NumericMenuItem* p_menu_component) {
   update_sun_times();
   menu_ack();
}
void on_set_geo_lat_selected(NumericMenuItem* p_menu_component) {
  update_sun_times();
  menu_ack();
}

void on_set_geo_tz_selected(NumericMenuItem* p_menu_component) {
  update_sun_times();
  menu_ack();
}



/* ========================================================================
 * Low power
\* ======================================================================== */

bool rtc_tick;

// RTC event


// Button Pin change interrupt

ISR(PCINT0_vect){
    PCICR &= ~(0b00000001); 
        // Crear interrupt enable to avoid interrupt from buttons when awake

    State.idle_time = millis(); // Reset idle counter, wake for another period
}    

// RTC Event

ISR(PCINT2_vect){
    rtc_tick = true;
} 


void low_power_setup() {

  pinMode(RTC_INT, INPUT_PULLUP);
  // Setup SQW
  rtc.write(0x07,0x10); // Enable SQW at 1 Hz
  
  //https://www.teachmemicro.com/arduino-interrupt-tutorial/#Pin_Change_Interrupt
  
  PCMSK0 |= 0b0000111;    // Enable PCINT0 for pins 8,9,10
  PCMSK2 |= 0b0000100;    // Enable PCINT2 for pins 2

  PCICR |= 0b00000100;    // turn on port d (PCIE2 =1)
  
  State.idle_time = millis();  

  rtc_tick = false;
}


void low_power_sleep_req() {
  
  digitalWrite(LED,LOW);

  // Enable PCINT0 for button change detection
  PCICR |= 0b00000001;    // turn on port b & d (PCIE0, PCIE2 =1)

  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
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

  EEPROM.get(EE_ADDR_GATE, State.gate);
  if (State.gate == 0xFF) {
        State.gate = GATE_CLOSE;
        EEPROM.put(EE_ADDR_GATE, State.gate);
  }
}

void setup()
{

  if (DEBUG) Serial.begin(9600);
  
  Wire.begin();

  // ADC
  analogReference(INTERNAL);

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

  // LED
  pinMode(LED, OUTPUT);
  
  // Buttons
  btn_setup(&State.buttons.up, BTN_UP);
  btn_setup(&State.buttons.down, BTN_DOWN);
  btn_setup(&State.buttons.enter, BTN_ENTER);

  // Config setup
  config_setup();

  // Motor
  motor_setup();
  
  update_state(1);

  // Menu
  menu_setup();

  // Low power
  low_power_setup();

  if (DEBUG) Serial.println(F("Setup done!")); 
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

  if (State.gate == GATE_OPEN) {
    mm_move_door.set_name("1.Close door");
    mu2_set_door_state.set_name("Set as closed");
  }
  else {
    mm_move_door.set_name("1.Open door");
    mu2_set_door_state.set_name("Set as open");
  }
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
          // 
          State.idle_time = millis();
      }
    }
    return ret_val;
}

void ui_handler() {
    if (State.disp_state == UI_STATE_DEEPSLEEP) {
      disp.setPowerSave(0);
      disp.clear();
      // Clear buttons state to detect new press after wakeup
      btn_read(&State.buttons.enter);
      btn_read(&State.buttons.up);
      btn_read(&State.buttons.down);
      
      State.disp_state = UI_STATE_DEFAULT;
    }

    if ((millis() - State.state_time) > 10000) {
        // Leave menu state
        if (State.disp_state == UI_STATE_MENU) {
          ms.reset();
          disp.clear();
          State.disp_state = UI_STATE_DEFAULT;
        }

    }

    // Regular system update event
    //if ((millis() - State.last_update) > 1000) {
    if (rtc_tick) {
      State.last_update = millis();
      rtc_tick = false;

      update_state(0); // No dusk dawn update
      if (State.disp_state == UI_STATE_DEFAULT) {
        // Display info
        display_info();
      }
    }

    if (State.disp_state == UI_STATE_MENU) {
      display_time(false); // Only display time on 1st line
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


void display_time(bool large) {
  //disp.clearLine(0);
  char buf[] = "DD/MM/YY  hh:mm";
  State.currentTime.format(buf);
  if (large)
    disp.draw1x2String(0, 0, buf);  
  else
    disp.drawString(0, 0, buf);  
}

// Display full info

void display_info(void) {
  // graphic commands to redraw the complete screen should be placed here  
  display_time(true);

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
  disp.setCursor(0, 7);
  disp.print(String("VBat: ")+State.vbat); 
}

/* ========================================================================
 * Action Handler
\* ======================================================================== */

/*
 * Action handler
 * Check when actions needs to be taken
 */


void gate_move(bool dir) {
  Serial.print(dir);
  State.motor_start_time = millis();
  State.idle_time = millis(); // Prevent deep sleep entry
  // IO control
  digitalWrite(MOTOR_A,!dir);
  digitalWrite(MOTOR_B,dir);
  digitalWrite(MOTOR_EN,1);
  State.motor_state = dir ? MOTOR_OPEN : MOTOR_CLOSE;
}


void gate_stop() {

  State.disp_state = UI_STATE_DEFAULT;
  State.motor_state = MOTOR_OFF;
  // IO control
  digitalWrite(MOTOR_A,0);
  digitalWrite(MOTOR_B,0);
  digitalWrite(MOTOR_EN,0); // Saves power
}

bool is_night() {
  // Define if the time is day or night
  // Take delay into account

  int time_min = State.currentTime.hour() * 60 + State.currentTime.minute(); // Time expressed in minutes

  if (time_min < State.open_time) 
    return 1; // Night, before open_time
  if (time_min < State.close_time) 
    return 0; // Day
  return 1; // Night, after close_time
}

void action_handler() {
    // Test gate versus request

    // Gate open
    if ((State.motor_state != MOTOR_OFF) && ((millis() -  State.motor_start_time) > MOTOR_TIMEOUT*1000)) {
       // Gate action completed
       if (State.motor_state == MOTOR_OPEN) {
          State.gate = GATE_OPEN;
          EEPROM.put(EE_ADDR_GATE, State.gate);
       }
       if (State.motor_state == MOTOR_CLOSE) {
          State.gate = GATE_CLOSE;
          EEPROM.put(EE_ADDR_GATE, State.gate);
       }
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
    
    float geo_long = mu3_geo_long.get_value();
    float geo_lat = mu3_geo_lat.get_value();
    float geo_tz = mu3_geo_tz.get_value();

    uint16_t open_delay = mu2_open_del.get_value();
    uint16_t close_delay = mu2_close_del.get_value();

    Dusk2Dawn localTimeData(geo_lat,geo_long, geo_tz);

    State.open_time     = (localTimeData.sunrise(State.currentTime.year(), State.currentTime.month(), State.currentTime.day(), false) +  open_delay) % DAY_IN_MINUTES;
    State.close_time    = (localTimeData.sunset(State.currentTime.year(), State.currentTime.month(), State.currentTime.day(), false)  + close_delay) % DAY_IN_MINUTES;

    State.last_duskdawn_day = State.currentTime.day();
}

void update_state(int init_state) {
    

    if (init_state) {
        State.disp_state = UI_STATE_DEFAULT;
        State.state_time = millis();
        State.last_update = 0;

        State.day_state = is_night();  
        State.idle_time = millis();
    }

    // Copy to temp var to avoid leakage if assigned directly...
    DateTime dt = rtc.now();
    State.currentTime = DateTime(dt.year(), dt.month(), dt.day(),dt.hour(),dt.minute(), dt.second());
    
    // Update every day or when init is set
    if (init_state || (dt.day() != State.last_duskdawn_day)) {
        update_sun_times();
    }

    // Check battery level
    int val = analogRead(VBAT);  // read the input pin

    // With a divider 100K+1M the ration Vadc/Vin = 1/11
    float vadc = (float)val/1024*1.1;
    State.vbat = vadc * VBAT_DIVIDER; 

    if (DEBUG) Serial.println(String("")+is_night()+" " + State.day_state + " " + State.gate );

}




/* ========================================================================
 * Main loop
\* ======================================================================== */

void loop()
{

  ui_handler();
  
  action_handler();  
  
  // Low power handler
  if ((millis() - State.idle_time) > 60000 ) {
    // 1 minute
    disp.setPowerSave(1);
    State.disp_state = UI_STATE_DEEPSLEEP;
    low_power_sleep_req();
  }
  // Monitors when system active (debug)
  digitalWrite(LED,HIGH);
}
