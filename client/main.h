
#ifndef _MAIN_H_
#define _MAIN_H_

#define ERR_FATAL   1
#define ERR_WARN    2
#define ERR_INFO    4

typedef struct {
    char *key;
    int (*func)(int argc, char *argv[]);
} option_t;

/* Function prototypes */
void error(int level, char *format, ...);
void die(char *format, ...);
int opt_load(int argc, char *argv[]);
int opt_dispatch(int argc, char *argv[]);
int opt_exec(int argc, char *argv[]);
int opt_download(int argc, char *argv[]);
int opt_checksum(int argc, char *argv[]);
int opt_fault(int argc, char *argv[]);
int opt_reset(int argc, char *argv[]);
int opt_echo(int argc, char *argv[]);;

#endif /* _MAIN_H_ */
