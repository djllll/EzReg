
#include "ezreg.h"
#include "stdlib.h"


static ez_reg_t *head = NULL;
static uint16_t bytes_num_need_save = 0; // count of bytes need to save
static uint8_t save_flash_check = 0x04;  // check the flash saved flag ,default value is a seed;

#if (USE_EXTERNAL_COMMU == 1)
#define WAITING_HEAD1     0
#define WAITING_HEAD2     1
#define WAITING_ID        2
#define WAITING_LEN       3
#define WAITING_RW        4
#define WAITING_DATA      5
#define WAITING_CHECK_SUM 6

#define CMD_HEAD1 0x7B
#define CMD_HEAD2 0xA0
#define CMD_READ  0x11
#define CMD_WRITE 0x12
static uint8_t match_status = 0;
static uint8_t slave_id = 1;
#endif


ez_reg_t *ez_reg_create(uint16_t start_addr, reg_perm_t perm, void *bindings, uint16_t bytelen);
ez_reg_t *ez_reg_create_patch(uint16_t start_addr, reg_perm_t perm, void **bindings, uint16_t bind_len, uint16_t bind_size);
void ez_reg_destroy_all(void);
void ez_reg_operation(uint16_t addr, uint8_t *data, uint16_t len, reg_op_t op);
reg_ret_t ez_reg_save(void);
reg_ret_t ez_reg_load(void);
void ez_reg_set_slave_id(uint8_t id);


/**
 * @brief create a register
 *
 * @param start_addr
 * @param bytelen
 * @param perm
 * @return ez_reg_t*
 */
