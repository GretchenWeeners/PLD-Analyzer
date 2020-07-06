/*
    PLD data analysis program
    (C) 2007-2008 Charles MacDonald
    WWW: http://cgfm2.emuviews.com
*/
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "shared.h"

#define MAX_EQU             (256*1024)
#define ALWAYS_LO           (1 << 0)    // Pin is always outputting LO
#define ALWAYS_HI           (1 << 1)    // Pin is always outputting HI
#define ALWAYS_HIZ          (1 << 2)    // Pin is always tristated
#define HAS_HIZ             (1 << 3)    // 1= Has been tristated, 0= hasn't
#define HAS_OE              (1 << 4)    // 1= Has been outputting, 0= hasn't

typedef struct {
    int attr;
    int iterms_used;
    int polarity;
    int *equ_list;
    int equ_count;
    int oe_iterms_used;
    int oe_polarity;
    int *oe_equ_list;
    int oe_equ_count;
} macrocell;

static const int pin_number[] = {
    1,2,3,4,5,6,7,8,9,11,12,
    13,14,15,16,17,18,19
};

static const char *signal_name[] = {
    "I0", "I1", "I2", "I3", "I4", "I5", "I6", "I7", "I8", "I9",
    "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7"
};

macrocell mc[8];
uint8 data[MAX_EQU];
uint8 hiz_data[MAX_EQU];
int iterms_ever_used;
int siglen;

/* Function prototypes */
char *pin_name(int which);
char *bx_name(int which);
void analyze_pal(char *filename);
static const struct {
    char *str;
    int (*func)(char *argv[], int argc, int argi, int left);
    char *use;
    char *help;
} opt_list[] = {
    {"dump"    , dump_func,    "<file.bin>", "Dump PLD to data file."},
    {"analyze" , analyze_func, "<file.bin>", "Analyze PLD dump."},
    {NULL      , NULL },
};

void pld_read(int devno, char *path);

int analyze_func(char *argv[], int argc, int argi, int left)
{
    if(argc < 3)
    {
        printf("Specify a filename.\n");
        return 0;
    }

    analyze_pal(argv[2]);

    return 0;
}

int dump_func(char *argv[], int argc, int argi, int left)
{
    int devno;

    if(argc < 3)
    {
        printf("Specify a filename.\n");
        return 0;
    }

    devno = intf_open();
    intf_reset(devno);
    pld_read(devno, argv[2]);
    intf_close(devno);

    return 0;
}













int main (int argc, char *argv[])
{
    int i;    
    int argi;

    /* Print help */
    if(argc < 2)
    {
        print_help(argc, argv);
        return 0;
    }

    /* Check options */
    for(argi = 1; argi < argc; argi++)
    {
        int left = argc - 1;
        int found = 0;

        /* Search for a matching command */
        for(i = 0; opt_list[i].str != NULL && !found; i++)
        {
            /* Run matching function for this command */
            if(stricmp(argv[argi], opt_list[i].str) == 0)
            {
                opt_list[i].func(argv, argc, argi, left);
                return 1;
                found = 1;
            }
        }

        if(!found)
            printf("Unknown option `%s'.\n", argv[argi]);

        if(argi == 1)
            break;
    }

    return 1;
}


void print_help(int argc, char **argv)
{
    int device_count;
    int i, len, maxlen;
    char buf[128];
    char fmt[128];

    /* Print help text */
    printf("PAL dumping and analysis utility\n");
    printf("(C) 2007-2008 Charles MacDonald\n");
    printf("usage: %s <option> <arguments>\n", argv[0]);
    printf("\n");
    printf("Available options:\n");

    /* Determine maximum length of command string */
    maxlen = 1;
    for(i = 0; opt_list[i].func != NULL; i++)
    {
        len = strlen(opt_list[i].str);
        if(len > maxlen) maxlen = len;
    }

    /* Print options */
    for(i = 0; opt_list[i].func != NULL; i++)
    {
        sprintf(fmt, " %%-%ds %%s", maxlen);
        sprintf(buf, fmt, opt_list[i].str, opt_list[i].use);
        printf("%-26s : %s\n", buf, opt_list[i].help);
    }

    /* Scan for devices */
    device_count = ft_scan_devices(DEVICE_ID);

    /* Display device information list */
    if(!device_count)
        printf("\n No devices present.\n");
    else       
    {
        printf("\nAvailable devices:\n");
        for(i = 0; i < device_count; i++)
        {
            ft_device_context *p = &ft_device[i];      
            printf(" - Device #%d: `%s' (%s)\n",
                i,
                p->info.Description,
                p->info.SerialNumber);
        }
    }
}



