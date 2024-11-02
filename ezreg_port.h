#ifndef _EZREG_PORT_H_
#define _EZREG_PORT_H_

#include "stdint.h"

/* config */
#define USE_EXTERNAL_COMMU 1
#define CMD_DATA_BUF_LEN   70
#define CMD_HEAD1 0x7B
#define CMD_HEAD2 0xA0

typedef enum {
    REG_OK = 0,
    REG_SAVE_ERR,
    REG_LOAD_ERR,
    COMMU_NO_DATA,
    COMMU_DATA_OK,
} reg_ret_t;

extern reg_ret_t ez_reg_port_save(uint8_t *data, uint16_t len);
extern reg_ret_t ez_reg_port_load(uint8_t *data, uint16_t len);
#if (USE_EXTERNAL_COMMU == 1)
extern void ez_reg_port_commu_send(uint8_t *data, uint16_t len);
extern uint8_t ez_reg_port_commu_get_a_char(uint8_t *data);
#endif

#endif