// Constants and timing definitions for KEY_STATUS_MACHINE

#ifndef KEY_H
#define KEY_H

// Define the button count for the system
#define BUTTON_COUNT 4  // Number of buttons in the system

// Define time constants (in milliseconds) for button debounce and response
#define DEBOUNCE_TIME  20   // Time (in ms) to debounce button press (2 scans)
#define LONG_PRESS_TIME 300  // Time (in ms) to define a long press (30 scans)
#define DOUBLE_PRESS_TIME 200 // Time (in ms) to define a double press (20 scans)

// Detailed documentation on time thresholds:
// DEBOUNCE_TIME: This is the time required to stabilize the button reading after a press. 
//               It is defined as thresholds for 2 scans at 10ms period, hence 20ms total.
// LONG_PRESS_TIME: This value determines the duration for which the button must be held down
//                  for it to be recognized as a long press. It is measured at 30 scans, translating to 300ms.
// DOUBLE_PRESS_TIME: This threshold represents the maximum interval allowed between two
//                    button presses for them to be considered a double press. This is set to 200ms,
//                    indicating a time span of 20 scans.

#endif // KEY_H
