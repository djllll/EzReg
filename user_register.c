
#include "user_register.h"
#include "stdlib.h"

static user_register_t *head = NULL;
static uint16_t bytes_num_need_save = 0; // count of bytes need to save
static uint8_t save_flash_check = 0x04;     // check the flash saved flag ,default value is a seed;

user_register_t *user_register_create(uint16_t start_addr, reg_perm_t perm, uint8_t *bindings, uint8_t bytelen);
user_register_t *user_register_create_patch(uint16_t start_addr, reg_perm_t perm, uint8_t **bindings, uint16_t bind_len, uint8_t bind_size);
void user_register_destroy_all(void);
void user_register_operation(uint16_t addr, uint8_t *data, uint16_t len, reg_op_t op);
reg_save_load_ret_t user_register_save(void);
reg_save_load_ret_t user_register_load(void);

static reg_save_load_ret_t user_register_port_save(uint8_t *data, uint16_t len);
static reg_save_load_ret_t user_register_port_load(uint8_t *data, uint16_t len);


/**
 * @brief create a register
 *
 * @param start_addr
 * @param bytelen
 * @param perm
 * @return user_register_t*
 */
user_register_t *user_register_create(uint16_t start_addr, reg_perm_t perm, uint8_t *bindings, uint8_t bytelen)
{
    if (bytelen == 0) {
        return NULL;
    }
    user_register_t *reg = (user_register_t *)malloc(sizeof(user_register_t));
    if (reg == NULL) {
        return NULL;
    }
    reg->addr = start_addr;
    reg->bytelen = bytelen;
    reg->p_reg = bindings;
    reg->next = NULL;
    reg->perm = perm;
    if ((perm & REG_PERM_S) != 0) {
        bytes_num_need_save += bytelen;
        save_flash_check = (uint8_t)(save_flash_check + start_addr);
        if(save_flash_check==0xff){  //avoid erased flash 0xff
            save_flash_check--;
        }
    }
    if (head == NULL) {
        head = reg;
    } else {
        if (reg->addr < head->addr) {
            /* head insert */
            user_register_t *temp = head;
            head = reg;
            head->next = temp;
        } else {
            /* insert sorted by addr */
            user_register_t *p = head;
            while (p->next != NULL) {
                if (reg->addr < p->next->addr) {
                    user_register_t *temp = p->next;
                    p->next = reg;
                    reg->next = temp;
                    return reg;
                }
                p = p->next;
            }
            p->next = reg;
        }
    }
    return reg;
}


/**
 * @brief create register patch
 *
 * @param start_addr
 * @param perm
 * @param bindings
 * @param bind_len
 * @param bind_size
 * @return user_register_t*  register last created
 */
user_register_t *user_register_create_patch(uint16_t start_addr, reg_perm_t perm, uint8_t **bindings, uint16_t bind_len, uint8_t bind_size)
{
    user_register_t *last_create = NULL;
    for (uint16_t i = 0; i < bind_len; i++) {
        last_create = user_register_create(start_addr + bind_size * i, perm, bindings[i], bind_size);
    }
    return last_create;
}


/**
 * @brief destroy registers
 *
 */
void user_register_destroy_all(void)
{
    user_register_t *p = head;
    while (p != NULL) {
        user_register_t *next = p->next;
        free(p);
        p = next;
    }
    head = NULL;
    bytes_num_need_save = 0;
}


/**
 * @brief operate registers
 *
 * @param addr
 * @param data
 * @param len
 * @param op
 */
void user_register_operation(uint16_t addr, uint8_t *data, uint16_t len, reg_op_t op)
{
    user_register_t *p = head;
    while (p != NULL) {
        if (p->addr >= addr && p->addr <= addr + len) {
            uint16_t p_end = p->addr + p->bytelen;
            uint16_t op_end = addr + len;
            uint16_t real_len = op_end > p_end ? p_end - p->addr : op_end - p->addr;
            for (uint16_t i = 0; i < real_len; i++) {
                if (op == OP_REG_READ && ((p->perm & REG_PERM_R) != 0)) {
                    data[p->addr - addr + i] = p->p_reg[i];
                } else if (op == OP_REG_WRITE && (p->perm & REG_PERM_W) != 0) {
                    p->p_reg[i] = data[p->addr - addr + i];
                }
            }
        } else if (p->addr > addr + len) {
            break;
        }
        p = p->next;
    }
}


/**
 * @brief save all registers with PERM_S
 *
 */
reg_save_load_ret_t user_register_save(void)
{
    if (bytes_num_need_save == 0) {
        return REG_SAVE_ERR;
    }
    uint8_t bytesbuf[bytes_num_need_save + 1];
    uint16_t bufp = 0;
    user_register_t *p = head;
    while (p != NULL) {
        if ((p->perm & REG_PERM_S) != 0) {
            for (uint16_t i = 0; i < p->bytelen; i++) {
                if (bufp < bytes_num_need_save + 1) {
                    bytesbuf[bufp + 1] = p->p_reg[i];
                    bufp++;
                } else {
                    return REG_SAVE_ERR;
                }
            }
        }
        p = p->next;
    }
    bytesbuf[0] = save_flash_check;
    return user_register_port_save(bytesbuf, bytes_num_need_save + 1);
}


/**
 * @brief load all registers with PERM_S
 *
 */
reg_save_load_ret_t user_register_load(void)
{
    if (bytes_num_need_save == 0) {
        return REG_LOAD_ERR;
    }
    uint8_t bytesbuf[bytes_num_need_save + 1];
    user_register_port_load(bytesbuf, bytes_num_need_save + 1);
    if (bytesbuf[0] != save_flash_check) {
        return REG_LOAD_ERR;
    }
    uint16_t bufp = 0;
    user_register_t *p = head;
    while (p != NULL) {
        if ((p->perm & REG_PERM_S) != 0) {
            for (uint16_t i = 0; i < p->bytelen; i++) {
                if (bufp < bytes_num_need_save + 1) {
                    p->p_reg[i] = bytesbuf[bufp + 1];
                    bufp++;
                } else {
                    return REG_SAVE_ERR;
                }
            }
        }
        p = p->next;
    }
    return REG_SAVE_LOAD_OK;
}


/* ************************* port ******************************* */

/* user include */
#include "flash_operation.h"

/**
 * @brief port function to save
 *
 * @param data
 * @param len
 * @return reg_save_load_ret_t
 */
static reg_save_load_ret_t user_register_port_save(uint8_t *data, uint16_t len)
{
    return flash_save(FLASH_AREA_1,data, len) == FLASH_OK ? REG_SAVE_LOAD_OK : REG_SAVE_ERR;
}


/**
 * @brief port function to load
 *
 * @param data
 * @param len
 * @return reg_save_load_ret_t
 */
static reg_save_load_ret_t user_register_port_load(uint8_t *data, uint16_t len)
{
    return flash_load(FLASH_AREA_1,data, len,0) == FLASH_OK ? REG_SAVE_LOAD_OK : REG_LOAD_ERR;
}
