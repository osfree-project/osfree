
# MAKE file for Listbox.MAK     Version 3.0

# Revised:  1994-07-05

# Copyright ¸ 1987-1994  Prominare Inc.

# MAKE file created by Prominare Builder  Version 3.0

Update: $(OBJDIR)EntryFld.Obj $(OBJDIR)EventMgr.Obj $(OBJDIR)heapmgr.Obj\
        $(OBJDIR)listbox.Obj $(OBJDIR)Listbox.Res $(OBJDIR)listmgr.Obj\
        $(OBJDIR)SndMgr.Obj $(OBJDIR)TraffMgr.Obj

$(OBJDIR)EntryFld.Obj: EntryFld.C
 $(CC) $(C_SW)-Fo$*.Obj EntryFld.C

$(OBJDIR)EventMgr.Obj: EventMgr.C
 $(CC) $(C_SW)-Fo$*.Obj EventMgr.C

$(OBJDIR)heapmgr.Obj: heapmgr.c
 $(CC) $(C_SW)-Fo$*.Obj heapmgr.c

$(OBJDIR)listbox.Obj: ListBox.C
 $(CC) $(C_SW)-Fo$*.Obj ListBox.C

$(OBJDIR)Listbox.Res: Listbox.Rc
 $(RC) $(RC_SW) Listbox.Rc $*.Res

$(OBJDIR)listmgr.Obj: listmgr.c
 $(CC) $(C_SW)-Fo$*.Obj listmgr.c

$(OBJDIR)SndMgr.Obj: SndMgr.C
 $(CC) $(C_SW)-Fo$*.Obj SndMgr.C

$(OBJDIR)TraffMgr.Obj: TraffMgr.C
 $(CC) $(C_SW)-Fo$*.Obj TraffMgr.C

