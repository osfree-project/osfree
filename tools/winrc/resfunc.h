/*************************************************************************

    resfunc.h	1.7
    exported functions for process resources
  
    Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

**************************************************************************/

/* Add a resource to the list */
void add_resource(const char *res_name, int flags, resp r);

/* Create resources from binary data or linked lists */
resp make_bitmap_resource(buffp b);
resp make_cursor_resource(buffp b);
resp make_icon_resource(buffp b);
resp make_menu_resource(buffp b);
resp make_dialog_resource(buffp b);
resp make_accelerator_resource(buffp b);
resp make_string_resource(buffp b);
resp make_binary_resource(buffp b);
resp make_dlginit_resource(buffp b);
resp make_toolbar_resource(buffp b);

/* Menu functions */
buffp append_menu_node(buffp b, buffp append);
buffp set_menu_last(buffp b);
buffp create_menu_popup(char *name, buffp popup, int flags);
buffp create_menu_item(char *name, int id, int flags);
buffp create_menu_separator(void);
unsigned menu_checked(void);
unsigned menu_grayed(void);
unsigned menu_inactive(void);
unsigned menu_help(void);

/* Dialog-box functions */
buffp make_dialog(struct dimvals dims);
buffp set_dialog_style(buffp b, int style);
buffp set_dialog_caption(buffp b, char *caption);
buffp set_dialog_class(buffp b, char *dlgclass);
buffp set_dialog_font(buffp b, char *fontname, int fontsize);
buffp set_dialog_menu(buffp b, const char *menuid);
buffp set_dialog_controls(buffp b, buffp controls);
buffp append_control(buffp b, buffp append);
buffp create_ltext_control(char *name, int id, struct dimvals dims, struct maskvals style);
buffp create_rtext_control(char *name, int id, struct dimvals dims, struct maskvals style);
buffp create_ctext_control(char *name, int id, struct dimvals dims, struct maskvals style);
buffp create_icon_control(char *name, int id, struct dimvals dims, struct maskvals style);
buffp create_edit_control(int id, struct dimvals dims, struct maskvals style);
buffp create_listbox_control(int id, struct dimvals dims, struct maskvals style);
buffp create_combobox_control(int id, struct dimvals dims, struct maskvals style);
buffp create_groupbox_control(char *name, int id, struct dimvals dims, struct maskvals style);
buffp create_pushbutton_control(char *name, int id, struct dimvals dims, struct maskvals style);
buffp create_defpushbutton_control(char *name, int id, struct dimvals dims, struct maskvals style);
buffp create_checkbox_control(char *name, int id, struct dimvals dims, struct maskvals style);
buffp create_radiobutton_control(char *name, int id, struct dimvals dims, struct maskvals style);
buffp create_autoradiobutton_control(char *name,int id, struct dimvals dims, struct maskvals style);
buffp create_scrollbar_control(int id, struct dimvals dims, struct maskvals style);
buffp create_generic_control(char *name, int id, char *class, struct maskvals style, struct dimvals dims);
buffp make_control(char *name, int id, char *class, struct maskvals stylemask, struct dimvals dims, int style);

/* dlginit functions */

buffp make_dlginit();
buffp add_dlginit_item(buffp b, BYTE bItem);

/* toolbar functions */

buffp make_toolbar(WORD wVersion, WORD wWidth, WORD wHeight);
buffp set_toolbar_dims(buffp b, WORD wWidth, WORD wHeight);
buffp add_toolbar_item(buffp b, WORD wItem);

/* Accelerator functions */
buffp append_accel(buffp b, buffp append);
buffp make_accel_s(char *accel, int id, int flags);
buffp make_accel_i(int accel, int id, int flags);

/* String table functions */
void set_string_flags(int flags);
void add_table_string(int id, char *string);

/* Generic resource functions */
resp type_resource_rcdata(resp r);
resp type_resource(resp r, const char *type);
buffp pack_rcdata(struct binflagvals f, struct userresbuffer *urb);
struct userresbuffer *urb_string(char *str);
struct userresbuffer *urb_short(unsigned long i);
struct userresbuffer *urb_long(unsigned long i);
struct userresbuffer *urb_binary(buffp b);
struct userresbuffer *urb_cat(struct userresbuffer *urb1, struct userresbuffer *urb2);

/* Some other potentially useful stuff */
const char *makeintresource(int id); /* MAKEINTRESOURCE without windows.h */
