#define INCL_DOSDATETIME
#include "incl.hpp"
#include <string.h>
#include <stddef.h>

extern "C" {

APIRET os2APIENTRY DosGetDateTime(PDATETIME pdt) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        pdt->hours = (UCHAR)st.wHour;
        pdt->minutes = (UCHAR)st.wMinute;
        pdt->seconds = (UCHAR)st.wSecond;
        pdt->hundredths = (UCHAR)(st.wMilliseconds/10);
        pdt->day = (UCHAR)st.wDay;
        pdt->month = (UCHAR)st.wMonth;
        pdt->year = (USHORT)st.wYear;
        pdt->weekday = (UCHAR)st.wDayOfWeek;
        TIME_ZONE_INFORMATION tzi;
        if(GetTimeZoneInformation(&tzi)!=(DWORD)-1)
                pdt->timezone = (SHORT)tzi.Bias;
        else
                pdt->timezone = (SHORT)-1;
        return 0;
}


APIRET os2APIENTRY DosSetDateTime(PDATETIME pdt) {
        SYSTEMTIME st;
        st.wHour = pdt->hours;
        st.wMinute = pdt->minutes;
        st.wSecond = pdt->seconds;
        st.wMilliseconds = (WORD)(pdt->hundredths * 10);
        st.wDay = pdt->day;
        st.wMonth = pdt->month;
        st.wYear = pdt->year;
        if(SetLocalTime(&st))
                return 0;
        else
                return 327; //ts_datetime (most likely)
}

};

