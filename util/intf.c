
#include "shared.h"

struct {
    uint8 rx_level;
    uint8 ixl_level;
    uint8 ixh_level;
    uint8 bxga_level[2];    /* Last BX and gate value set */
} intf_state;

int intf_open(void)
{
    int devno = -1;
    devno = acquire_device();
    if(devno == -1)
        return 0;
    ft_open_device(devno);
    return devno;
}

/*
    NOTE: Opens device #0 and displays info.
    Returns -1 if no devices are present.
*/
int acquire_device(void)
{
    int devno = -1;
    int num_devices;

    /* Scan for devices and report if none found */
    num_devices = ft_scan_devices(DEVICE_ID);

    if(!num_devices)
    {
        printf("Error: No devices attached.\n");
        return -1;
    }
    else
    {
        devno = 0;
        ft_device_context *p = &ft_device[devno];      
        printf("Using device #%d: `%s' (%s).\n",
            devno,
            p->info.Description,
            p->info.SerialNumber
            );
    }

    return devno;
}


void intf_close(int devno)
{
    intf_reset(devno);
    ft_close_device(devno);
}

void intf_reset(int devno)
{
    uint8 cmd[128];
    uint8 *p;
    int size;

    /* Clear interface state */
    memset(&intf_state, 0, sizeof(intf_state));

    /* Reset all latches and disable output drivers */
    p = cmd;
    p += set_rx(p, 0x00);
#ifndef REV2
    p += set_bx(p, 0x00, 0x00);
#endif
    p += set_ix(p, 0x0000);
    size = (int)p - (int)cmd;
    ft_send_data(devno, cmd, size);
}

int read_bx(uint8 *cmd)
{
    cmd[0] = MCU_READ;
    cmd[1] = REG_BX;
    cmd[2] = 0x00;

    return 3;
}

int read_bx_flush(uint8 *cmd)
{
    cmd[0] = MCU_READ;
    cmd[1] = REG_BX;
    cmd[2] = 0x00;
    cmd[3] = MCU_SEND_IMM;

    return 4;
}

int set_rx(uint8 *cmd, uint8 level)
{
    intf_state.rx_level = level;

    cmd[0] = MCU_WRITE;
    cmd[1] = REG_RX;
    cmd[2] = 0x00;
    cmd[3] = level;

    return 4;
}


int set_ix(uint8 *cmd, uint16 level)
{
    uint8 data;

    data = 0;
#ifdef REV2
    if(level & 0x0100) data |= 0x01;
    if(level & 0x0200) data |= 0x02;
#else
    if(level & 0x0100) data |= 0x02;
    if(level & 0x0200) data |= 0x01;
#endif
    intf_state.ixh_level = data;
    cmd[0] = MCU_WRITE;
    cmd[1] = REG_IXH;
    cmd[2] = 0x00;
    cmd[3] = data;

    data = 0;
#ifdef REV2
    if(level & 0x0001) data |= 0x01;
    if(level & 0x0002) data |= 0x02;
    if(level & 0x0004) data |= 0x04;
    if(level & 0x0008) data |= 0x08;
    if(level & 0x0010) data |= 0x10;
    if(level & 0x0020) data |= 0x20;
    if(level & 0x0040) data |= 0x40;
    if(level & 0x0080) data |= 0x80;
#else
    if(level & 0x0001) data |= 0x80;
    if(level & 0x0002) data |= 0x40;
    if(level & 0x0004) data |= 0x20;
    if(level & 0x0008) data |= 0x10;
    if(level & 0x0010) data |= 0x08;
    if(level & 0x0020) data |= 0x04;
    if(level & 0x0040) data |= 0x02;
    if(level & 0x0080) data |= 0x01;
#endif
    intf_state.ixl_level = data;
    cmd[4] = MCU_WRITE;
    cmd[5] = REG_IXL;
    cmd[6] = 0x00;
    cmd[7] = data;

    return 8;
}

int set_ixl(uint8 *cmd, uint16 level)
{
    uint8 data;

    data = 0;
#ifdef REV2
    if(level & 0x0001) data |= 0x01;
    if(level & 0x0002) data |= 0x02;
    if(level & 0x0004) data |= 0x04;
    if(level & 0x0008) data |= 0x08;
    if(level & 0x0010) data |= 0x10;
    if(level & 0x0020) data |= 0x20;
    if(level & 0x0040) data |= 0x40;
    if(level & 0x0080) data |= 0x80;
#else
    if(level & 0x0001) data |= 0x80;
    if(level & 0x0002) data |= 0x40;
    if(level & 0x0004) data |= 0x20;
    if(level & 0x0008) data |= 0x10;
    if(level & 0x0010) data |= 0x08;
    if(level & 0x0020) data |= 0x04;
    if(level & 0x0040) data |= 0x02;
    if(level & 0x0080) data |= 0x01;
#endif

    intf_state.ixl_level = data;
    cmd[0] = MCU_WRITE;
    cmd[1] = REG_IXL;
    cmd[2] = 0x00;
    cmd[3] = data;

    return 4;
}

int set_ixh(uint8 *cmd, uint16 level)
{
    uint8 data;

    data = 0;

#ifdef REV2
    if(level & 0x0100) data |= 0x01;
    if(level & 0x0200) data |= 0x02;
#else
    if(level & 0x0100) data |= 0x02;
    if(level & 0x0200) data |= 0x01;
#endif

    intf_state.ixh_level = data;
    cmd[0] = MCU_WRITE;
    cmd[1] = REG_IXH;
    cmd[2] = 0x00;
    cmd[3] = data;

    return 4;
}


#ifdef REV1

int set_bx(uint8 *cmd, uint8 level, uint8 enable)
{
    uint8 data;

    data = 0;
    if(enable & (1 << 0)) data |= 0x80;
    if(level  & (1 << 0)) data |= 0x40;
    if(enable & (1 << 2)) data |= 0x08;
    if(level  & (1 << 2)) data |= 0x04;
    if(enable & (1 << 5)) data |= 0x20;
    if(level  & (1 << 5)) data |= 0x10;
    if(enable & (1 << 7)) data |= 0x02;
    if(level  & (1 << 7)) data |= 0x01;
    intf_state.bxga_level[0] = data;
    cmd[0] = MCU_WRITE;
    cmd[1] = REG_BXGA1;
    cmd[2] = 0x00;
    cmd[3] = data;

    data = 0;
    if(enable & (1 << 1)) data |= 0x80;
    if(level  & (1 << 1)) data |= 0x40;
    if(enable & (1 << 3)) data |= 0x08;
    if(level  & (1 << 3)) data |= 0x04;
    if(enable & (1 << 4)) data |= 0x20;
    if(level  & (1 << 4)) data |= 0x10;
    if(enable & (1 << 6)) data |= 0x02;
    if(level  & (1 << 6)) data |= 0x01;
    intf_state.bxga_level[1] = data;
    cmd[4] = MCU_WRITE;
    cmd[5] = REG_BXGA2;
    cmd[6] = 0x00;
    cmd[7] = data;

    return 8;
}

#endif
