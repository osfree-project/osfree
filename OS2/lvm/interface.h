/* LVM2\interface.h */
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#define INCL_DOSPROCESS
#include <os2.h>

#define TEXT_25        0
#define TEXT_30 1

#define FLG_NOTHING    0xFFFF
#define FLG_NOEXIT     0
#define FLG_EXIT       1
#define FLG_YES                1
#define FLG_NO         0
#define FLG_FAILED     1
#define FLG_SUCCESS    0

#define REG_NONE       0x0000
#define REG_ALT                0x0008
#define REG_CTRL       0x0004
#define REG_LSHIFT     0x0002
#define REG_RSHIFT     0x0001

#define BSTR0  "        "
#define BSTR1  "┌─┐│┘─└│"
#define BSTR2  "╒═╕│╛═╘│"
#define BSTR3  "╓─╖║╜─╙║"
#define BSTR4  "╔═╗║╝═╚║"
#define BSTR5  "░░░░░░░░"
#define BSTR6  "▒▒▒▒▒▒▒▒"
#define BSTR7  "▓▓▓▓▓▓▓▓"
#define BSTR8  "████████"
#define BSTR9  "▄▄▄█▀▀▀█"
#define BSTR10 "█▀███▄██"
#define BSTR11 "███▐███▌"

#define LINE1  "├─┤"
#define LINE2  "╞═╡"
#define LINE3  "╟─╢"
#define LINE4  "╠═╣"
#define LINE5  "░░░"
#define LINE6  "▒▒▒"
#define LINE7  "▓▓▓"
#define LINE8  "███"

#define LINE9  "█▄█"

#define LINE10 "█▀█"

#define SPACE2  " "

#define MKATRB(bcolor,fcolor )     ((BYTE) (bcolor << 4 ) + fcolor )

#define WM_BLACK             0x00
#define WM_BLUE              0x01
#define WM_GREEN             0x02
#define WM_CYAN              0x03
#define WM_RED               0x04
#define WM_MAGENTA           0x05
#define WM_BROWN             0x06
#define WM_PALEGRAY          0x07
#define WM_DKGREY            0x08
#define WM_LBLUE             0x09
#define WM_LGREEN            0x0A
#define WM_LCYAN             0x0B
#define WM_LRED              0x0C
#define WM_LMAGENTA          0x0D
#define WM_YELLOW            0x0E
#define WM_WHITE             0x0F




#define  KBD_ASCII  0
#define  KBD_CNTRL  1

#define ST_INS 0x80

#define CR          13
#define CTRLCR      10
#define ESC         27
#define SPBAR       57
#define SP          32
#define BS           8
#define CTRLBS     127
#define TAB          9


#define CURT        77
#define CULT        75
#define CTRLCURT   116
#define CTRLCULT   115
#define CUUP        72
#define CUDN        80
#define END         79
#define HOME        71
#define CTRLEND    117
#define CTRLHOME   119
#define PGUP        73
#define PGDN        81
#define CTRLPGUP   132
#define CTRLPGDN   118
#define SHIFTTAB    15


#define DEL         83
#define INS         82
#define GRAYMINUS   74
#define GRAYPLUS    78
#define GRAYASTR    55

/* */
#define F1          59
#define F2          60
#define F3          61
#define F4          62
#define F5          63
#define F6          64
#define F7          65
#define F8          66
#define F9          67
#define F10         68
/* */

#define SHIFTF1     84
#define SHIFTF2     85
#define SHIFTF3     86
#define SHIFTF4     87
#define SHIFTF5     88
#define SHIFTF6     89
#define SHIFTF7     90
#define SHIFTF8     91
#define SHIFTF9     92
#define SHIFTF10    93

#define CTRLF1      94
#define CTRLF2      95
#define CTRLF3      96
#define CTRLF4      97
#define CTRLF5      98
#define CTRLF6      99
#define CTRLF7     100
#define CTRLF8     101
#define CTRLF9     102
#define CTRLF10    103

