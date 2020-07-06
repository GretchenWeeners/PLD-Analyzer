/*
    Client program
*/

#include "shared.h"

#define CMD_NOP         0x00
#define CMD_ECHO        0x01
#define CMD_DOWNLOAD    0x02
#define CMD_UPLOAD      0x03
#define CMD_EXEC        0x04
#define CMD_RESET       0x05
#define CMD_CHECKSUM    0x06
#define CMD_FAULT       0x07

#define LOAD_ADDR       0x400000

int verbose = -1;
FILE *error_log = NULL;

static const option_t option_list[] = {
    {"-l", &opt_load},
    {"-c", &opt_dispatch},
    {"-x", &opt_exec},
    {"-d", &opt_download},
    {"-r", &opt_reset},
    {"-f", &opt_fault},
    {"-e", &opt_echo},
    {NULL, NULL},
};


int main (int argc, char *argv[])
{
    int i, j;

    /* Print help if insufficient arguments given */
    if(argc < 2)
    {
        printf("%s [-command]\n", argv[0]);
        printf("Commands are:\n");
        printf(" -x <file.bin> <addr>       \tRun program.\n");
        printf(" -c <file.bin> <addr>       \tRun program and process commands.\n");
        printf(" -l <file.bin> <addr>       \tLoad file to memory.\n");
        printf(" -d <file.bin> <addr> <size>\tSave memory to file.\n");
        printf(" -f                         \tTrigger double bus fault.\n");
        printf(" -r                         \tReset target.\n");
        printf(" -e                         \tEcho test.\n");
        return 1;
    }

    /* Set up error reporting */
    verbose = ERR_FATAL | ERR_WARN | ERR_INFO;
    error_log = stdout;

    /* Process commands */
    for(i = 1; i < argc; i++)
    {
        for(j = 0; option_list[j].key != NULL; j++)
        {
            if(stricmp(argv[i], option_list[j].key) == 0)
            {
                option_list[j].func(argc, argv);
            }
        }
    }

    return 0;
}

void die(char *format, ...)
{
    if(error_log && (verbose & ERR_FATAL))
    {
        va_list ap;
        va_start(ap, format);
        if(error_log) vfprintf(error_log, format, ap);
        va_end(ap);
    }

    exit(1);
}

void error(int level, char *format, ...)
{
    if(error_log && (verbose & level))
    {
        va_list ap;
        va_start(ap, format);
        if(error_log) vfprintf(error_log, format, ap);
        va_end(ap);
    }
}

/*--------------------------------------------------------------------------*/

void cmd_upload(uint8 *buf, uint32 address, uint32 size)
{
    int i;
    uint8 host_checksum;
    uint8 target_checksum;

    usb_sendb(CMD_UPLOAD);
    usb_sendl(address);
    usb_sendl(size);
    usb_send(buf, size);

    target_checksum = usb_getb();

    host_checksum = 0;
    for(i = 0; i < size; i++)
        host_checksum = (host_checksum + buf[i]) & 0xFF;
    usb_sendb(host_checksum);

    if(target_checksum != host_checksum)
    {
        printf("Error: Upload checksum mismatch (H:%02X,T:%02X)\n",
                host_checksum,
                target_checksum);
    }
}

void cmd_download(uint8 *buf, uint32 address, uint32 size)
{
    int i;
    uint8 host_checksum;
    uint8 target_checksum;

    usb_sendb(CMD_DOWNLOAD);
    usb_sendl(address);
    usb_sendl(size);
    usb_get(buf, size);

    target_checksum = usb_getb();

    host_checksum = 0;
    for(i = 0; i < size; i++)
        host_checksum = (host_checksum + buf[i]) & 0xFF;
    usb_sendb(host_checksum);

    if(target_checksum != host_checksum)
    {
        printf("Error: Download checksum mismatch (H:%02X,T:%02X)\n",
                host_checksum,
                target_checksum);
    }
}

void cmd_exec(uint32 address)
{
    usb_sendb(CMD_EXEC);
    usb_sendl(address);
}

void cmd_echo(void)
{
    uint8 tx, rx;
    int running = 1;

    usb_sendb(CMD_ECHO);

    printf("Echo test, press any key (ESC to quit).\n");
    while(running)
    {
        tx = getch();
        if(tx == 27)
            running = 0;
        printf("Key %02X, ", tx);
        usb_sendb(tx);
        printf("Send %02X, ", tx);
        rx = usb_getb();
        printf("Get %02X\n", rx);
    }
}

