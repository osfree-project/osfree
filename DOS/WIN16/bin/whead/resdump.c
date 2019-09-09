#include <stdio.h>

 
 
#define RESTYPE_CURSOR          0x8001
#define RESTYPE_BITMAP          0x8002
#define RESTYPE_ICON            0x8003
#define RESTYPE_MENU            0x8004
#define RESTYPE_DIALOG          0x8005
#define RESTYPE_STRING          0x8006
#define RESTYPE_FONTDIR         0x8007
#define RESTYPE_FONT            0x8008
#define RESTYPE_ACCEL           0x8009
#define RESTYPE_RCDATA          0x800a
#define RESTYPE_GROUP_CURSOR    0x800d
#define RESTYPE_GROUP_ICON      0x800e
#define RESTYPE_NAME_TABLE      0x800f
 
#define	MOVABLE                 0x0010
#define SHAREABLE               0x0020
#define PRELOAD                 0x0040
 
#define	RES_TBL_OFFSET		0x24

#define	NUMBER		1
#define	STRING		2

#define BYTE	unsigned char
#define WORD	unsigned short
#define DWORD	unsigned int

char *res_name_tbl[] = {
        "Cursor",
        "Bitmap",
        "Icon",
        "Menu",
        "Dialog",
        "String",
        "Fontdir",
        "Font",
        "Accel",
        "User Data",
        "Unknown #11",
        "Unknown #12",
        "Unknown #13",
        "Icon  Dir",
        "Name table",
        "Unknown #16",
};

DWORD	restbl_offset;
WORD	restbl_size;
WORD	shift_count;
BYTE	*restbl;
BYTE	*name_tbl;
 
extern	DWORD  ne_offset;
extern	WORD	get_word();
extern	DWORD	get_dword();

typedef struct name_list {
        WORD    type_ordinal;
        WORD    id_ordinal;
        char    *type_name;
        char    *id_name;
        struct  name_list *next;
} NAMELIST;
 
NAMELIST *name_table = (NAMELIST *) NULL;

void
asciiz_to_pascal(str)
char    *str;   
{
    char count, i;
 
    count = strlen(str);
    
    for (i = count; i > 0; i--)
        str[i] = str[i - 1];
    str[i] = count;
}
 

void
print_pascal_string(fileptr, str)
FILE    *fileptr;
char    *str;
{
    BYTE    i;

    if ((str == (char *)NULL) || (str[0] == 0)) {
        fprintf(fileptr, "0");
        return;
    }
    fprintf(fileptr, "\"");
    for (i = 1; i <= ( BYTE )str[0]; i++)
        fprintf(fileptr,"%c",str[i]);
    fprintf(fileptr, "\"");
    for (i = (BYTE ) str[0];i < 20; i++)
        fprintf(fileptr," ");

}


char    *
get_count(tblptr, count_addr)
char    *tblptr;
WORD    *count_addr;
{
        tblptr += 2;
        *count_addr = get_word(tblptr);
        tblptr += 6;
        return  (tblptr);
}


DWORD
load_data(in, data_tbl_ptr, tblptr)
FILE	*in;
BYTE **data_tbl_ptr;
char *tblptr;
{
        DWORD data_offset;
        DWORD data_len;

        data_offset = (DWORD)(get_word(tblptr) << shift_count);
        data_len = (DWORD)(get_word(tblptr+2) << shift_count);
        if((*data_tbl_ptr = (BYTE *) malloc(data_len)) == (BYTE *)NULL) {
                fprintf(stderr, "No Memory to load data\n");
		exit(-1);
	}
        fseek(in, data_offset, 0);
        fread(*data_tbl_ptr, 1, data_len, in);
        return (data_len);
}


 
char    *
search_namelist(res_type, id, id_type)
WORD    res_type, id;
int	*id_type;
{
        NAMELIST *temp;
 
        temp = name_table;
        
        while(temp != (NAMELIST *) NULL) {
                if ((temp->type_ordinal == (res_type & 0x7fff))
                   && (temp->id_ordinal == id)) {
			*id_type = STRING;
                        return temp->id_name;
		}
                temp = temp->next;
        }
	*id_type = NUMBER;
        return ((char *)(DWORD)id);
}
 
int
get_flags(res_type, tblptr, flags_addr, res_id_addr)
WORD    res_type;
char    *tblptr;
WORD    *flags_addr;
char    **res_id_addr;
{
	int id_type = NUMBER;

        *flags_addr = get_word(tblptr + 4);
        *res_id_addr = (char *) (long) get_word(tblptr + 6);
        if (((int )*res_id_addr & 0x8000) == 0) {
		id_type = STRING;
                *res_id_addr =  (char *) restbl + (int)*res_id_addr;
		return (id_type);
        } else {
                *res_id_addr = search_namelist(res_type, *res_id_addr, &id_type);
		return (id_type);
	}
}
 
