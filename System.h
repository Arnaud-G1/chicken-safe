/* ========================================================================
 * Global constants
\* ======================================================================== */

#define DEBUG     false

// IOs

#define BTN_ENTER 8
#define BTN_UP    9
#define BTN_DOWN 10

#define MOTOR_A   5
#define MOTOR_B   6
#define MOTOR_EN  7

#define RTC_INT   2

#define VBAT      A2

#define LED       LED_BUILTIN

// Vbat divier with 100+470 ohm
#define VBAT_DIVIDER 5.7 

// Other params

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
#define UI_STATE_DEEPSLEEP  8

#define NIGHT 1
#define DAY   0

#define EE_ADDR_CFG       0
#define EE_ADDR_GATE      1
#define EE_ADDR_OPEN_DEL  2
#define EE_ADDR_CLOSE_DEL 4

#define EE_ADDR_LONG     10 // Float : 4 bytes
#define EE_ADDR_LAT      14 // Float : 4 bytes
#define EE_ADDR_TZ       18 // Float : 4 bytes
