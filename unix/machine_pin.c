#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int mt76x8_gpio_get_pin(int pin);
void mt76x8_gpio_set_pin_direction(int pin, int is_output);
void mt76x8_gpio_set_pin_value(int pin, int value);

#define GPIO_MODE_INPUT         (0)
#define GPIO_MODE_INPUT_OUTPUT  (1)

#define MMAP_PATH   "/dev/mem"

#define RALINK_GPIO_DIR_IN      0
#define RALINK_GPIO_DIR_OUT     1

#define RALINK_REG_PIOINT       0x690
#define RALINK_REG_PIOEDGE      0x6A0
#define RALINK_REG_PIORENA      0x650
#define RALINK_REG_PIOFENA      0x660
#define RALINK_REG_PIODATA      0x620
#define RALINK_REG_PIODIR       0x600
#define RALINK_REG_PIOSET       0x630
#define RALINK_REG_PIORESET     0x640

#define RALINK_REG_PIO6332INT       0x694
#define RALINK_REG_PIO6332EDGE      0x6A4
#define RALINK_REG_PIO6332RENA      0x654
#define RALINK_REG_PIO6332FENA      0x664
#define RALINK_REG_PIO6332DATA      0x624
#define RALINK_REG_PIO6332DIR       0x604
#define RALINK_REG_PIO6332SET       0x634
#define RALINK_REG_PIO6332RESET     0x644

#define RALINK_REG_PIO9564INT       0x698
#define RALINK_REG_PIO9564EDGE      0x6A8
#define RALINK_REG_PIO9564RENA      0x658
#define RALINK_REG_PIO9564FENA      0x668
#define RALINK_REG_PIO9564DATA      0x628
#define RALINK_REG_PIO9564DIR       0x608
#define RALINK_REG_PIO9564SET       0x638
#define RALINK_REG_PIO9564RESET     0x648

typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    uint8_t id;
} machine_pin_obj_t;

STATIC const machine_pin_obj_t machine_pin_obj[] = {
    {{&machine_pin_type}, 0},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{&machine_pin_type}, 14},
    {{&machine_pin_type}, 15},
    {{&machine_pin_type}, 16},
    {{&machine_pin_type}, 17},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{&machine_pin_type}, 39},
    {{&machine_pin_type}, 40},
    {{&machine_pin_type}, 41},
    {{&machine_pin_type}, 42},
};

static uint8_t* gpio_mmap_reg = NULL;
static int gpio_mmap_fd = 0;

void machine_pins_init(void) {
    static bool isinit = false;
    if (!isinit){
        
        //user space mapping
        if((gpio_mmap_fd = open(MMAP_PATH, O_RDWR))< 0){
            fprintf(stderr, "Unable to open mmap file");
            return;
        }

        gpio_mmap_reg = (uint8_t*)mmap(NULL, 1024, PROT_READ|PROT_WRITE,
                MAP_FILE | MAP_SHARED, gpio_mmap_fd, 0x10000000);
        if (gpio_mmap_reg == MAP_FAILED){
            fprintf(stderr, "Failed to mmap");
            machine_pins_deinit();
            return;
        }
    }
}

void machine_pins_deinit(void) {
    gpio_mmap_reg = NULL;
    close(gpio_mmap_fd);
}

int mt76x8_gpio_get_pin(int pin)
{
    uint32_t tmp = 0;

    /* MT7621, MT7628 */
    if (pin <= 31) {
            tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIODATA);
            tmp = (tmp >> pin) & 1u;
    } else if (pin <= 63) {
            tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332DATA);
            tmp = (tmp >> (pin-32)) & 1u;
    } else if (pin <= 95) {
            tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564DATA);
            tmp = (tmp >> (pin-64)) & 1u;
            tmp = (tmp >> (pin-24)) & 1u;
    }
    return tmp;
}

