#include "sharedtemp.h"

volatile float current_temp = -50.0;
volatile time_t last_ts;
volatile bool temp_set = false;
