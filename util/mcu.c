/*
    mcu.c --
    MPSSE Host Bus Emulation interface code.
*/
#include "shared.h"

int mcu_flush(uint8 *buf)
{
    buf[0] = MCU_SEND_IMM;
    return 1;
}

int mcu_get_gpioh(int which)
{
    uint8 command[3];

    /* Make command packet */
    command[0] = MCU_GET_GPIOH;

    /* Send command */
    ft_send_data(which, command, 1);

    /* Get data */
    ft_get_data(which, command, 1);

    /* Return data bits */
    return (command[0] & GPIOH_DATA_MASK);
}

void mcu_set_gpioh(int which, int direction, int data)
{
    uint8 command[3];

    /* Make command packet */
    command[0] = MCU_SET_GPIOH;
    command[1] = (data & GPIOH_DATA_MASK);
    command[2] = (direction & GPIOH_DIR_MASK);

    /* Send command */
    ft_send_data(which, command, 3);
}


void mcu_send(int which, int address, uint8 *buffer, int size)
{
    int i;
    uint8 *cmd, *cmd_ptr;
    int cmd_len = (size * 4);

    /* Allocate command list */
    cmd_ptr = cmd = malloc(cmd_len);
    memset(cmd, 0, cmd_len);

    /* Make command packet list */
    for(i = 0; i < size; i++)
    {
        /* Calculate offset */
        int offset = (address + i) & 0xFFFF;

        /* Make command packets */
        *cmd_ptr++ = MCU_WRITE;
        *cmd_ptr++ = (offset >> 8) & 0xFF;
        *cmd_ptr++ = (offset >> 0) & 0xFF;
        *cmd_ptr++ = buffer[i];
    }

    /* Send command */
    ft_send_data(which, cmd, cmd_len);

    if(cmd)
        free(cmd);
}

void mcu_poke(int which, int address, int data)
{
    uint8 command[4];

    /* Make command packet */
    command[0] = MCU_WRITE;
    command[1] = (address >> 8) & 0xFF;
    command[2] = (address >> 0) & 0xFF;
    command[3] = data;

    /* Send command */
    ft_send_data(which, command, 4);
}

int mcu_peek(int which, int address)
{
    uint8 command[4];

    /* Make command packet */
    command[0] = MCU_READ;
    command[1] = (address >> 8) & 0xFF;
    command[2] = (address >> 0) & 0xFF;
    command[3] = MCU_SEND_IMM;

    /* Send command */
    ft_send_data(which, command, 4);

    /* Get data */
    ft_get_data(which, command, 1);

    return command[0];
}

/*--------------------------------------------------------------------------*/

/*
    Retrieving data from multiple MPSSE HBE read commands is complicated
    as only so much return data can accumulate before it needs to be
    read. To accomodate this, transfers are split up into blocks so
    that USB activity alternates between sending blocks of read commands
    and reading the command results until the full transfer takes place.

    Pitfalls:

    - If the amount of data sent by FT_Write() is less than the amount
      written, a partial command may be sent. This affects the # of bytes
      the subsequent reading code waits for and could cause a lockup.

      E.g. 6 bytes (3 commands) written, 5 bytes sent, so only one full
      command is processed. The return data will be 1 byte, not 2.
*/
void mcu_get(int which, int address, uint8 *buffer, int size)
{
    int i;
    ft_device_context *p = &ft_device[which];
    uint8 *cmd;
    uint8 *dst_ptr;
    uint8 *cmd_ptr;
    DWORD amount;
    DWORD transferred;
    int cmd_size = 3;

    int blk_size = (USB_BLOCK_SIZE * cmd_size);
    int cmd_rem  = (size * cmd_size);
    int data_rem = size;
    uint8 flush_cmd[1] = {MCU_SEND_IMM};

    /* Check parameters */
    if(size < 0) {
        printf("Error: Invalid transfer size (%d bytes).\n", size);
        return;
    }
        
    /* Wrap address past 64K */
    address &= 0xFFFF;

    /* Allocate command list */
    cmd_ptr = cmd = malloc(cmd_rem);
    if(!cmd)
    {
        printf("Error: Couldn't allocate %X bytes for command list.\n", cmd_rem);
        return;
    }
    dst_ptr = buffer;

    /* Fill command list */
    for(i = 0; i < size; i++)
    {
        /* Calculate offset */
        int offset = (address + i) & 0xFFFF;

        /* Make command packets */
        *cmd_ptr++ =  MCU_READ;
        *cmd_ptr++ = (offset >> 8) & 0xFF;
        *cmd_ptr++ = (offset >> 0) & 0xFF;
    }
   
    /* Loop until all command bytes sent */
    cmd_ptr = cmd;
    while(1)
    {
        /* Assign transfer length for this pass */
        amount = (cmd_rem >= blk_size) ? blk_size : cmd_rem;

        /* Write command data */
        p->status = FT_Write(p->handle, cmd_ptr, amount, &transferred);
        ft_checkerr(p->status, "mcu_get: Error writing command data.");

        /* Update offset in command list and remaining command list bytes */
        cmd_ptr += transferred;
        cmd_rem -= transferred;

        /* Send RX buffer flush command */
        p->status = FT_Write(p->handle, flush_cmd, 1, &amount);
        ft_checkerr(p->status, "mcu_get: Error writing flush command.");

        /* Wait until the above commands have been dispatched */
        while(ft_queue_status(which) < (transferred / cmd_size))
            Sleep(0);

        /* Read the results */
        amount = transferred / cmd_size;
        p->status = FT_Read(p->handle, dst_ptr, amount, &transferred);
        ft_checkerr(p->status, "mcu_get: Error reading result data.");

        /* Update progress of how much command data has been read */
        dst_ptr += transferred;
        data_rem -= transferred;

        /* Have all the commands been sent? */
        if(!cmd_rem)
            break;
    }

    /* Any remaining data that hasn't been read yet?
       This could potentially happen if FT_Read() read less bytes than
       were present in the queue prior to the call. */
    while(data_rem)
    {
        /* Wait until there's something in the queue */
        do {
            /* Send RX buffer flush command */
            p->status = FT_Write(p->handle, flush_cmd, 1, &amount);
            ft_checkerr(p->status, "mcu_get: Error writing flush command.");

            /* See if there any bytes to read */
            amount = ft_queue_status(which);

            if(!amount)
                Sleep(0);

        } while (!amount);

        /* Read the results */
        p->status = FT_Read(p->handle, dst_ptr, amount, &transferred);
        ft_checkerr(p->status, "mcu_get: Error reading result data.");

        /* Update progress of how much command data has been read */
        dst_ptr += transferred;
        data_rem -= transferred;
    }

    /* Check if there is anything left in the queue after the transfer
       and remainder check */
    if(ft_queue_status(which))
        printf("Error: queue not empty after mcu_get() complete.\n");

    /* Clean up */
    if(cmd)
        free(cmd);
}
