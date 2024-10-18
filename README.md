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
