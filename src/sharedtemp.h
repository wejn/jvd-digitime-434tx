#include <inttypes.h>
#include <stdbool.h>
#include <time.h>

#ifndef __SHAREDTEMP_H__
#define __SHAREDTEMP_H__

extern volatile float current_temp;
extern volatile time_t last_ts; 
extern volatile bool temp_set;

#endif