void mt76x8_gpio_set_pin_direction(int pin, int is_output)
{
    uint32_t tmp;

    /* MT7621, MT7628 */
    if (pin <= 31) {
        tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIODIR);
        if (is_output)
            tmp |=  (1u << pin);
        else
            tmp &= ~(1u << pin);
        *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIODIR) = tmp;
    } else if (pin <= 63) {
        tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332DIR);
        if (is_output)
            tmp |=  (1u << (pin-32));
        else
            tmp &= ~(1u << (pin-32));
        *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332DIR) = tmp;
    } else if (pin <= 95) {
        tmp = *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564DIR);
        if (is_output)
            tmp |=  (1u << (pin-64));
        else
            tmp &= ~(1u << (pin-64));
        *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564DIR) = tmp;
    }
}

void mt76x8_gpio_set_pin_value(int pin, int value)
{
    uint32_t tmp;

    /* MT7621, MT7628 */
    if (pin <= 31) {
        tmp = (1u << pin);
        if (value)
            *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIOSET) = tmp;
        else
            *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIORESET) = tmp;
    } else if (pin <= 63) {
        tmp = (1u << (pin-32));
        if (value)
            *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332SET) = tmp;
        else
            *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO6332RESET) = tmp;
    } else if (pin <= 95) {
        tmp = (1u << (pin-64));
        if (value)
            *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564SET) = tmp;
        else
            *(volatile uint32_t *)(gpio_mmap_reg + RALINK_REG_PIO9564RESET) = tmp;
    }
}


STATIC void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    machine_pin_obj_t *self = self_in;
    mp_printf(print, "Pin(%u)", self->id);
}

// pin.init(mode, pull=None, *, value)
STATIC mp_obj_t machine_pin_obj_init_helper(const machine_pin_obj_t *self, size_t n_args, const mp_obj_t *pos_args,  mp_map_t *kw_args){
    enum {ARG_mode, ARG_pull, ARG_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_pull, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    };

    //parse args 
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    //configure the pin for gpio
    //
    //set initial value (do this before configuring mode/pull)
    if (args[ARG_value].u_obj != MP_OBJ_NULL){
        mt76x8_gpio_set_pin_value(self->id, mp_obj_is_true(args[ARG_value].u_obj));
    }
    //
    //configure mode 
    //
    if (args[ARG_mode].u_obj != mp_const_none){
        mt76x8_gpio_set_pin_direction(self->id, mp_obj_get_int(args[ARG_mode].u_obj));
    }
    //configure pull 
    //
    return mp_const_none;
}

// constructor(id, ...)
STATIC mp_obj_t machine_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    //get the wanted pin object
    int wanted_pin = mp_obj_get_int(args[0]);
    printf("wanted_pin = %d\n", wanted_pin);
    const machine_pin_obj_t *self = NULL;
    if (0 <= wanted_pin && wanted_pin < MP_ARRAY_SIZE(machine_pin_obj)){
        self = (machine_pin_obj_t*)&machine_pin_obj[wanted_pin];
    }
    if (self == NULL || self->base.type == NULL){
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid pin"));
    }

    if (n_args > 1 || n_kw > 0) {
        //pin mode given, so configure this GPIO
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args+n_args);
        machine_pin_obj_init_helper(self, n_args-1, args+1, &kw_args);
    }
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t machine_pin_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args){
    return mp_const_none;
}

// pin.init(mode, pull)
STATIC mp_obj_t machine_pin_obj_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args){
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_init_obj, 1, machine_pin_obj_init);

STATIC const mp_rom_map_elem_t machine_pin_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pin_init_obj) },

    //class constants
    { MP_ROM_QSTR(MP_QSTR_IN), MP_ROM_INT(GPIO_MODE_INPUT) },
    { MP_ROM_QSTR(MP_QSTR_OUT), MP_ROM_INT(GPIO_MODE_INPUT_OUTPUT) },
};

STATIC MP_DEFINE_CONST_DICT(machine_pin_locals_dict, machine_pin_locals_dict_table);

const mp_obj_type_t machine_pin_type = {
    { &mp_type_type },
    .name = MP_QSTR_Pin,
    .print = machine_pin_print,
    .make_new = machine_pin_make_new,
    .call = machine_pin_call,
    .locals_dict = (mp_obj_t)&machine_pin_locals_dict,
};