#define ALTF1      104
#define ALTF2      105
#define ALTF3      106
#define ALTF4      107
#define ALTF5      108
#define ALTF6      109
#define ALTF7      110
#define ALTF8      111
#define ALTF9      112
#define ALTF10     113

#define ALTA        30
#define ALTS        31
#define ALTD        32
#define ALTF        33
#define ALTG        34
#define ALTH        35
#define ALTJ        36
#define ALTK        37
#define ALTL        38

#define ALTQ        16
#define ALTW        17
#define ALTE        18
#define ALTR        19
#define ALTT        20
#define ALTY        21
#define ALTU        22
#define ALTI        23
#define ALTO        24
#define ALTP        25

#define ALTZ        44
#define ALTX        45
#define ALTC        46
#define ALTV        47
#define ALTB        48
#define ALTN        49
#define ALTM        50

#define ALTMIN     130

typedef int (* PFI)();
typedef void (* PFV)();


/**********
* Цепочка функций
**********/
typedef struct _CHF{
   PFI fun;       /* функц. с аргументом void* возвращающая 0 - следующий, 1 - стоп цепочка  */
   struct _CHF *nxt;
   void *usd;     /* пользовательские данные */
   }CHF;

/***************************
 Цепочка клавиш управления
 объектом редактирования
***************************/
typedef struct _CCK{
   int ckey;           /* спец.клавиша по которой надо произвести действие */
   PFI fun;            /* функц. с аргументами  void* объект редактирования, void* usd */
                       /* если =NULL  возвращается 0 -конец редактирования */
                       /* если !=NULL возвращает 1 - конец редактирования, -1 -ошибка */
   void *usd;          /* пользовательские данные */
   struct _CCK *nxt;
   }CCK;

// объект редактирования
typedef struct _EDIT_OBJECT
{
CCK *sp_func_key;      // цепочка клавиш дополнительных операций или окончания редактирования
char *allow;           // символы разрешенные к вводу в объект редактирования
char *deny;            // символы не разрешенные к вводу в объект редактирования
PFI inp_filtr;         // входная фильтрация вводимых символов, если символ недопустим возвращает 0
char *buf;             // буфер объекта редактирования
int blen;              // длина буфера
int ins;               // Insert mode
int cursor;            // точка редактирования
int luy;               // левый верхний
int lux;               // угол
}EDO;
typedef EDO *PEDO;

//поле редактирования
typedef struct _FIELD
{
CCK *sp_func_key;
char *allow;
char *deny;
PFI inp_filtr;
char *buf;
int blen;      // длина буфера
int ins;       // Insert mode
int cur_in_b;  // позиция в буфере
int row;
int col;
int cursor;
int hide;      // Hide mode = char replacer
int wlen;      // длина окна
int cur_in_w;  // позиция в окне
}FLD;
typedef FLD *PFLD;

//окно редактирования (text area)
typedef struct _TEXT_AREA
{
CCK *sp_func_key;
char *allow;
char *deny;
PFI inp_filtr;
char *buf;
int blen;      // длина буфера
int ins;       // Insert mode
int cursor;    // точка редактирования
int luy;       // левый верхний
int lux;       // угол
int height;    // высота окна
int width;     // ширина окна
int f_color;
int b_color;
}MLE;
typedef MLE *PMLE;


// структура описания области экрана
typedef struct _AREA
{
int row;
int col;
int width;
int height;
char *store;
}AREA;
typedef AREA *PAREA;

// структура описания вывода HELP
typedef struct _HELP
{
int lux;       // верхний левый
int luy;       // угол
int width;     // ширина области вывода
int height;    // высота области вывода
int x_offset;  // смещение точки
int y_offset;  // вывода
int f_color;   // цвет переднего плана
int b_color;   // цвет заднего плана
int num_lines; // кол-во строк в тексте
int mw_lines;  // макс. ширина линии
char **text;   // текст
} HELP2;

