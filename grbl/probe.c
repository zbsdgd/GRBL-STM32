/*
  probe.c - code pertaining to probing methods
  Part of Grbl

  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "grbl.h"
#include "gpio.h"

// Inverts the probe pin state depending on user settings and probing cycle mode.
uint16_t probe_invert_mask;


// Probe pin initialization routine.
void probe_init()
{
  probe_configure_invert_mask(false); // Initialize invert mask.
}


// Called by probe_init() and the mc_probe() routines. Sets up the probe pin invert mask to
// appropriately set the pin logic according to setting for normal-high/normal-low operation
// and the probing cycle modes for toward-workpiece/away-from-workpiece.
void probe_configure_invert_mask(uint8_t is_probe_away)
{
  probe_invert_mask = 0; // Initialize as zero.
  if (bit_isfalse(settings.flags,BITFLAG_INVERT_PROBE_PIN)) { probe_invert_mask ^= PROBE_MASK; }
  if (is_probe_away) { probe_invert_mask ^= PROBE_MASK; }
}


// Returns the probe pin state. Triggered = true. Called by gcode parser and probe state monitor.
uint8_t probe_get_state() {
	uint8_t probe_state = 0;
	uint16_t pin = (GPIO_ReadPort(PROBE_GPIO_Port) & PROBE_MASK) ^ probe_invert_mask;
	if (pin & get_probe_pin_mask()) {probe_state |= 1;}
	return probe_state;
}


// Monitors probe pin state and records the system position when detected. Called by the
// stepper ISR per ISR tick.
// NOTE: This function must be extremely efficient as to not bog down the stepper ISR.
void probe_state_monitor()
{
  if (probe_get_state()) {
    sys_probe_state = PROBE_OFF;
    memcpy(sys_probe_position, sys_position, sizeof(sys_position));
    bit_true(sys_rt_exec_state, EXEC_MOTION_CANCEL);
  }
}
