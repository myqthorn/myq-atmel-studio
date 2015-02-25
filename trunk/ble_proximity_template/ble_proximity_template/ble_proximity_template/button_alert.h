
#include <stdint.h>
#include "alert_level_characteristic.h"


#ifndef BUTTON_ALERT_H__
#define BUTTON_ALERT_H__

extern void button_alert_hook();

void button_alert_pipes_updated_evt_rcvd(uint8_t pipe_num, uint8_t *buffer);

#endif//BUTTON_ALERT_H__


