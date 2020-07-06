/*
    usb.c --
    DLP-USB245M interface routines.
*/
#include "shared.h"

FT_STATUS status;
FT_HANDLE handle;
FILE *usb_logfile = NULL;
DWORD device_no;
DWORD num_devices;

#define BLOCK_SIZE      0x8000

void usb_get(uint8 *buffer, uint32 size)
{
    DWORD count;
    uint32 i;
    uint32 block_cnt = (size / BLOCK_SIZE);
    uint32 block_rem = (size % BLOCK_SIZE);

    for(i = 0; i < block_cnt; i++)
    {
        size = BLOCK_SIZE;

        status = FT_Read(handle, buffer, size, &count);

        if(status != FT_OK)
            die("Read status (block %d of %d): %s\n", i, block_cnt-1, usb_getstatus(status));
    
        if(count != size)
            die("Read count (block %d of %d) was %d, expected %d\n", i, block_cnt-1, count, size);

        buffer += BLOCK_SIZE;
    }

    if(block_rem)
    {
        size = block_rem;

        status = FT_Read(handle, buffer, size, &count);

        if(status != FT_OK)
            die("Read status (block remainder): %s\n", i, block_cnt-1, usb_getstatus(status));
    
        if(count != size)
            die("Read count (block remainder) was %d, expected %d\n", i, block_cnt-1, count, size);
    }
}

void usb_send(uint8 *buffer, uint32 size)
{
    DWORD count;
    uint32 i;
    uint32 block_cnt = (size / BLOCK_SIZE);
    uint32 block_rem = (size % BLOCK_SIZE);

    for(i = 0; i < block_cnt; i++)
    {
        size = BLOCK_SIZE;

        status = FT_Write(handle, buffer, size, &count);

        if(status != FT_OK)
            die("Write status (block %d of %d): %s\n", i, block_cnt-1, usb_getstatus(status));
    
        if(count != size)
            die("Write count (block %d of %d) was %d, expected %d\n", i, block_cnt-1, count, size);

        buffer += BLOCK_SIZE;
    }

    if(block_rem)
    {
        size = block_rem;

        status = FT_Write(handle, buffer, size, &count);

        if(status != FT_OK)
            die("Write status (block remainder): %s\n", i, block_cnt-1, usb_getstatus(status));
    
        if(count != size)
            die("Write count (block remainder) was %d, expected %d\n", i, block_cnt-1, count, size);
    }
}

void usb_reset(void)
{
    status = FT_ResetDevice(handle);
    if(status != FT_OK)
        die("Reset: status: %s\n", usb_getstatus(status));
}

void usb_purge(void)
{
    status = FT_Purge(handle, FT_PURGE_RX | FT_PURGE_TX);
    if(status != FT_OK)
        die("Purge: status: %s\n", usb_getstatus(status));
}

void usb_init(void)
{
    status = FT_ListDevices(&num_devices, NULL, FT_LIST_NUMBER_ONLY);
    if(status != FT_OK)
        die("ListDevices: Get Count: status: %s\n", usb_getstatus(status));

    if(!num_devices)
        die("No devices connected.\n");

    /* FIXME: hardcoded device number (0= 1st DLP-USB245M module present) */
    device_no = 0;

    status = FT_Open(device_no, &handle);
    if(status != FT_OK)
        die("Open status: %s\n", usb_getstatus(status));

    /* Reset device */
    usb_reset();

    /* Flush buffers */
    usb_purge();

    /* FIXME; hardcoded output stream (stdout) */
    usb_logfile = stdout;

    atexit(usb_shutdown);
}

void usb_shutdown(void)
{
    status = FT_Close(handle);
    if(status != FT_OK)
        die("Close status: %s\n", usb_getstatus(status));

    if(usb_logfile)
    {
        fflush(usb_logfile);
        fclose(usb_logfile);
    }
}

char *usb_getstatus(FT_STATUS status)
{
    switch(status)
    {
        case FT_OK:
            return "Ok";
        case FT_INVALID_HANDLE:
            return "Invalid handle";
        case FT_DEVICE_NOT_FOUND:
            return "Device not found";
        case FT_DEVICE_NOT_OPENED:
            return "Device not opened";
        case FT_IO_ERROR:
            return "I/O error";
        case FT_INSUFFICIENT_RESOURCES:
            return "Insufficient resouces";
        case FT_INVALID_PARAMETER:
            return "Invalid parameter";
        case FT_INVALID_BAUD_RATE:
            return "Invalid baud rate";
        case FT_DEVICE_NOT_OPENED_FOR_ERASE:
            return "Device not open for erase";
        case FT_DEVICE_NOT_OPENED_FOR_WRITE:
            return "Device not open for write";
        case FT_FAILED_TO_WRITE_DEVICE:
            return "Failed to write device";
        case FT_EEPROM_READ_FAILED:
            return "EEPROM read failed";
        case FT_EEPROM_WRITE_FAILED:
            return "EEPROM write failed";
        case FT_EEPROM_NOT_PRESENT:
            return "EEPROM not present";
        case FT_EEPROM_NOT_PROGRAMMED:
            return "EEPROM not programmed";
        case FT_INVALID_ARGS:
            return "Invalid args";
        case FT_NOT_SUPPORTED:
            return "Not supported";
        case FT_OTHER_ERROR:
            return "Other error";
    }
    return NULL;
}

void usb_logerror(int level, char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(usb_logfile, format, ap);
    va_end(ap);
}

/*--------------------------------------------------------------------------*/
/* Send data                                                                */
/*--------------------------------------------------------------------------*/

void usb_sendb(uint8 value)
{
    uint8 buffer[1];
    buffer[0] = value;
    usb_send(buffer, sizeof(buffer));
}

void usb_sendw(uint16 value)
{
    uint8 buffer[2];
    buffer[0] = (value >>  8) & 0xFF;
    buffer[1] = (value >>  0) & 0xFF;
    usb_send(buffer, sizeof(buffer));
}

void usb_sendl(uint32 value)
{
    uint8 buffer[4];
    buffer[0] = (value >> 24) & 0xFF;
    buffer[1] = (value >> 16) & 0xFF;
    buffer[2] = (value >>  8) & 0xFF;
    buffer[3] = (value >>  0) & 0xFF;
    usb_send(buffer, sizeof(buffer));
}

/*--------------------------------------------------------------------------*/
/* Get data                                                                 */
/*--------------------------------------------------------------------------*/

uint8 usb_getb(void)
{
    uint8 value = 0;
    uint8 buffer[1];
    usb_get(buffer, sizeof(buffer));
    value = buffer[0];    
    return value;
}

uint16 usb_getw(void)
{
    uint16 value = 0;
    uint8 buffer[2];
    usb_get(buffer, sizeof(buffer));
    value |= (buffer[0] <<  0);
    value |= (buffer[1] <<  8);
    return value;
}

uint32 usb_getl(void)
{
    uint32 value = 0;
    uint8 buffer[4];
    usb_get(buffer, sizeof(buffer));
    value |= (buffer[0] <<  0);
    value |= (buffer[1] <<  8);
    value |= (buffer[2] << 16);
    value |= (buffer[3] << 24);
    return value;
}
