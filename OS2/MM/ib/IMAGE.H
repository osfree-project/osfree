#define ID_IMAGE                        100
#define ID_ARTICON                      101
#define ID_CROSSICON                    102

#define IDM_CONVERTIMAGE                500
#define IDM_IMAGELOAD                   501
#define IDM_BESTSIZE                    502
#define IDM_IMAGEINFO                   503
#define IDM_CLOSE                       504
#define IDM_SAVEIMAGE                   505
#define IDM_CROP                        506
#define IDM_COPY                        507
#define IDM_PASTE                       508
#define IDM_CANCEL                      509
#define IDM_ICONIFY                     510
#define IDM_STRETCHPASTE                511
#define IDM_WEIRDPASTE                  512
#define IDM_HORIZONTAL                  513
#define IDM_VERTICAL                    514
#define IDM_RESIZE                      515
#define IDM_ROTATE                      516
#define IDM_IMAGERELOAD                 517
#define IDM_COMPRESSION                 518

#define IDM_HELP                        999

#define IMGS_FRAME                      1000
#define IMGS_LISTBOX                    1001
#define IMGS_NAME                       1002
#define IMGS_BITCOUNT                   1003
#define IMGS_BITCOUNTHDR                1004
#define IMGS_ICONIFY                    1005

#define LIMGS_FRAME                     2000

#define RSZ_FRAME                       2100
#define RSZ_OLDSIZE                     2101
#define RSZ_XSIZE                       2102
#define RSZ_YSIZE                       2103
#define RSZ_RATIO                       2104


#define UM_NOTIFY                       (WM_USER + 1)
#define UM_CONTAINER_FILLED             (WM_USER + 2)
#define UM_THREADUSE                    (WM_USER + 3)
#define UM_SETUP                        (WM_USER + 4)
#define UM_PAINT                        (WM_USER + 5)
#define UM_XHAIRS                       (WM_USER + 6)

#define priority_idle()     DosSetPriority(PRTYS_THREAD,PRTYC_IDLETIME,10L,0L)
#define priority_normal()   DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,0L,0L)
#define priority_tweaked()  DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,1L,0L)
#define priority_bumped()   DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,3L,0L)
#define priority_critical() DosSetPriority(PRTYS_THREAD,PRTYC_FOREGROUNDSERVER,3L,0L)
#define priority_max()      DosSetPriority(PRTYS_THREAD,PRTYC_FOREGROUNDSERVER,31L,0L)

