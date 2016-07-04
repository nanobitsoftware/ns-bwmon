/*
 * Nanobit Software (http://www.nanobit.net/) 
 *    Michael Hayes (Bioteq) 2002-2017 
 *
 * NS:BWMon - Monitor current, real time, bandwidth of a linux
 * based system. This will read the /proc/net/dev file and parse out the
 * specified network adapter, reading in the bandwidth being used. 
 *
 * It allows for 1000 and 1024 conversion, depending on your choice of 1000
 * bits per KB or 1024 bits per KB. 
 *
 * This program was written back in 2002 for a job I had with a 
 * web hosting company. It is not extremly well written, but it works 
 * very well for what it is desigend for. 
 *
 * I have not tested this on BSD style systems, but if someone does 
 * try it and/or does work to make it work on BSD systems, please 
 * contact me and I would love to give you credit for your work. 
 *
 * This software is free to use, as is its source code. 
 * I only ask that you give me credit where credit is due and you 
 * do not claim full responsibility for this software.
 *
 * Please send me an email to mike@nanobit.net and let me know you're
 * using it. Thanks!
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

float bsent;
float brec;
static unsigned long int bsentrem;
static unsigned long int brecrem;
float _brec = 0.00;
float _bsent = 0.00;
int fetchline(char s[], int lim, FILE * fp);

void print_cmd_error(void)
{
    printf("Usage: bwmon <adapter> <1000 or 1024 (for mb/kb accounting)>\r\n");
    return;
}

int main(int argc, char *argv[])
{
    struct tm che;
    time_t thetime;
    long int old_time;
    FILE *fp;
    thetime = time(NULL);
    float fsent, frecv;
    char type[2];
    char rtype[2];
    char adapt[1024];
    int mult;
    int i;
    char file[1024];
    localtime_r(&thetime, &che);
    old_time = 100;

    fsent = frecv = 0.0f;
    type[0] = rtype[0] = file[0] = adapt[0] = '\0';
    mult = 1024;
    if (argc <= 1)
        print_cmd_error();

    for (i = 0; i < argc; i++) {
        printf("Arguments: %d) %s\r\n", i, argv[i]);
    }

    if (argv[1])
        sprintf(adapt, "%s", argv[1]);
    if (argc > 2 && argv[2])
        mult = atoi(argv[2]);

    if (mult <= 0)
        mult = 0;
    if (mult != 1024 && mult != 1000) {
        printf("Second argument must be 1000 or 1024.\r\n");
        return;
    }

    printf("Beginning.\n\r\n\r");

    while (old_time > -10) {
        // thetime = time(NULL);
        localtime_r(&thetime, &che);
        // if (old_time == che.tm_sec)
        // continue;
        sleep(1);
        old_time = che.tm_sec;
        // if (fp != NULL)
        // parse_bytes(fp);

        if ((fp = fopen("/proc/net/dev", "r")) == NULL) {
            printf("Unable to open proc file!\n\n");
            return 0;
        }

        while (!feof(fp)) {

            unsigned char bline[500];

            fetchline(bline, 1000, fp);
            if (strstr(bline, adapt[0] == '\0' ? "eth0:" : adapt)) {
                char deli[] = " ";
                char *token, *buf;

                buf = strdup(&bline[7]);

                token = strtok(buf, deli);      // bytes (rec)
                brec = strtof(token, NULL);

                token = strtok(NULL, deli);     // packets
                token = strtok(NULL, deli);     // errs
                token = strtok(NULL, deli);     // drop
                token = strtok(NULL, deli);     // fifo
                token = strtok(NULL, deli);     // frame
                token = strtok(NULL, deli);     // compressed
                token = strtok(NULL, deli);     // multicast
                token = strtok(NULL, deli);     // bytes (send)
                bsent = strtof(token, NULL);
                token = strtok(NULL, deli);     // packets
                token = strtok(NULL, deli);     // errs
                token = strtok(NULL, deli);     // drop
                token = strtok(NULL, deli);     // fifo
                token = strtok(NULL, deli);     // frame
                token = strtok(NULL, deli);     // compressed
                token = strtok(NULL, deli);     // multicast

                _brec = brec - _brec;
                _bsent = bsent - _bsent;

                fsent = (float)_bsent;
                frecv = (float)_brec;

                type[0] = 'b';
                type[1] = '\0';
                rtype[0] = 'b';
                rtype[1] = '\0';

                if (fsent > mult) {
                    fsent = fsent / mult;
                    type[0] = 'K';
                    type[1] = '\0';
                }
                if (fsent > mult) {
                    fsent = fsent / mult;
                    type[0] = 'M';
                    type[1] = '\0';
                }
                if (fsent > mult) {

                    fsent = fsent / mult;
                    type[0] = 'G';
                    type[1] = '\0';
                }

                if (frecv > mult) {

                    frecv = frecv / mult;
                    rtype[0] = 'K';
                    rtype[1] = '\0';
                }

                if (frecv > mult) {

                    frecv = frecv / mult;
                    rtype[0] = 'M';
                    rtype[1] = '\0';
                }

                if (frecv > mult) {

                    frecv = frecv / mult;
                    rtype[0] = 'G';
                    rtype[1] = '\0';
                }

                printf("\e[1A\e[2K");

                printf("Send: %.3f%s/s - Rec: %.3f%s/s\n", fsent, type, frecv, rtype);

                _bsent = bsent;
                _brec = brec;
                bsentrem = 0;
                brecrem = 0;

            }

        }
        fclose(fp);

    }

    return 0;
}

int fetchline(char s[], int lim, FILE * fp)
{
    int c, i, j;
    for (i = 0, j = 0; (c = getc(fp)) != EOF && c != '\n'; ++i) {
        if (i < lim - 1) {
            s[j++] = c;
        }
    }
    if (c == '\n') {
        if (i <= lim - 1) {
            s[j++] = c;
        }
        ++i;
    }
    s[j] = '\0';
    return i;
}
