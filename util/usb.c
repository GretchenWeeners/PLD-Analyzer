/*
    usb.c --
    USB device interface code.
*/
#include "shared.h"


ft_device_context ft_device[MAX_FTDI_DEVICE];

int ft_queue_status(int which)
{
    DWORD count;
    ft_device_context *p = &ft_device[which];

    /* Get queue status */
    p->status = FT_GetQueueStatus(p->handle, &count);
    ft_checkerr(p->status, "Can't get queue status.");

    return count;
}

int ft_get_data(int which, uint8 *buffer, int size)
{
    ft_device_context *p = &ft_device[which];
    uint32 i;
    uint32 block_count = (size / USB_BLOCK_SIZE);
    uint32 block_remainder = (size % USB_BLOCK_SIZE);
    DWORD bytes_read;

    int total_read = 0;
    int total_got  = 0;

    /* Loop over blocks */
    for(i = 0; i < block_count; i++)
    {
        p->status = FT_Read(p->handle, buffer, USB_BLOCK_SIZE, &bytes_read);
        ft_checkerr(p->status, "Error reading data (block)");
        buffer += USB_BLOCK_SIZE;

        total_read += USB_BLOCK_SIZE;
        total_got += bytes_read;
    }

    if(block_remainder)
    {
        p->status = FT_Read(p->handle, buffer, block_remainder, &bytes_read);
        ft_checkerr(p->status, "Error reading data (remainder)");

        total_read += block_remainder;
        total_got += bytes_read;
    }

    return 1;    
}


int ft_send_data(int which, uint8 *buffer, int size)
{
    ft_device_context *p = &ft_device[which];
    uint32 i;
    uint32 block_count = (size / USB_BLOCK_SIZE);
    uint32 block_remainder = (size % USB_BLOCK_SIZE);
    DWORD bytes_written;

    int total_wrote = 0;
    int total_sent = 0;

    /* Loop over blocks */
    for(i = 0; i < block_count; i++)
    {
        p->status = FT_Write(p->handle, buffer, USB_BLOCK_SIZE, &bytes_written);
        ft_checkerr(p->status, "Error writing data (block)");
        buffer += USB_BLOCK_SIZE;

        total_wrote += USB_BLOCK_SIZE;
        total_sent += bytes_written;
    }

    if(block_remainder)
    {
        p->status = FT_Write(p->handle, buffer, block_remainder, &bytes_written);
        ft_checkerr(p->status, "Error writing data (remainder)");
                          
        total_wrote += block_remainder;
        total_sent += bytes_written;
    }

    return 1;    
}


int ft_scan_devices(char *type)
{
    int i;
    int found_matching_devices;
    FT_STATUS status;
    FT_DEVICE_LIST_INFO_NODE *device_info_list;
    DWORD num_devices = 0;

    /* Create a list of FTDI devices attached to the PC */
    status = FT_CreateDeviceInfoList(&num_devices);
    ft_checkerr(status, "Error: Can't create device info list.");

    /* No devices present */
    if(!num_devices)
        return 0;            

    /* Retrieve the device information list */
    device_info_list = (FT_DEVICE_LIST_INFO_NODE *)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) * num_devices);
    status = FT_GetDeviceInfoList(device_info_list, &num_devices);
    ft_checkerr(status, "Error: Can't retrieve device info list.");

    /* Search for devices that match what we're looking for */
    found_matching_devices = 0;
    for(i = 0; i < num_devices; i++)
    {
        FT_DEVICE_LIST_INFO_NODE *p = &device_info_list[i];

        /* See if this device matches the type we are looking for */
        if(strstr(p->Description, type) != NULL)
        {
            /* Clear out device context */
            memset(&ft_device[found_matching_devices], 0, sizeof(ft_device_context));

            /* Make local copy of device info from temporary list */
            memcpy(&ft_device[found_matching_devices].info, p, sizeof(FT_DEVICE_LIST_INFO_NODE));

            ++found_matching_devices;
        }
    }

    /* Error: Couldn't find any matching devices */
    if(!found_matching_devices)
    {
        return 0;
    }
   
    /* Clean up */
    if(device_info_list)
        free(device_info_list);

    return found_matching_devices;
}

int ft_close_device(int which)
{
    ft_device_context *p = &ft_device[which];

    /* Check if device is open */
    if(p->opened)
    {
        /* Purge any remaining data */
        p->status = FT_Purge(p->handle, FT_PURGE_RX | FT_PURGE_TX);
        ft_checkerr(p->status, "Error purging device.");
    
        /* Close device */
        p->status = FT_Close(p->handle);
        ft_checkerr(p->status, "Error closing device.");

        /* Mark device as closed */
        p->opened = 0;

        return 1;
    }
    else
    {
        printf("Error: Device was already closed.\n");
        return 0;
    }
}

int ft_open_device(int which)
{
    DWORD count;
    ft_device_context *p = &ft_device[which];

    /* Check if this device is already open */
    if(p->opened)
    {
        printf("Error: Device already opened.\n");
        return 0;    
    }

    /* Open device by serial number */
    p->status = FT_OpenEx(p->info.SerialNumber, FT_OPEN_BY_SERIAL_NUMBER, &p->handle);
    ft_checkerr(p->status, "Can't open device.");

    /* Reset device */
    p->status = FT_ResetDevice(p->handle);
    ft_checkerr(p->status, "Error resetting device.");

    /* Flush TX and RX queues */
    p->status = FT_Purge(p->handle, FT_PURGE_RX | FT_PURGE_TX);
    ft_checkerr(p->status, "Error purging device.");

    /* Get queue status */
    p->status = FT_GetQueueStatus(p->handle, &count);
    ft_checkerr(p->status, "Can't get queue status.");

    /* Set latency timer */
    p->status = FT_SetLatencyTimer(p->handle, 16);
    ft_checkerr(p->status, "Can't set latency timer.");

    /* Set MPSSE HBE mode */
    p->status = FT_SetBitMode(p->handle, 0x00, 0x08);
    ft_checkerr(p->status, "Can't set MPSSE Host Bus Emulation' bit mode.");

    /* Mark device as opened */
    p->opened = 1;

    return 1;
}



char *ft_getstatus(FT_STATUS status)
{
    switch(status)
    {
        case FT_OK:
            return "O.K.";
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
        case FT_EEPROM_ERASE_FAILED:
            return "EEPROM erase failed";
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
    return "Unknown status condition";
}

void ft_checkerr(FT_STATUS status, char *msg)
{
    if(status != FT_OK)
    {
        printf("Error:  %s\n", msg);
        printf("Status: %s\n", ft_getstatus(status));
        exit(0);
    }
}