// вывод времени
typedef struct _TIME_SYS
{
long ctime;
int row;
int col;
int f_color;   // цвет переднего плана
int b_color;   // цвет заднего плана
TID service;
CHF *add_service;
}TIMES;

// подстрочник команд на shift  регистрах
typedef struct _KEY_BAR
{
int row;
int col;
int width;
int max;       // кол-во строк выведенных последний раз
int f_color;   // цвет переднего плана
int b_color;   // цвет заднего плана
TID service;
int flag;
char **n_kbh;
char **s_kbh;
char **c_kbh;
char **a_kbh;
char status;
}KEY_BAR;

extern TIMES Times;

#define PAUSE Kbd_Wait()
//***************** Function *******************
extern void Kbd_Wait(void);
extern int Kbd_GetType(void);
extern int Kbd_GetCode(void);

extern void Vid_Init(int);
extern void Vid_Reset(void);

extern void Vid_Erase(int , int);
extern void Vid_ErasePart(int, int, int, int, int, int);
extern void Vid_EraseLine(int, int, int);

extern void Vid_DrawShadow_1(int, int, int, int, char *);
extern void Vid_DrawShadow_2(int, int, int, int, char *);
extern void Vid_DrawShadow_3(int, int, int, int, char *);
extern void Vid_DrawShadow_4(int, int, int, int, char *);
extern void Vid_DrawFrame(int, int, int, int, char *);
extern void Vid_DrawLine(int, int, int , char *);

extern void Vid_PutStr(char *str,int,int);
extern void Vid_PutStrR(char *,int);
extern void Vid_PutStrC(char *,int);
extern void Vid_PutText(char **,int, int, int, int, int, int);


extern void EDO_addCCKey(PEDO,int,PFI,void *);
extern void EDO_delCCKey(PEDO);
extern int EDO_runCCKey(PEDO, int);
extern int EDO_allow(PEDO,int);
extern int EDO_deny(PEDO,int);
extern int EDO_filtr(PEDO,int);

extern void StartTimeService(int,int,int,int);
extern void StopTimeService(void);

extern void StartKeyBarService(int,int,int,int,int);
extern void StopKeyBarService(void);
extern void SetKeyBar(int,char **);
extern void ResetKeyBar(void);

extern PFLD FLD_new(int, int,char *, char *, char *, PFI);
extern void FLD_delete(PFLD);
extern void FLD_reset(PFLD);
extern int FLD_edit_ascii(UCHAR, PFLD);
extern int FLD_edit_control(UCHAR, PFLD);
extern int FLD_edit(PFLD);
extern void FLD_PrepArray(PFLD *);
extern int FLD_editArray(PFLD *, int *, int);
extern void FLD_load(int, int, PFLD, int, int);

extern PMLE MLE_new(int, int,int,char *, char *, char *, PFI);
extern void MLE_delete(PMLE);
extern void MLE_reset(PMLE);
extern void MLE_lastline(PMLE);
extern void MLE_load(int, int, PMLE, int, int);
extern void MLE_reload(PMLE);
extern int MLE_edit_ascii(UCHAR, PMLE);
extern int MLE_edit_control(UCHAR, PMLE);
extern int MLE_edit(PMLE);



extern PAREA Store_area(int,int,int,int);
extern void Restore_area(PAREA);
extern void Free_area(PAREA);

extern void CHF_AddChain(CHF **,PFI, void *);
extern void CHF_DelChain(CHF *, PFI);
extern void CHF_RunChain(CHF *);
extern void CHF_RemoveChain(CHF *);

extern void KbdAddFLT(PFI, void *);
extern void KbdDelFLT( PFI);
extern void FLT_RunChain(void);

extern void SetHelp(int,int,int,int,int,int);
extern void SetHelpText(char **);
extern void ResetHelp(void);
extern void Load_Help(HELP2 *);
extern void Run_Help(void);


#endif //__INTERFACE_H__
