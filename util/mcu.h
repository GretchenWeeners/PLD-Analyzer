#ifndef _MCU_H_
#define _MCU_H_

/* MPSSE commands */
#define MCU_SET_GPIOH       0x82
#define MCU_GET_GPIOH       0x83
#define MCU_SEND_IMM        0x87
#define MCU_WAIT_GPIOH1_HI  0x88
#define MCU_WAIT_GPIOH1_LO  0x89
#define MCU_READ_SHORT      0x90
#define MCU_READ            0x91
#define MCU_WRITE_SHORT     0x92
#define MCU_WRITE           0x93

/* Function prototypes*/
int mcu_get_gpioh(int which);
void mcu_set_gpioh(int which, int direction, int data);
void mcu_get(int which, int address, uint8 *buffer, int size);
void mcu_send(int which, int address, uint8 *buffer, int size);
void mcu_poke(int which, int address, int data);
int mcu_peek(int which, int address);
int mcu_flush(uint8 *buf);


#endif /* _MCU_H_*/
