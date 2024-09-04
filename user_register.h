#ifndef _USER_REGISTER_H_
#define _USER_REGISTER_H_

#include "main.h"

typedef uint8_t reg_perm_t;

typedef struct st_register {
    struct st_register *next;
    uint16_t addr;
    uint8_t bytelen;
    uint8_t bindsize;
    uint8_t perm;
    uint8_t *p_reg;
} user_register_t;


typedef enum {
    OP_REG_READ = 0,
    OP_REG_WRITE,
} reg_op_t;

typedef enum{
    REG_SAVE_LOAD_OK = 0,
    REG_SAVE_ERR,
    REG_LOAD_ERR
}reg_save_load_ret_t;

#define REG_PERM_R (0x01)
#define REG_PERM_W (0x02)
#define REG_PERM_S (0x04)

#define REG_PERM_RW  (0x03)
#define REG_PERM_RWS (0x07)



extern user_register_t *user_register_create(uint16_t start_addr,reg_perm_t perm,uint8_t *bindings,uint8_t bytelen);
extern user_register_t *user_register_create_patch(uint16_t start_addr,reg_perm_t perm,uint8_t **bindings,uint16_t bind_len,uint8_t bind_size);
extern void user_register_destroy_all(void);
extern void user_register_operation(uint16_t addr, uint8_t *data, uint16_t len, reg_op_t op);
extern reg_save_load_ret_t user_register_save(void);
extern reg_save_load_ret_t user_register_load(void);

#endif
