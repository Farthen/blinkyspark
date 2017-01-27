/* Name: requests.h
 * Project: custom-class, a basic USB example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-09
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

/* This header is shared between the firmware and the host software. It
 * defines the USB request numbers (and optionally data types) used to
 * communicate between the host and the device.
 */

#ifndef __REQUESTS_H_INCLUDED__
#define __REQUESTS_H_INCLUDED__

/* Enable or disable the device */
#define USB_RQ_ONOFF 0

/* Set auto mode */
#define USB_RQ_MODE_AUTO 2

/* Sets the LED array to one color */ 
#define USB_RQ_ANIMATION_COLOR_MODE 3

/* Configures the animation mode */
#define USB_RQ_ANIMATION_MODE 4

#define USB_RQ_ANIMATION_COLOR_DELAY 5

/* Enters bootloader */
#define USB_RQ_BOOTLOADER 0xff

#endif /* __REQUESTS_H_INCLUDED__ */
