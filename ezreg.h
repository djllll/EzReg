#ifndef _ez_reg_H_
#define _ez_reg_H_

#include "stdint.h"

typedef uint8_t reg_perm_t;

typedef struct st_register {
    struct st_register *next;
    uint16_t addr;
    uint16_t bytelen;
    uint8_t bindsize;
    uint8_t perm;
    uint8_t *p_reg;
} ez_reg_t;


typedef enum {
    OP_REG_READ = 0,
    OP_REG_WRITE,
} reg_op_t;

typedef enum {
    REG_SAVE_LOAD_OK = 0,
    REG_SAVE_ERR,
    REG_LOAD_ERR
} reg_save_load_ret_t;

#define REG_PERM_R (0x01)
#define REG_PERM_W (0x02)
#define REG_PERM_S (0x04)

#define REG_PERM_RW  (0x03)
#define REG_PERM_RWS (0x07)


extern ez_reg_t *ez_reg_create(uint16_t start_addr, reg_perm_t perm, void *bindings, uint16_t bytelen);
extern ez_reg_t *ez_reg_create_patch(uint16_t start_addr, reg_perm_t perm, void **bindings, uint16_t bind_len, uint16_t bind_size);
extern void ez_reg_destroy_all(void);
extern void ez_reg_operation(uint16_t addr, uint8_t *data, uint16_t len, reg_op_t op);
extern reg_save_load_ret_t ez_reg_save(void);
extern reg_save_load_ret_t ez_reg_load(void);

#endif
