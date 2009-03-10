#if __cplusplus
extern "C" {
#endif

#define WM_PLAYSTOP           WM_USER+69
#define WM_PLAYERROR          WM_USER+100
#define WM_SEEKSTOP           WM_USER+666
#define WM_METADATA           WM_USER+42
#define WM_CHANGEBR           WM_USER+43


int  _System decoder_init(void **w);
BOOL _System decoder_uninit(void *w);

#define DECODER_PLAY     1 /* returns 101 -> already playing
                                      102 -> error, decoder killed and restarted */
#define DECODER_STOP     2 /* returns 101 -> already stopped
                                      102 -> error, decoder killed (and stopped) */
#define DECODER_FFWD     3
#define DECODER_REW      4
#define DECODER_JUMPTO   5
#define DECODER_SETUP    6
#define DECODER_EQ       7
#define DECODER_BUFFER   8
#define DECODER_SAVEDATA 9

typedef struct {
   int size;

   /* --- DECODER_PLAY, STOP */

   char *filename;
   char *URL;
   char *drive; /* for CD ie.: "X:" */
   int  track;
   int sectors[2];  /* play from sector x to sector y */
   char *other;

   /* --- DECODER_REW, FFWD and JUMPTO */

   int jumpto;     /* absolute positioning in milliseconds */
   int ffwd;       /* 1 = start ffwd, 0 = end ffwd */
   int rew;        /* 1 = start rew, 0 = end rew */

   /* --- DECODER_SETUP */

   /* specify a function which the decoder should use for output */
   int (* _System output_play_samples)(void *a, FORMAT_INFO *format, char *buf, int len, int posmarker);
   void *a; /* only to be used with the precedent function */
   int audio_buffersize;

   char *proxyurl; /* NULL = none */
   char *httpauth; /* NULL = none */

   /* error message function the decoder should use */
   void (* _System error_display)(char *);

   /* info message function the decoder should use */
   /* this information is always displayed to the user right away */
   void (* _System info_display)(char *);

   HEV playsem;  /* this semaphore is reseted when DECODER_PLAY is requested
                    and is posted on stop */

   HWND hwnd; // commodity for PM interface, decoder must send a few messages to this handle

   /* values used for streaming inputs by the decoder */
   int buffersize; /* read ahead buffer in bytes, 0 = disabled */
   int bufferwait; /* block the first read until the buffer is filled */

   char *metadata_buffer; /* the decoder will put streaming metadata in this  */
   int   metadata_size;   /* buffer before posting WM_METADATA                */

   /* -- DECODER_BUFFER */

   int bufferstatus; /* reports how many bytes there are available in the buffer */

   /* --- DECODER_EQ */

   /* usually only useful with MP3 decoder */
   int equalizer;    /* TRUE or FALSE */
   float *bandgain;  /* point to an array like this bandgain[#channels][10] */

   /* --- DECODER_SAVEDATA */

   char *save_filename;

} DECODER_PARAMS;

/* returns 0 -> ok
           1 -> command unsupported
           1xx -> msg specific */
ULONG _System decoder_command(void *w, ULONG msg, DECODER_PARAMS *params);

#define DECODER_STOPPED  0
#define DECODER_PLAYING  1
#define DECODER_STARTING 2
ULONG _System decoder_status(void *w);

/* WARNING!! this _can_ change in time!!! returns stream length in ms */
/* the decoder should keep in memory a last valid length so the call  */
/* remains valid even if decoder_status() == DECODER_STOPPED          */
ULONG _System decoder_length(void *w);

// static char *modes[4] = { "Stereo", "Joint-Stereo", "Dual-Channel", "Single-Channel" };

/* NOTE: the information returned is only based on the FIRST header */
typedef struct {
   int size;

   FORMAT_INFO format; /* stream format after decoding */

   int songlength; /* in milliseconds, smaller than 0 -> unknown */
   int junklength; /* bytes of junk before stream start, if < 0 -> unknown */

   /* mpeg stuff */
   int mpeg; /* 25 = MPEG 2.5, 10 = MPEG 1.0, 20 = MPEG 2.0, 0 = not an MPEG */
   int layer; /* 0 = unknown */
   int mode; /* use it on modes[4] */
   int modext; /* didn't check what this one does */
   int bpf; /* bytes in the mpeg frame including header */
   int bitrate; /* in kbit/s */
   int extention; /* didn't check what this one does */

   /* track stuff */
   int startsector;
   int endsector;

   /* module stuff */
   int numchannels;  /* 0 = not a MODule */
   int numpatterns;
   int numpositions;

   /* general technical information string */
   char tech_info[128];

   /* song information */
   char title[128];
   char artist[128];
   char album[128];
   char year[128];
   char comment[128];
   char genre[128];

} DECODER_INFO;

/* returns
      0 = everything's perfect, structure is set
      100 = error reading file (too small?)
      200 = decoder can't play that
      1001 = http error occured, check http_strerror() for string;
      other values = errno */
ULONG _System decoder_fileinfo(char *filename, DECODER_INFO *info);
ULONG _System decoder_trackinfo(char *drive, int track, DECODER_INFO *info);

typedef struct
{
   int sectors;
   int firsttrack;
   int lasttrack;
} DECODER_CDINFO;

ULONG _System decoder_cdinfo(char *drive, DECODER_CDINFO *info);


/* returns ORed values */
#define DECODER_FILENAME  0x1
#define DECODER_URL       0x2
#define DECODER_TRACK     0x4
#define DECODER_OTHER     0x8
/* size is i/o and is the size of the array.
   each ext should not be bigger than 8bytes */
ULONG _System decoder_support(char *fileext[], int *size);


#if __cplusplus
}
#endif
