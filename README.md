# EzReg 
C语言模拟寄存器  

1. 通过寄存器地址绑定变量地址，实现实时读写
2. 每个寄存器拥有读、写、存三种权限
3. 带有存储接口、外部通信读写接口

## 函数说明
### ez_reg_create
创建一个寄存器  
|参数|类型|含义|
|:--:|:--:|:--:|
|start_addr|uint16_t|起始地址|
|perm|reg_perm_t|寄存器权限|
|bindings|void *|绑定变量地址|
|bytelen|uint16_t|绑定的字节数|

### ez_reg_create_patch
批量创建连续寄存器  
|参数|类型|含义|
|:--:|:--:|:--:|
|start_addr|uint16_t|起始地址|
|perm|reg_perm_t|寄存器权限|
|bindings|void **|绑定变量指针数组|
|bind_len|uint16_t|指针数组长度|
|bind_size|uint16_t|每个元素的大小|

### ez_reg_operation
操作寄存器  
|参数|类型|含义|
|:--:|:--:|:--:|
|addr|uint16_t|寄存器地址|
|data|uint8_t *|操作数据指针|
|len|uint16_t|数据长度|
|op|reg_op_t|操作类型|


### ez_reg_save
保存所有带有存储权限的寄存器(需要移植`ez_reg_port_save`函数)

### ez_reg_load
加载所有带有存储权限的寄存器(需要移植`ez_reg_port_load`函数)


### ez_reg_destroy_all
销毁所有寄存器


## 可选外部通信协议
### 读寄存器操作
指令如下：
||数值|说明|
|:--:|:--:|:--:|
|byte[0]|0x7B|包头|
|byte[1]|0xA0|包头|
|byte[2]|ID|手ID|
|byte[3]|0x04|数据长度|
|byte[4]|0x11|读寄存器|
|byte[5]|ADDR_L|寄存器起始地址低八位|
|byte[6]|ADDR_H|寄存器起始地址高八位|
|byte[7]|Reg_Length|读取寄存器长度|
|byte[8]|Check_Sum|除包头以外校验和|


### 回复如下：
||数值|说明|
|:--:|:--:|:--:|
|byte[0]|0xA0|包头|
|byte[1]|0x7B|包头|
|byte[2]|ID|手ID|
|byte[3]|Reg_Length+3|数据长度|
|byte[4]|0x11|读寄存器回复|
|byte[5]|ADDR_L|寄存器起始地址低八位|
|byte[6]|ADDR_H|寄存器起始地址高八位|
|byte[7]~byte[7+Reg_Length-1]| \ |寄存器值|
|byte[7+Reg_Length]|Check Sum|除包头以外校验和|

### 写寄存器操作
指令如下：
||数值|说明|
|:--:|:--:|:--:|
|byte[0]|0x7B|包头|
|byte[1]|0xA0|包头|
|byte[2]|ID|手ID|
|byte[3]|Reg_Length+3|数据长度|
|byte[4]|0x12|写寄存器|
|byte[5]|ADDR_L|寄存器起始地址低八位|
|byte[6]|ADDR_H|寄存器起始地址高八位|
|byte[7]~byte[7+Reg_Length-1]| \ |寄存器值|
|byte[7+Reg_Length]|Check Sum|除包头以外校验和|


### 回复如下：
||数值|说明|
|:--:|:--:|:--:|
|byte[0]|0xA0|包头|
|byte[1]|0x7B|包头|
|byte[2]|ID|手ID|
|byte[3]|0x04|数据长度|
|byte[4]|0x12|写寄存器回复|
|byte[5]|ADDR_L|寄存器起始地址低八位|
|byte[6]|ADDR_H|寄存器起始地址高八位|
|byte[7]|1|固定值|
|byte[8]|Check_Sum|除包头以外校验和|