/*
 * Generates the structures for the  name table entries
 */
 
void
do_nametbl(in, resptr_addr)
FILE	*in;
char **resptr_addr;
{
        WORD i, count, flags;
        char *tblptr, *res_id;
        WORD    type_ordinal;
        WORD    id_ordinal;
        WORD    byte_count;
        BYTE    *type, *id;
        BYTE    *ptr;
        NAMELIST *new_name;
 
        tblptr  = get_count(*resptr_addr, &count);
        for (i = 0; i < count; i++) {
                load_data(in, &name_tbl, tblptr);
                ptr = name_tbl;
                while(1) {
                        byte_count = get_word(ptr);
                        if (byte_count == 0)
                                break;
                        if((new_name = (NAMELIST *) malloc(sizeof(NAMELIST)))
                           == (NAMELIST *)NULL) {
				fprintf(stderr,"No Memory to load data\n");
				exit(-1);
			}
                        new_name->type_ordinal = get_word(ptr +2);
                        new_name->id_ordinal = get_word(ptr + 4);
                        ptr += 6;
                        new_name->type_name = (char *)ptr;
                        ptr +=  (strlen(ptr) + 1);
                        asciiz_to_pascal(new_name->type_name);
                        new_name->id_name = (char *)ptr;
                        ptr += (strlen(ptr) + 1);
                        asciiz_to_pascal(new_name->id_name);
                        new_name->next = name_table;
                        name_table = new_name;
                }
                tblptr += 12;
        }

        *resptr_addr = tblptr;
}

void
print_common(res_type,resptr)
WORD    res_type;
char    *resptr;
{
        WORD    count, i, flags;
        DWORD   data_offset, data_len;
	int	id_type;
        char    *tblptr, *res_id, *type_str;

	if (res_type > 0x8010)
		res_type = 0x8010;
	type_str = res_name_tbl[res_type - 0x8001];
        tblptr = get_count(resptr, &count);
        for ( i = 1; i <= count; i++) {
                fprintf(stdout, "%-20s", type_str);
                id_type = get_flags(res_type, tblptr, &flags, &res_id);
                data_offset = (DWORD)(get_word(tblptr) << shift_count);
                data_len = (DWORD)(get_word(tblptr+2) << shift_count);
                if (id_type ==  NUMBER)
                        fprintf(stdout, "%-21d ", (int)res_id & 0x7fff);
                else
                        print_pascal_string(stdout, res_id);
                fprintf(stdout, "%10lx%10lx\t", data_offset, data_len);
                if (flags & MOVABLE)
                        fprintf(stdout," M ");
                if (flags & SHAREABLE)
                        fprintf(stdout," S ");
                if (flags & PRELOAD)
                        fprintf(stdout," P");
                fprintf(stdout,"\n");
                tblptr += 12;
        }
}
 

do_skip(resptr_addr)
char   **resptr_addr;
{
        char *tblptr;
        WORD    count;

        tblptr = get_count(*resptr_addr, &count);
        tblptr += count * 12;
        *resptr_addr = tblptr;
}


dump_restbl(in)
FILE	*in;
{
	char	temp_buf[4];
	BYTE	*resptr;
	WORD	res_type;

	fseek(in, ne_offset + RES_TBL_OFFSET, 0);
	fread(temp_buf, 1, 4, in);
	restbl_offset = ne_offset + (DWORD)get_word(temp_buf);
	restbl_size = get_word(temp_buf+2) - get_word(temp_buf);
        if (restbl_size == 0) {
                fprintf(stderr, "No resources found \n");
                return;
        }
        if((restbl = (BYTE *) malloc(restbl_size)) == (BYTE *)NULL) {
		fprintf(stderr, "No Memory to load tables\n");
		return;
	}
        fprintf(stdout,"Type               Identifer                    Offset  length   Attributes\n\n");

        fseek(in, restbl_offset, 0);
        fread(restbl, 1, restbl_size, in);
	resptr = restbl;
	shift_count = get_word(restbl);
	resptr += 2;
	do {
		res_type = get_word(resptr);
		if (res_type == 0)
			break;
		if (res_type & 0x8000) {
			print_common(res_type, resptr);
			if (res_type == RESTYPE_NAME_TABLE) {
				do_nametbl(in, &resptr);
				continue;
			}
		}
		do_skip(&resptr);
	} while(res_type != 0);
}