#define STATE_SIZE      (256*1024)
#define CMD_SIZE        (4+(1024*(8+3))+1)

void pld_read(int devno, char *path)
{
    int rx, ix, size;
    uint8 state[STATE_SIZE];
    uint8 cmd[CMD_SIZE];
    uint8 *ptr;
    FILE *fd;

    printf("Reading PLD. Press Ctrl+C to abort.\n");

    /* Reset BX latches; disable BX output drivers */

    /* Loop over all possible combinations of RX settings */
    for(rx = 0; rx < 256; rx++)
    {
        /* Display progress indicator */
        fprintf(stderr, "Progress: %.1f%%\r", (100.0f / 255.0f) * (double)rx);

        /* Make command packet */
        ptr = cmd;

        /* Set pull-up/down resistor state */
        ptr += set_rx(cmd, rx);

        /* Loop over all possible combinations of IX settings */
        for(ix = 0; ix < 1024; ix++)
        {
            ptr += set_ix(ptr, ix);
            ptr += read_bx(ptr);
        }

        /* Add flush command */
        ptr += mcu_flush(ptr);

        /* Send command packet and get results */
        size = (uint32)ptr - (uint32)cmd;
        ft_send_data(devno, cmd, size);
        ft_get_data(devno, &state[rx*1024], 1024);
    }

    printf("Finished reading PLD.\t\t\n");

    /* Save state to disk */
    fd = fopen(path, "wb");
    if(!fd) {
        printf("Error: could not write to `%s'.\n", path);
        return;
    }
    fwrite(state, sizeof(state), 1, fd);
    fclose(fd);

    printf("Data saved to `%s'.\n", path);
}



