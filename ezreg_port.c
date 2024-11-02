#include "ezreg_port.h"



/* user include here */


/**
 * @brief port function to save
 *
 * @param data
 * @param len
 * @return reg_ret_t
 */
reg_ret_t ez_reg_port_save(uint8_t *data, uint16_t len)
{
    return REG_SAVE_ERR;
}


/**
 * @brief port function to load
 *
 * @param data
 * @param len
 * @return reg_ret_t
 */
reg_ret_t ez_reg_port_load(uint8_t *data, uint16_t len)
{
    return REG_LOAD_ERR;
}


#if (USE_EXTERNAL_COMMU == 1)
/**
 * @brief send bytes buffer
 *
 * @param data
 * @param len
 */
void ez_reg_port_commu_send(uint8_t *data, uint16_t len)
{
    
}


/**
 * @brief commucation get a char,
 * if no data comming, return COMMU_NO_DATA
 * else return  COMMU_DATA_OK
 *
 * @param data
 */
uint8_t ez_reg_port_commu_get_a_char(uint8_t *data)
{
    return COMMU_NO_DATA;
}
#endif