#ifndef _USB_H_
#define _USB_H_

/* Function prototypes */
void usb_logerror(int level, char *format, ...);
char *usb_getstatus(FT_STATUS status);
void usb_init(void);
void usb_shutdown(void);
void usb_send(uint8 *buffer, uint32 size);
void usb_sendb(uint8 value);
void usb_sendw(uint16 value);
void usb_sendl(uint32 value);
void usb_get(uint8 *buffer, uint32 size);
uint8 usb_getb(void);
uint16 usb_getw(void);
uint32 usb_getl(void);
void usb_purge(void);
void usb_reset(void);

#endif /* _USB_H_ */