void cmd_reset(void)
{
    usb_sendb(CMD_RESET);
}

void cmd_fault(void)
{
    usb_sendb(CMD_FAULT);
}

uint32 cmd_checksum(void)
{
    usb_sendb(CMD_CHECKSUM);
    return usb_getl();
}

/*--------------------------------------------------------------------------*/


int opt_load(int argc, char *argv[])
{
    FILE *fd;
    uint8 *buffer;
    uint32 address;
    uint32 size;
    int left = argc - 2;

    /* Check parameters */
    if(left < 2)
        die("Insufficient arguments.\n");

    /* Convert parameters */
    address = strtoul(argv[3], NULL, 16);

    /* Range check */
    address &= 0x00FFFFFF;

    /* Open input file */
    fd = fopen(argv[2], "rb");
    if(!fd)
        die("Couldn't open file `%s' for reading.\n", argv[2]);

    /* Get file size */
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    /* Check file size */
    if(!size)
    {
        fclose(fd);
        die("Invalid input file size (%d bytes).\n", (int)size);
        return 0;
    }

    /* Allocate buffer */
    buffer = malloc(size);
    if(!buffer)
    {
        die("Error allocating %d bytes.\n", (int)size);
        fclose(fd);
        return 0;
    }

    /* Clear buffer */
    memset(buffer, 0, size);

    /* Read file data */
    fread(buffer, size, 1, fd);
    fclose(fd);

    /* Upload data */
    error(ERR_INFO, "Loading `%s' to $%06X-$%06X.\n", argv[2], (int)address, (int)(address + size - 1));
    usb_init();
    cmd_upload(buffer, address, size);
    usb_shutdown();

    /* Free buffer */
    free(buffer);

    return 1;
}


int opt_dispatch(int argc, char *argv[])
{
    FILE *fd;
    uint8 *buffer;
    uint32 address;
    uint32 size;
    int running = 1;
    int left = argc - 2;
    uint8 host_checksum;
    uint8 target_checksum;

    /* Check parameters */
    if(left < 2) {
        die("Insufficient arguments.\n");
        return 0;
    }

    /* Convert parameters */
    address = strtoul(argv[3], NULL, 16);

    /* Range check */
    address &= 0x00FFFFFE;

    /* Open input file */
    fd = fopen(argv[2], "rb");
    if(!fd)
    {
        die("Couldn't open file `%s' for reading.\n", argv[2]);
        return 0;
    }

    /* Get file size */
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    /* Check file size */
    if(!size)
    {
        die("Invalid input file size (%d bytes).\n", (int)size);
        fclose(fd);
        return 0;
    }

    /* Allocate buffer */
    buffer = malloc(size);
    if(!buffer)
    {
        die("Error allocating %d bytes.\n", (int)size);
        fclose(fd);
        return 0;
    }

    /* Clear buffer */
    memset(buffer, 0, size);

    /* Read file data */
    fread(buffer, size, 1, fd);
    fclose(fd);

    /* Upload data */
    error(ERR_INFO, "Loading `%s' to $%06X-$%06X.\n", argv[2], (int)address, (int)(address + size - 1));
    usb_init();
    cmd_upload(buffer, address, size);
    cmd_exec(address);

    /* Free buffer */
    free(buffer);

    /* Inform user how to manually exit dispatch mode */
    error(ERR_INFO, "Waiting for command...\n(Press Ctrl+C to exit)\n");

    fd = NULL;

    int accum = 0;
    int total = 0x800000;

    while(running)
    {
        int i;
        uint8 checksum, target_checksum;
        char ch;
        int addr;
        char str[1024];

        /* Get command byte */
        ch = usb_getb();

        /* Process command */
        switch(ch)
        {
            case 'o': /* Open file */
                fd = fopen("data.bin", "wb");
                error(ERR_INFO, "Opened file.\n");
                break;

            case 'w': /* Write data to file */
                size = usb_getl();

                accum += size;
                error(ERR_INFO, "Download %X/%X bytes.\r", accum, total);

                buffer = malloc(size);
                memset(buffer, 0, size);
                usb_get(buffer, size);

                target_checksum = usb_getb();
                host_checksum = 0;
                for(i = 0; i < size; i++)
                    host_checksum = (host_checksum + buffer[i]) & 0xFF;
                usb_sendb(host_checksum);

            
                if(target_checksum != host_checksum)
                {
                    printf("Error: Download checksum mismatch (H:%02X,T:%02X)\n",
                            host_checksum,
                            target_checksum);
                }

                if(fd)
                    fwrite(buffer, size, 1, fd);

                free(buffer);

                break;

            case 'q': /* Quit */
                if(fd) fclose(fd);
                error(ERR_INFO, "Quit.\n");
                running = 0;
                break;

            default:
                error(ERR_WARN, "Received unknown character `%c' (%02X)\n", ch, ch);
                break;
        }
    }

    usb_shutdown();
    return 1;
}



