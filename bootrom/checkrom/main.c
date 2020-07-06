/*
    ROM checksum utility program.
*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>

typedef unsigned char uint8;
typedef unsigned long int uint32;

int main (int argc, char *argv[])
{
    char *filename = argv[1];
    int i;
    int size;
    int bread;
    uint8 *buf;
    uint32 acc;
    FILE *fd;

    // Print help if no argument specified
    if(argc < 2)
    {
        printf("usage: %s <file.bin>\n", argv[0]);
        return 0;
    }

    // Attempt to open input file
    fd = fopen(filename, "rb");
    if(!fd)
    {
        printf("Error opening file `%s' for reading.\n", filename);
        return 0;
    }

    // Determine file size
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    // Abort if file size is invalid (4 bytes data, 4 bytes sum minimum)
    if(size <= 8)
    {
        printf("Invalid file size (%d bytes).\n", size);
        fclose(fd);
        return 0;
    }

    // Attempt to allocate buffer for file data
    buf = malloc(size);
    if(!buf)
    {
        printf("Error allocating memory (%d bytes).\n", size);
        fclose(fd);
        return 0;
    }

    // Read file data into buffer
    memset(buf, 0, size);
    bread = fread(buf, size, 1, fd);
    fclose(fd);

    // Clear checksum accumulator
    acc = 0;

    // Checksum data (vector table)
    for(i = 0; i < 0x400; i += 4)
    {
        uint32 temp = 0;
        temp |= buf[i+0] << 24;
        temp |= buf[i+1] << 16;
        temp |= buf[i+2] <<  8;
        temp |= buf[i+3] <<  0;
        acc += temp;
    }

    // Continue to checksum data (remainder)
    for(i = 0x500; i < size; i += 4)
    {
        uint32 temp = 0;
        temp |= buf[i+0] << 24;
        temp |= buf[i+1] << 16;
        temp |= buf[i+2] <<  8;
        temp |= buf[i+3] <<  0;
        acc += temp;
    }

    // Insert checksum into parameter block
    buf[0x0400] = (size >> 24) & 0xFF;
    buf[0x0401] = (size >> 16) & 0xFF;
    buf[0x0402] = (size >>  8) & 0xFF;
    buf[0x0403] = (size >>  0) & 0xFF;

    buf[0x0404] = (acc >> 24) & 0xFF;
    buf[0x0405] = (acc >> 16) & 0xFF;
    buf[0x0406] = (acc >>  8) & 0xFF;
    buf[0x0407] = (acc >>  0) & 0xFF;


    // Attempt to open file 
    fd = fopen(filename, "wb");
    if(!fd)
    {
        printf("Error opening file `%s' for writing.\n", filename);
        free(buf);
        return 0;
    }

    // Write modified file data 
    fwrite(buf, size, 1, fd);
    fclose(fd);

    free(buf);

    printf("File checksum: %08X\n", acc);

    return 1;
}



