#include "incl.hpp"
#include "filemgr0.hpp"

void ntfiletime2os2(const FILETIME &ft, FDATE *fdate, FTIME *ftime) {
        SYSTEMTIME st;
        FileTimeToSystemTime(&ft,&st);
        fdate->day = st.wDay;
        fdate->month = st.wMonth;
        fdate->year = (WORD)(st.wYear - 1980);
        ftime->twosecs = (WORD)(st.wSecond/2);
        ftime->minutes = st.wMinute;
        ftime->hours = st.wHour;

}

void ntfileattr2os2(DWORD dwAttrs, ULONG *ulAttrs) {
        *ulAttrs = 0;
        if(dwAttrs&FILE_ATTRIBUTE_ARCHIVE)
                *ulAttrs |= FILE_ARCHIVED;
        if(dwAttrs&FILE_ATTRIBUTE_DIRECTORY)
                *ulAttrs |= FILE_DIRECTORY;
        if(dwAttrs&FILE_ATTRIBUTE_HIDDEN)
                *ulAttrs |= FILE_HIDDEN;
        if(dwAttrs&FILE_ATTRIBUTE_READONLY)
                *ulAttrs |= FILE_READONLY;
        if(dwAttrs&FILE_ATTRIBUTE_SYSTEM)
                *ulAttrs |= FILE_SYSTEM;
}

void os22ntfiletime(FDATE fdate, FTIME ftime, FILETIME *ft) {
        SYSTEMTIME st;
        st.wYear = (WORD)(fdate.year + 1980);
        st.wMonth = fdate.month;
        st.wDay = fdate.day;
        st.wHour = ftime.hours;
        st.wMinute = ftime.minutes;
        st.wSecond = (WORD)(ftime.twosecs*2);
        st.wMilliseconds = 0;
        SystemTimeToFileTime(&st,ft);
}