ez_reg_t *ez_reg_create(uint16_t start_addr, reg_perm_t perm, void *bindings, uint16_t bytelen)
{
    if (bytelen == 0) {
        return NULL;
    }
    ez_reg_t *reg = (ez_reg_t *)malloc(sizeof(ez_reg_t));
    if (reg == NULL) {
        return NULL;
    }
    reg->addr = start_addr;
    reg->bytelen = bytelen;
    reg->p_reg = (uint8_t *)bindings;
    reg->next = NULL;
    reg->perm = perm;
    if ((perm & REG_PERM_S) != 0) {
        bytes_num_need_save += bytelen;
        save_flash_check = (uint8_t)(save_flash_check + start_addr);
        if (save_flash_check == 0xff) { // avoid erased flash 0xff
            save_flash_check--;
        }
    }
    if (head == NULL) {
        head = reg;
    } else {
        if (reg->addr < head->addr) {
            /* head insert */
            ez_reg_t *temp = head;
            head = reg;
            head->next = temp;
        } else {
            /* insert sorted by addr */
            ez_reg_t *p = head;
            while (p->next != NULL) {
                if (reg->addr < p->next->addr) {
                    ez_reg_t *temp = p->next;
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
 * @return ez_reg_t*  register last created
 */
ez_reg_t *ez_reg_create_patch(uint16_t start_addr, reg_perm_t perm, void **bindings, uint16_t bind_len, uint16_t bind_size)
{
    ez_reg_t *last_create = NULL;
    for (uint16_t i = 0; i < bind_len; i++) {
        last_create = ez_reg_create(start_addr + bind_size * i, perm, bindings[i], bind_size);
    }
    return last_create;
}


/**
 * @brief destroy registers
 *
 */
void ez_reg_destroy_all(void)
{
    ez_reg_t *p = head;
    while (p != NULL) {
        ez_reg_t *next = p->next;
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
void ez_reg_operation(uint16_t addr, uint8_t *data, uint16_t len, reg_op_t op)
{
    ez_reg_t *p = head;
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
reg_ret_t ez_reg_save(void)
{
    if (bytes_num_need_save == 0) {
        return REG_SAVE_ERR;
    }
    uint8_t bytesbuf[bytes_num_need_save + 1];
    uint16_t bufp = 0;
    ez_reg_t *p = head;
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
    return ez_reg_port_save(bytesbuf, bytes_num_need_save + 1);
}


/**
 * @brief load all registers with PERM_S
 *
 */
reg_ret_t ez_reg_load(void)
{
    if (bytes_num_need_save == 0) {
        return REG_LOAD_ERR;
    }
    uint8_t bytesbuf[bytes_num_need_save + 1];
    ez_reg_port_load(bytesbuf, bytes_num_need_save + 1);
    if (bytesbuf[0] != save_flash_check) {
        return REG_LOAD_ERR;
    }
    uint16_t bufp = 0;
    ez_reg_t *p = head;
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
    return REG_OK;
}


#if (USE_EXTERNAL_COMMU == 1)
/**
 * @brief set slave id
 *
 * @param id
 * @return reg_ret_t
 */
void ez_reg_set_slave_id(uint8_t id)
{
    slave_id = id;
}


/**
 * @brief ex_communication
 *
 */
void ex_commu_operate_tick(void)
{
    static uint8_t data_buf[CMD_DATA_BUF_LEN];
    static uint8_t bufp = 0;
    uint8_t data;
    while (ez_reg_port_commu_get_a_char(&data) == COMMU_DATA_OK) {
        switch (match_status) {
        case WAITING_HEAD1:
            match_status = (data == CMD_HEAD1 ? match_status + 1 : WAITING_HEAD1);
            data_buf[0] = CMD_HEAD2;
            break;
        case WAITING_HEAD2:
            match_status = (data == CMD_HEAD2 ? match_status + 1 : WAITING_HEAD1);
            data_buf[1] = CMD_HEAD1;
            break;
        case WAITING_ID:
            match_status = (data == slave_id ? match_status + 1 : WAITING_HEAD1);
            data_buf[2] = data;
            break;
        case WAITING_LEN:
            data_buf[3] = data; // data len
            if (data_buf[3] > CMD_DATA_BUF_LEN - 10) {
                match_status = WAITING_HEAD1;
            } else {
                match_status++;
            }
            break;
        case WAITING_RW:
            match_status = ((data == CMD_READ || data == CMD_WRITE) ? match_status + 1 : WAITING_HEAD1);
            data_buf[4] = data;
            bufp = 5;
            break;
        case WAITING_DATA:
            if (bufp < data_buf[3] + 4) {
                data_buf[bufp] = data;
                if (++bufp == data_buf[3] + 4) {
                    match_status++;
                }
            }
            break;
        case WAITING_CHECK_SUM: {
            uint8_t check_sum = 0;
            for (uint8_t i = 2; i < data_buf[3] + 4; i++) {
                check_sum = (uint8_t)(check_sum + data_buf[i]);
            }
            if (check_sum == data) {
                uint8_t reg_len = 0;
                uint8_t check_sum_idx = 0;
                if (data_buf[4] == CMD_READ) {
                    reg_len = data_buf[7];
                    check_sum_idx = 7 + reg_len;
                    ez_reg_operation(((uint16_t)data_buf[6] << 8) | data_buf[5], &data_buf[7], reg_len, OP_REG_READ);
                    data_buf[3] = reg_len + 3;
                } else if (data_buf[4] == CMD_WRITE && data_buf[3] > 3) {
                    ez_reg_operation(((uint16_t)data_buf[6] << 8) | data_buf[5], &data_buf[7], data_buf[3] - 3, OP_REG_WRITE);
                    data_buf[3] = 4;
                    data_buf[7] = 1;
                    reg_len = 1;
                    check_sum_idx = 8;
                }
                /* check sum and reply */
                data_buf[check_sum_idx] = 0;
                for (uint8_t i = 2; i < check_sum_idx; i++) {
                    data_buf[check_sum_idx] = (uint8_t)(data_buf[check_sum_idx] + data_buf[i]);
                }
                ez_reg_port_commu_send(data_buf, reg_len + 8);
            }
            match_status = WAITING_HEAD1;
        } break;
        default:
            break;
        }
    }
}
#endif
