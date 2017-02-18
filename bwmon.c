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
int fetchline (char s[], int lim, FILE * fp);
void sleep (int x);
void
print_cmd_error (void)
{
  printf ("Usage: bwmon <adapter> <1000 or 1024 (for mb/kb accounting)>\r\n");
  return;
}

int
main (int argc, char *argv[])
{
  struct tm che;
  time_t thetime;
  long int old_time;
  FILE *fp;
  char adapt[1024];
  char file[1024];
  char type[2];
  char rtype[2];
  float fsent, frecv;
  int mult;
  int i;
  int skip = 0;			//For skipping the tokens below.

  thetime = time (NULL);

  localtime_r (&thetime, &che);
  old_time = 100;

  fsent = frecv = 0.0f;
  type[0] = rtype[0] = file[0] = adapt[0] = '\0';
  mult = 1024;
  if (argc <= 1)
    print_cmd_error ();

  for (i = 0; i < argc; i++)
    {
      printf ("Arguments: %d) %s\r\n", i, argv[i]);
    }

  if (argv[1])
    sprintf (adapt, "%s", argv[1]);
  if (argc > 2 && argv[2])
    mult = atoi (argv[2]);

  if (mult <= 0)
    mult = 0;
  if (mult != 1024 && mult != 1000)
    {
      printf ("Second argument must be 1000 or 1024.\r\n");
      return 0;
    }

  printf ("Beginning.\n\r\n\r");

  while (old_time > -10)
    {
      localtime_r (&thetime, &che);

      sleep (1);		// Sleep for a bit before updating.

      old_time = che.tm_sec;

      if ((fp = fopen ("/proc/net/dev", "r")) == NULL)
	{
	  printf ("Unable to open proc file!\n\n");
	  return 0;
	}

      while (!feof (fp))
	{

	  unsigned char bline[500];

	  fetchline (bline, 1000, fp);
	  if (strstr (bline, adapt[0] == '\0' ? "eth0:" : adapt))
	    {
	      char deli[] = " ";
	      char *token, *buf;

	      buf = strdup (&bline[7]);

	      token = strtok (buf, deli);	// bytes (rec)
	      brec = strtof (token, NULL);

	      for (skip = 0; skip < 8; skip++)
		token = strtok (NULL, deli);	// Skip 7 tokens

	      bsent = strtof (token, NULL);

	      for (skip = 0; skip < 6; skip++)
		token = strtok (NULL, deli);	// Skip 6 tokens.

	      _brec = brec - _brec;
	      _bsent = bsent - _bsent;

	      fsent = (float) _bsent;
	      frecv = (float) _brec;

	      type[0] = rtype[0] = 'b';
	      type[1] = rtype[1] = '\0';
	      /* Yeah, these if-statements break every code formatting rule in the world
	       * but I do not really care right now. It saves space for such a small
	       * program. 
	       * -M/h
	       */
	      
	      if (fsent > mult) {fsent = fsent / mult; type[0] = 'K';}
	      if (fsent > mult) {fsent = fsent / mult; type[0] = 'M';}
	      if (fsent > mult) {fsent = fsent / mult; type[0] = 'G';}
	      if (frecv > mult)	{frecv = frecv / mult;rtype[0] = 'K';}
	      if (frecv > mult) {frecv = frecv / mult;rtype[0] = 'M';}
	      if (frecv > mult)	{frecv = frecv / mult;rtype[0] = 'G';}

	      rtype[1] = '\0';

	      printf ("\e[1A\e[2K");

	      printf ("Send: %.3f%s/s - Rec: %.3f%s/s\n", fsent, type, frecv,
		      rtype);

	      _bsent = bsent;
	      _brec = brec;
	      bsentrem = brecrem = 0;

	    }

	}
      fclose (fp);

    }

  return 0;
}

// Fetchline: reads one line out of the given file and returns with that data.
// Will fail gracefully if nothing is there to return.
int
fetchline (char s[], int lim, FILE * fp)
{
  int c, i, j;
  for (i = 0, j = 0; (c = getc (fp)) != EOF && c != '\n'; ++i)
    {
      if (i < lim - 1)
	{
	  s[j++] = c;
	}
    }
  if (c == '\n')
    {
      if (i <= lim - 1)
	{
	  s[j++] = c;
	}
      ++i;
    }
  s[j] = '\0';
  return i;
}
