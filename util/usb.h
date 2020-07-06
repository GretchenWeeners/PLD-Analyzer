
#ifndef _USB_H_
#define _USB_H_

//#define USB_BLOCK_SIZE      0xF800      /* Split transfers into this size */

#define USB_BLOCK_SIZE      0xF000      /* Split transfers into this size */
#define MAX_FTDI_DEVICE     16          /* Total # of devices supported */

#define GPIOH0              0x01
#define GPIOH1              0x02

#define GPIOH0_LO           0x00
#define GPIOH0_HI           0x01
#define GPIOH1_LO           0x00
#define GPIOH1_HI           0x02

#define GPIOH0_IN           0x00
#define GPIOH0_OUT          0x01
#define GPIOH1_IN           0x00
#define GPIOH1_OUT          0x02

#define GPIOH_DATA_MASK     (GPIOH1 | GPIOH0)
#define GPIOH_DIR_MASK      (GPIOH1 | GPIOH0)


/* Device context */
typedef struct {
    struct {
        uint8 direction;
        uint8 data;
    } gpioh;
    int opened;
    FT_HANDLE handle;
    FT_STATUS status;
    FT_DEVICE_LIST_INFO_NODE info;
} ft_device_context;

/* Global variables */
extern ft_device_context ft_device[MAX_FTDI_DEVICE];

/* Function prototypes */
char *ft_getstatus(FT_STATUS status);
void ft_checkerr(FT_STATUS status, char *msg);
int ft_scan_devices(char *type);
int ft_open_device(int which);
int ft_close_device(int which);
int ft_queue_status(int which);
int ft_get_data(int which, uint8 *buffer, int size);
int ft_send_data(int which, uint8 *buffer, int size);

#endif /* _USB_H_*/
