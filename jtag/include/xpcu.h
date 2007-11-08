#ifndef XPCU_H
#define XPCU_H 1

#include <stdint.h>
#include <usb.h>

#define XPCU_VID 0x03FD
#define XPCU_PID 0x0008

struct usb_device *find_xpcu(void);
int xpcu_init();
int xpcu_close(struct usb_dev_handle *xpcu);
int xpcu_request_28(struct usb_dev_handle *xpcu, int value);
int xpcu_raise_ioa5(struct usb_dev_handle *xpcu);
int xpcu_write_gpio(struct usb_dev_handle *xpcu, uint8_t bits);
int xpcu_read_gpio(struct usb_dev_handle *xpcu, uint8_t *bits);
int xpcu_bitrev_test(struct usb_dev_handle *xpcu);
int xpcu_select_gpio(struct usb_dev_handle *xpcu, int select);
int xpcu_open(struct usb_dev_handle **xpcu);
int xpcu_request_a6(struct usb_dev_handle *xpcu, int nibbles, uint8_t *xmit, int inlen, uint8_t *recv);


#endif /* XPCU_H */

