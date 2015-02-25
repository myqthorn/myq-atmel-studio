#include <stdint.h>
#include "lib_aci.h"
#include "services.h"
#include "alert_level_characteristic.h"
#include "button_alert.h"

void button_alert_pipes_updated_evt_rcvd(uint8_t pipe_num, uint8_t *buffer)
{
	switch (pipe_num)
	{
	case PIPE_BUTTONCLICK_CLICK_RX_ACK_AUTO:
		button_alert_hook();
		break;
	}
}
