#ifndef _SHARED_H_
#define _SHARED_H_

#define REV2

#ifndef REV1
#ifndef REV2
#error "No hardware revision defined."
#else
#endif
#endif

#include <stdio.h>
#include <windows.h>
#include "ftd2xx.h"
#include <zlib.h>

#include "mytypes.h"
#include "usb.h"
#include "mcu.h"
#include "intf.h"
#include "main.h"

#endif /* _SHARED_H_ */