int opt_exec(int argc, char *argv[])
{
    FILE *fd;
    uint8 *buffer;
    uint32 address;
    uint32 size;
    int left = argc - 2;

    /* Check parameters */
    if(left < 2) {
        die("Insufficient arguments.\n");
        return 0;
    }

    /* Convert parameters */
    address = strtoul(argv[3], NULL, 16);

    /* Range check */
    address &= 0x00FFFFFE;

    /* Open input file */
    fd = fopen(argv[2], "rb");
    if(!fd)
    {
        die("Couldn't open file `%s' for reading.\n", argv[2]);
        return 0;
    }

    /* Get file size */
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    /* Check file size */
    if(!size)
    {
        die("Invalid input file size (%d bytes).\n", (int)size);
        fclose(fd);
        return 0;
    }

    /* Allocate buffer */
    buffer = malloc(size);
    if(!buffer)
    {
        die("Error allocating %d bytes.\n", (int)size);
        fclose(fd);
        return 0;
    }

    /* Clear buffer */
    memset(buffer, 0, size);

    /* Read file data */
    fread(buffer, size, 1, fd);
    fclose(fd);

    /* Upload data */
    error(ERR_INFO, "Loading `%s' to $%06X-$%06X.\n", argv[2], (int)address, (int)(address + size - 1));
    usb_init();
    cmd_upload(buffer, address, size);
    cmd_exec(address);
    usb_shutdown();

    /* Free buffer */
    free(buffer);

    return 1;
}


int opt_download(int argc, char *argv[])
{
    FILE *fd;
    uint8 *buffer;
    uint32 address;
    uint32 size;
    int left = argc - 2;

    /* Check parameters */
    if(left < 3) {
        die("Insufficient arguments.\n");
        return 0;
    }

    /* Convert parameters */
    address = strtoul(argv[3], NULL, 16);
    size    = strtoul(argv[4], NULL, 16);

    /* Range check */
    address &= 0x00FFFFFF;
    size    &= 0x00FFFFFF;

    /* Open input file */
    fd = fopen(argv[2], "wb");
    if(!fd)
    {
        die("Couldn't open file `%s' for writing.\n", argv[2]);
        return 0;
    }

    /* Allocate buffer */
    buffer = malloc(size);
    if(!buffer)
    {
        die("Error allocating %d bytes.\n", (int)size);
        fclose(fd);
        return 0;
    }

    /* Clear buffer */
    memset(buffer, 0, size);

    /* Download data */
    error(ERR_INFO, "Saving $%06X-$%06X to `%s'.\n", (int)address, (int)(address + size - 1), argv[2]);
    usb_init();
    cmd_download(buffer, address, size);
    usb_shutdown();

    /* Save to disk */
    fwrite(buffer, size, 1, fd);
    fclose(fd);

    /* Free buffer */
    free(buffer);

    return 1;
}

int opt_fault(int argc, char *argv[])
{
    usb_init();
    error(ERR_INFO, "Triggering double bus fault.\n");
    cmd_fault();
    usb_shutdown();
    return 1;
}

int opt_reset(int argc, char *argv[])
{
    usb_init();
    error(ERR_INFO, "Resetting target.\n");
    cmd_reset();
    usb_shutdown();
    return 1;
}

int opt_echo(int argc, char *argv[])
{
    usb_init();
    cmd_echo();
    usb_shutdown();
    return 1;
}






