#ifndef MICROPY_INCLUDE_MT76X8_MODMACHINE_H
#define MICROPY_INCLUDE_MT76X8_MODMACHINE_H

#include "py/obj.h"

extern const mp_obj_type_t machine_pin_type;

void machine_pins_init(void);
void machine_pins_deinit(void);

#endif
