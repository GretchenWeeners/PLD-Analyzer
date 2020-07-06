#ifndef _COMMS_H_
#define _COMMS_H_

#define FUNC_DOWNLOAD   0x01            /* AR firmware function no. */
#define FUNC_UPLOAD     0x09            /* AR firmware function no. */
#define FLAG_LOAD       0               /* Load data */
#define FLAG_EXEC       1               /* Load data and JSR to address */
#define AR_DATA         0x00            /* Data port */

/* Function prototypes */
void comms_init(int which);
void comms_shutdown(int which);
uint8 comms_exchange_byte(int which, uint8 data);
uint8 comms_get_byte(int which);
void comms_send_byte(int which, uint8 data);
int comms_sync(int which);
uint32 comms_get_long(int which);
void comms_send_long(int which, uint32 data);
void comms_exchange(int which, uint8 *txbuf, uint8 *rxbuf, int size);
void comms_send(int which, uint8 *buffer, int size);
void comms_get(int which, uint8 *buffer, int size);
int par_download_memory(int which, uint32 addr, uint32 size, uint8 *buffer);
int par_upload_memory(int which, uint32 addr, uint32 size, uint8 *buffer, uint8 mode);

#endif /* _COMMS_H_ */