void analyze_pal(char *filename)
{
    int i, j;
    int bx, ix;
    FILE *fd;

    /* Load dumped data */
    fd = fopen(filename, "rb");
    if(!fd)
    {
        printf("File `%s' not found.\n", filename);
        return;
    }
    fread(data, MAX_EQU, 1, fd);
    fclose(fd);

    /* Determine maximum pin name length */
    siglen = 0;
    for(i = 0; i < 18; i++)
    {
        if(strlen(signal_name[i]) > siglen)
            siglen = strlen(signal_name[i]);
    }

    /* Clear state */
    memset(&mc, 0, sizeof(mc));
    memset(hiz_data, 0, sizeof(hiz_data));
    iterms_ever_used = 0;

    /************************************************************************/
    /* Phase 1: Determine pin attributes                                    */
    /************************************************************************/

    for(bx = 7; bx >= 0; bx--)
    {
        int attr = (ALWAYS_LO | ALWAYS_HI | ALWAYS_HIZ);

        for(ix = 0; ix < MAX_EQU; ix++)
        {
            /* Get pin state when resistor is pulling low, and high */
            int state    = (data[ix] >> bx) & 1;
            int state_lo = (data[(ix & ~(1 << (bx+10)))] >> bx) & 1;
            int state_hi = (data[(ix |  (1 << (bx+10)))] >> bx) & 1;

            if(state_lo == 0 && state_hi == 1)
            {
                /* Pin follows resistor, it is tristated */
                attr |= HAS_HIZ;

                /* Flag this BX pin as being tristated for this set of inputs */
                hiz_data[ix] |= (1 << bx);
            }          
            else
            {
                /* Pin is being driven */
                attr |= HAS_OE;

                /* It can't always be tristated */
                attr &= ~ALWAYS_HIZ;

                /* If pin is low it can't always be high */
                if(state == 0)
                    attr &= ~ALWAYS_HI;

                /* If pin is high it can't always be low */
                if(state == 1)
                    attr &= ~ALWAYS_LO;
            }
        }

        /* If the pin is an input, we know it can't be stuck low or high */
        if(attr & ALWAYS_HIZ)
            attr &= ~(ALWAYS_LO | ALWAYS_HI);

        /* Assign pin attributes */
        mc[bx].attr = attr;
    }

    /************************************************************************/
    /* Phase 2: Determine iterms used to define output level                */
    /************************************************************************/

    for(bx = 7; bx >= 0; bx--)
    {
        int iterm;
        int iterms_used = 0;

        /* Skip BX pins that aren't outputs */
        if(!(mc[bx].attr & HAS_OE))
            continue;

        for(ix = 0; ix < MAX_EQU; ix++)
        {
            for(iterm = 0; iterm < 18; iterm++)
            {
                /* Skip BX pins that have no input capability */
                if(iterm >= 10 && !(mc[iterm-10].attr & HAS_HIZ))
                    continue;

                /* Skip "this" pin */
                if(iterm >= 10 && bx == (iterm-10))
                    continue;

                uint8 state_lo = (data[ix & ~(1 << iterm)] >> bx) & 1;
                uint8 state_hi = (data[ix |  (1 << iterm)] >> bx) & 1;
                uint8 hiz_lo = (hiz_data[ix & ~(1 << iterm)] >> bx) & 1;
                uint8 hiz_hi = (hiz_data[ix |  (1 << iterm)] >> bx) & 1;

                /* If this iterm changes the output state, but NOT so
                   that it tristates, this pin is dependant on it */
                if(state_lo != state_hi && hiz_lo != 1 && hiz_hi != 1)
                    iterms_used |= 1 << iterm;
            }
        }

        /* Assign pin attributes */
        mc[bx].iterms_used = iterms_used;
        iterms_ever_used |= iterms_used;
    }

    /************************************************************************/
    /* Phase 3: Determine iterms used by output enable                      */
    /************************************************************************/

    for(bx = 7; bx >= 0; bx--)
    {
        int iterm;
        int iterms_used = 0;

        /* Skip BX pins that are permanently tristated */
        if(mc[bx].attr & ALWAYS_HIZ)
            continue;

        /* Skip BX pins that are never tristated */
        if(!(mc[bx].attr & HAS_HIZ))
            continue;

        for(ix = 0; ix < MAX_EQU; ix++)
        {
            for(iterm = 0; iterm < 18; iterm++)
            {
                /* Skip BX pins that have no input capability */
                if(iterm >= 10 && !(mc[iterm-10].attr & HAS_HIZ))
                    continue;

                /* Skip "this" pin */
                if(iterm >= 10 && bx == (iterm-10))
                    continue;

                uint8 state_lo = (hiz_data[ix & ~(1 << iterm)] >> bx) & 1;
                uint8 state_hi = (hiz_data[ix |  (1 << iterm)] >> bx) & 1;
            
                /* This input term changed the output, we depend on it */
                if(state_lo != state_hi)
                    iterms_used |= 1 << iterm;
            }
        }

        mc[bx].oe_iterms_used = iterms_used;
        iterms_ever_used |= iterms_used;
    }


    /************************************************************************/
    /* Determine device configuration                                       */
    /************************************************************************/

    printf("/*\n");

    int goe_err = 0;
    int any_hiz = 0;

    /* If any output term uses I9, the device is not registered */
    for(bx = 7; bx >= 0; bx--)
    {
        if(mc[bx].attr & HAS_HIZ)
            any_hiz = 1;
        if(mc[bx].iterms_used & (1 << 9))
            goe_err = 1;
    }

    if(!goe_err)
    {
        /* I9 isn't used as an input term. For the device to be registered,
           at least one output HAS to use I9 as a global /OE */
        for(bx = 7; bx >= 0; bx--)
        {
            if(mc[bx].oe_iterms_used == (1 << 9))
                goe_err |= (1 << bx);
        }

        if(goe_err)
        {
            /* At least one term uses I9 exclusively for it's global /OE
               Verify the polarity matches */
            for(bx = 7; bx >= 0; bx--)
            {
                for(ix = 0; ix < MAX_EQU; ix++)
                {
                    int hiz_state = (hiz_data[ix] >> bx) & 1;
                    int i9_state = (ix >> 9) & 1;
        
                    /* If the tristate status of a pin does not follow I9
                       where I9=L (driven), I9=H (tristate), then I9 is not
                       functioning as a global output enable for *this* pin */
                    if(i9_state != hiz_state)
                        goe_err &= ~(1 << bx);
                }
            }

            if(goe_err)
            {
                printf("\tWARNING: The following pins MAY be registered:\n");
                for(bx = 7; bx >= 0; bx--)
                {
                    if(goe_err & (1 << bx))
                        printf("\t\tPin %d (%s)\n", pin_number[10+bx], bx_name(bx));
                }
                printf("\n");
                printf("\tAnalysis results WILL NOT be correct for registered devices.\n\n");
            }
        }
    }

    printf("\tData source:\n\t\t%s\n\n", filename);
    printf("\tDevice configuration:\n\t\t");

    if(any_hiz)
    {
        if( (mc[0].attr & HAS_OE) && (mc[7].attr & HAS_OE) && (iterms_ever_used & (1<<10|1<<17))==0)
            printf("%s", "complex");
        else                
            printf("%s", "unknown");
    }
    else    
            printf("%s", "simple");


    printf("\n\n");

    printf("\tSuggested GAL16V8 device type for WinCUPL:\n\t\t");

    if(any_hiz)
    {
        if( (mc[0].attr & HAS_OE) && (mc[7].attr & HAS_OE) && (iterms_ever_used & (1<<10|1<<17))==0)
            printf("%s\n", "g16v8ma");
        else                
            printf("%s\n", "g16v8");
    }
    else    
            printf("%s\n", "g16v8as");

    printf("\n\tSummary of bidirectional pins:\n\n");

    /* Report pin attributes */
    for(bx = 7; bx >= 0; bx--)
    {
        printf("\t");
        printf("Pin %d (%s)    : ", pin_number[10+bx], bx_name(bx));
        switch(mc[bx].attr)
        {
            case ALWAYS_LO | HAS_OE | HAS_HIZ:
                printf("Output w/ enable, fixed LO.\n");
                break;
    
            case ALWAYS_HI | HAS_OE | HAS_HIZ:
                printf("Output w/ enable, fixed HI.\n");
                break;
    
            case ALWAYS_LO | HAS_OE:
                printf("Output, fixed LO.\n");
                break;
    
            case ALWAYS_HI | HAS_OE:
                printf("Output, fixed HI.\n");
                break;
    
            case ALWAYS_HIZ | HAS_HIZ:
                printf("Input.\n");
                break;
    
            case HAS_HIZ | HAS_OE:
                printf("Output with enable.\n");
                break;
    
            case HAS_OE:
                printf("Output.\n");
                break;
    
            default:
                printf("Unknown attributes (%02X)\n", mc[bx].attr);
                break;
        }
    }

    printf("*/\n");
    printf("\n");

///--------------------------------------------------------------------------

    char name[128];
    strcpy(name, filename);
    strcpy(strrchr(name, '.'), "\0");

    printf("Name     %s;\n", name);
    printf("PartNo   ;\n");
    printf("Date     ;\n");
    printf("Revision ;\n");
    printf("Designer ;\n");
    printf("Company  ;\n");
    printf("Assembly ;\n");
    printf("Location ;\n");
    printf("Device   virtual;\n");


///--------------------------------------------------------------------------

    printf("\n/* Dedicated input pins */\n\n");
    for(i = 0; i < 10; i++)
    {
        printf("pin %d\t=\t%s;\t", pin_number[i], pin_name(i));
        if(!(iterms_ever_used & (1 << i)))
        {
            printf(" /* Unused input */");
        }
        printf("\n");
    }

    if(any_hiz)
    {
        printf("\n/* Input and/or bidirectional pins */\n\n");
        for(i = 0; i < 8; i++)
        {
            if(mc[i].attr & HAS_HIZ)
            {
                printf("pin %d\t=\t%s;\t", pin_number[10+i], bx_name(i));
    
                if(!(iterms_ever_used & (1 << (10+i))))
                {
                    if(mc[i].attr & ALWAYS_HIZ)
                        printf(" /* Unused input */");
                    else
                        printf(" /* Not used as an input when tristated */");
                }
    
                printf("\n");
            }
        }
    }

    int any_out = 0;
    for(i=0;i<8;i++)
        if(!(mc[i].attr & HAS_HIZ))
            any_out = 1;

    if(any_out)
    {
        printf("\n/* Dedicated output pins */\n\n");
        for(i = 0; i < 8; i++)
        {
            if(!(mc[i].attr & HAS_HIZ))
            {
                printf("pin %d\t=\t%s;\t", pin_number[10+i], bx_name(i));
                printf("\n");
            }
        }
    }
    printf("\n/* Output and output enable equations */\n");

///--------------------------------------------------------------------------

    uint8 list[2][MAX_EQU];
    int polarity;
    int which;
    int count[2];

    /************************************************************************/
    /* Phase 4: Determine # of equations based on polarity                  */
    /************************************************************************/

    for(bx = 7; bx >= 0; bx--)
    {
        if(!(mc[bx].attr & HAS_OE))
            continue;
        if(mc[bx].attr & (ALWAYS_LO | ALWAYS_HI))
            continue;

        memset(list, 0, sizeof(list));

        for(polarity = 0; polarity < 2; polarity++)
        {
            count[polarity] = 0;

            for(ix = 0; ix < MAX_EQU; ix++)
            {
                int lvl_state = (data[ix] >> bx) & 1;
                int hiz_state = (hiz_data[ix] >> bx) & 1;

                if(hiz_state == 0 && lvl_state == polarity)
                    list[polarity][ix & mc[bx].iterms_used] |= 1;
            }

            for(ix = 0; ix < MAX_EQU; ix++)
            {
                if(list[polarity][ix])
                      ++count[polarity];
            }
        }

        which = 1;
        if(count[0] && count[0] < count[1])
            which = 0;

        mc[bx].polarity = which;
        mc[bx].equ_count = count[which];
        mc[bx].equ_list = malloc(count[which] * sizeof(uint32));
    }

    /************************************************************************/
    /* Phase 5: Print equation lists for non-fixed output pins              */
    /************************************************************************/

    for(bx = 7; bx >= 0; bx--)
    {
        if(!(mc[bx].attr & HAS_OE))
            continue;

        if(mc[bx].attr & ALWAYS_LO)
            printf("\n%c%s\t= 'b'0;", mc[bx].polarity ? ' ' : '!', bx_name(bx));
        if(mc[bx].attr & ALWAYS_HI)
            printf("\n%c%s\t= 'b'1;", mc[bx].polarity ? ' ' : '!', bx_name(bx));

        if(mc[bx].attr & (ALWAYS_LO | ALWAYS_HI))
            continue;

        memset(list, 0, sizeof(list));

        for(ix = 0; ix < MAX_EQU; ix++)
        {
            int lvl_state = (data[ix] >> bx) & 1;
            int hiz_state = (hiz_data[ix] >> bx) & 1;

            if(hiz_state == 0 && lvl_state == mc[bx].polarity)
                list[0][ix & mc[bx].iterms_used] |= 1;
        }

        int total = 0;

        printf("\n%c%s\t= ", mc[bx].polarity ? ' ' : '!', bx_name(bx));
        for(i = 0; i < MAX_EQU; i++)
        {
            if(list[0][i])
            {
                int last = 0;

                for(j = 0; j < 18; j++)
                    if(mc[bx].iterms_used & (1 << j))
                        last = j;
                    
                for(j = 0; j < 18; j++)
                {
                    if(mc[bx].iterms_used & (1 << j))
                    {
                        int iterm = (i >> j) & 1;
                        printf("%c%s", iterm ? ' ' : '!', pin_name(j));
                        if(j != last) printf(" & ");
                    }
                }

                if(total != mc[bx].equ_count-1)
                    printf("\n\t# ");
                else
                    printf(";");
                ++total;
            }
        }
    }

    printf("\n");

    ////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////

    /************************************************************************/
    /* Phase 6: Determine # of equations based on default oe                */
    /************************************************************************/
    for(bx = 7; bx >= 0; bx--)
    {
        if(!(mc[bx].attr & HAS_HIZ))
            continue;
        if(mc[bx].attr & ALWAYS_HIZ)
            continue;

        memset(list, 0, sizeof(list));

        for(polarity = 0; polarity < 2; polarity++)
        {
            count[polarity] = 0;

            for(ix = 0; ix < MAX_EQU; ix++)
            {
                int hiz_state = (hiz_data[ix] >> bx) & 1;

                if(hiz_state == polarity)
                    list[polarity][ix & mc[bx].oe_iterms_used] |= 1;
            }

            for(ix = 0; ix < MAX_EQU; ix++)
            {
                if(list[polarity][ix])
                    ++count[polarity];
            }
        }

        which = 1;
        if(count[0] && count[0] < count[1]) which = 0;

        mc[bx].oe_polarity = which;
        mc[bx].oe_equ_count = count[which];
        mc[bx].oe_equ_list = malloc(count[which] * sizeof(uint32));
    }

    /************************************************************************/
    /* Phase 7: Print equation lists for non-fixed output pins              */
    /************************************************************************/

    for(bx = 7; bx >= 0; bx--)
    {
        if(!(mc[bx].attr & HAS_HIZ))
            continue;
        if(mc[bx].attr & ALWAYS_HIZ)
            continue;

        memset(list, 0, sizeof(list));

        for(ix = 0; ix < MAX_EQU; ix++)
        {
            int hiz_state = (hiz_data[ix] >> bx) & 1;

            if(hiz_state == mc[bx].oe_polarity)
                list[0][ix & mc[bx].oe_iterms_used] |= 1;
        }

        int total = 0;

        printf("%c%s.oe\t= ", mc[bx].oe_polarity ? ' ' : '!', bx_name(bx));
        for(i = 0; i < MAX_EQU; i++)
        {
            if(list[0][i])
            {
                int last = 0;

                for(j = 0; j < 18; j++)
                    if(mc[bx].oe_iterms_used & (1 << j))
                        last = j;
                    
                for(j = 0; j < 18; j++)
                {
                    if(mc[bx].oe_iterms_used & (1 << j))
                    {
                        int iterm = (i >> j) & 1;
                        printf("%c%s", iterm ? ' ' : '!', pin_name(j));
                        if(j != last) printf(" & ");
                    }
                }

                if(total != mc[bx].oe_equ_count-1)
                    printf("\n\t# ");
                else
                    printf(";");
                ++total;
            }
        }
        if(bx) printf("\n");
    }

    printf("\n\n/* End */\n");
}


char *pin_name(int which)
{
    char fmt[32], str[32];
    sprintf(fmt, "%%s");
    sprintf(str, fmt, signal_name[which]);
    return strdup(str);
}


char *bx_name(int which)
{
    return pin_name(10+which);
}



