#include "ezreg.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

uint16_t reg1 = 100;
uint16_t reg2 = 200;
uint16_t reg3 = 300;
uint16_t reg4 = 400;

static struct {
    uint8_t test_reg1[4];
    uint8_t test_reg2[4];
    uint8_t test_reg3[4];
    uint8_t test_reg4[4];
} commu = {
    .test_reg1 = {1,2,3,4},
    .test_reg2 = {2,2,3,4},
    .test_reg3 = {3,2,3,4},
    .test_reg4 = {4,2,3,4},
};

static void init()
{
    ez_reg_create(1000, REG_PERM_RWS, (void *)&commu, sizeof(commu));
    uint16_t *bind_values_16[4] = {&reg1, &reg2, &reg3, &reg4};
    ez_reg_create_patch(2000, REG_PERM_RW, (void **)bind_values_16, 4, 2);
}


int main(void)
{
    init();
    uint8_t values[8];


    /* reading test 1 */
    printf("\r\nReading 8 Registers at 2000\r\n");
    ez_reg_operation(2000, values, 8, OP_REG_READ);
    printf("Result :");
    for (uint8_t i = 0; i < 8; i++) {
        printf("%02x,", values[i]);
    }
    printf("\r\n");


    /* reading test 2 */
    printf("\r\nReading 4 Registers at 1000\r\n");
    ez_reg_operation(1000, values, 4, OP_REG_READ);
    printf("Result :");
    for (uint8_t i = 0; i < 4; i++) {
        printf("%02x,", values[i]);
    }
    printf("\r\n");

    /* reading test 2 */
    printf("\r\nReading 4 Registers at 1004\r\n");
    ez_reg_operation(1004, values, 4, OP_REG_READ);
    printf("Result :");
    for (uint8_t i = 0; i < 4; i++) {
        printf("%02x,", values[i]);
    }
    printf("\r\n");

    /* writing test 1 */
    printf("\r\nWriting 4 Registers at 1000\r\n");
    values[0] = 0x0f;
    values[1] = 0x1e;
    values[2] = 0x2d;
    values[3] = 0x3c;
    ez_reg_operation(1000, values, 4, OP_REG_WRITE);
    printf("Result :");
    printf("%02x,%02x,%02x,%02x",
           commu.test_reg1,
           commu.test_reg2,
           commu.test_reg3,
           commu.test_reg4);
    printf("\r\n");
    printf("\r\n");


    ez_reg_destroy_all();
    return 0;
}