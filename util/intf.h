#ifndef _INTF_H_
#define _INTF_H_

#define DEVICE_ID       "DLP2232M A"    /* DLP-2232M device ID string */

#ifdef REV2

#define REG_RX          0x00    /* Latch driving resistors on B7-0 */
#define REG_IXL         0x01    /* Latch driving I7-0 */
#define REG_IXH         0x02    /* Latch driving I9-8 */
#define REG_BX          0x00    /* Buffer to input B7-0 state */

#else

#define REG_RX          0x00    /* Latch driving resistors on B7-0 */
#define REG_IXL         0x01    /* Latch driving I7-0 */
#define REG_IXH         0x02    /* Latch driving I9-8 */
#define REG_BXGA1       0x03    /* Latch driving B7-0 output level and enable (1/2) */
#define REG_BXGA2       0x04    /* Latch driving B7-0 output level and enable (2/2) */
#define REG_BX          0x05    /* Buffer to input B7-0 state */

#endif

/* Function prototypes */
int intf_open(void);
void intf_reset(int devno);
void intf_close(int devno);
int acquire_device(void);
int read_bx(uint8 *cmd);
int set_rx(uint8 *cmd, uint8 level);
int set_ix(uint8 *cmd, uint16 level);
int set_bx(uint8 *cmd, uint8 level, uint8 enable);
int read_bx_flush(uint8 *cmd);

#endif /* _INTF_H_ */
