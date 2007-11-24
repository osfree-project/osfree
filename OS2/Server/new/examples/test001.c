/*----------------------------------------------------------------------------*/
/*                                                                            */
/* The following OS/2 APIs are tested here:                                   */
/*                                                                            */
/* DosSleep                                                                   */
/* DosGetDateTime                                                             */
/* DosBeep                                                                    */
/* VioWrtTTY                                                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* !!!! Don't include LibC or something like this here! Only pure OS/2 API */
/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSPROCESS
#define INCL_DOSDATETIME
#include "os2.h"


void putdec(unsigned int byte)
{
  unsigned char b1;
  int b[30];
  signed int nb;
  int i=0;
  char digits[]="0123456789";
  
  while (1)
  {
    b1=byte%10;
    b[i]=b1;
    nb=byte/10;
    if (nb<=0)
    {
      break;
    }
    i++;
    byte=nb;
  }
  for (nb=i+1;nb>0;nb--) VioWrtTTY(&digits[b[nb-1]],1,0);
  
}

/*----------------------------------------------------------------------------*/
/* displaytime - print out the date and timem                                 */
/*----------------------------------------------------------------------------*/

void displaytime(PDATETIME pdt)
{
    char * days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    char * months[] = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11","12"};
    
    VioWrtTTY("The current time is ", 20, 0);
    VioWrtTTY(days[pdt->weekday], 3, 0);
    VioWrtTTY(" ", 1, 0);
    VioWrtTTY(months[(int)pdt->month], 2, 0);
    VioWrtTTY("-", 1, 0);
    if ((int)pdt->day<10) VioWrtTTY("0", 1, 0);
    putdec((int)pdt->day);
    VioWrtTTY("-", 1, 0);
    putdec((int)pdt->year);
    VioWrtTTY(" ", 1, 0);
    if ((int)pdt->hours<10) VioWrtTTY("0", 1, 0);
    putdec((int)pdt->hours);
    VioWrtTTY(":", 1, 0);
    if ((int)pdt->minutes<10) VioWrtTTY("0", 1, 0);
    putdec((int)pdt->minutes);
    VioWrtTTY(":", 1, 0);
    if ((int)pdt->seconds<10) VioWrtTTY("0", 1, 0);
    putdec((int)pdt->seconds);
    VioWrtTTY("\n", 1, 0);
    return;
}


/*----------------------------------------------------------------------------*/
/* main - the main entry point to the program                                 */
/*----------------------------------------------------------------------------*/

int main(void)
{
    APIRET apiret;
    DATETIME dt;
    ULONG sec = 5;

    VioWrtTTY("OS2 test program main function invoked\n", 39, 0);
    VioWrtTTY("Test basic library functionality:\n", 34, 0);
    VioWrtTTY("DosSleep, DosGetDateTime, VioWrtTTY, DosBeep\n", 45, 0);

    VioWrtTTY("Three beeps should follow this message\n", 39, 0);
    DosBeep(0, 0);
    DosSleep(125);
    DosBeep(0, 0);
    DosSleep(125);
    DosBeep(0, 0);

    apiret = DosGetDateTime(&dt);
    displaytime(&dt);

    VioWrtTTY("Sleeping for 5 seconds\n", 23, 0);
    DosSleep(sec * 1000);

    apiret = DosGetDateTime(&dt);
    displaytime(&dt);
    return 0;
}

