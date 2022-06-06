# makefile
# Created by IBM WorkFrame/2 MakeMake at 23:30:05 on 18 Feb 2012
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker

.SUFFIXES:

.SUFFIXES: \
    .c .obj 

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Iinclude /Sp1 /Ss /Q /Ti /G5 /Tm /Gf /Gi /C %s

{E:\Evgen\LVM2}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Iinclude /Sp1 /Ss /Q /Ti /G5 /Tm /Gf /Gi /C %s

all: \
    .\lvm.exe

.\lvm.exe: \
    .\volume.obj \
    .\Volume_Manager.obj \
    .\Bad_Block_Relocation.obj \
    .\BootManager.obj \
    .\constant.obj \
    .\CRC.obj \
    .\disk.obj \
    .\DiskIO.obj \
    .\Dlist.obj \
    .\Drive_Linking.obj \
    .\engine.obj \
    .\getsectors.obj \
    .\Handle_Manager.obj \
    .\help.obj \
    .\interface.obj \
    .\logging.obj \
    .\lvm.obj \
    .\LVM_Cli.obj \
    .\menu.obj \
    .\Partition_Manager.obj \
    .\Pass_Thru.obj \
    .\show.obj \
    .\strings.obj \
    .\user.obj
    @echo " Link::Linker "
    icc.exe @<<
     /B" /exepack:2 /st:256000 /packd /optfunc"
     /Felvm.exe 
     .\volume.obj
     .\Volume_Manager.obj
     .\Bad_Block_Relocation.obj
     .\BootManager.obj
     .\constant.obj
     .\CRC.obj
     .\disk.obj
     .\DiskIO.obj
     .\Dlist.obj
     .\Drive_Linking.obj
     .\engine.obj
     .\getsectors.obj
     .\Handle_Manager.obj
     .\help.obj
     .\interface.obj
     .\logging.obj
     .\lvm.obj
     .\LVM_Cli.obj
     .\menu.obj
     .\Partition_Manager.obj
     .\Pass_Thru.obj
     .\show.obj
     .\strings.obj
     .\user.obj
<<

.\volume.obj: \
    E:\Evgen\LVM2\volume.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}constant.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}display.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}getkey.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}panels.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}user.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}strings.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}help.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}alvm.h

.\user.obj: \
    E:\Evgen\LVM2\user.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}constant.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}display.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}getkey.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}panels.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}user.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}strings.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}help.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}alvm.h

.\strings.obj: \
    E:\Evgen\LVM2\strings.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}constant.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}panels.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}user.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}strings.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}alvm.h

.\show.obj: \
    E:\Evgen\LVM2\show.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}constant.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}display.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}getkey.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}panels.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}user.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}edit.h

.\Pass_Thru.obj: \
    E:\Evgen\LVM2\Pass_Thru.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}engine.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}dlist.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}diskio.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Constants.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}CRC.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Pass_Thru.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}os2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Types.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_stru.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Data.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_plug.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_LIST.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_HAND.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_gbls.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_cons.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_type.h

.\Partition_Manager.obj: \
    E:\Evgen\LVM2\Partition_Manager.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}engine.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}dlist.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}diskio.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Constants.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Handle_Manager.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}partition_manager.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}CRC.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Logging.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Volume_Manager.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}os2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Types.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_stru.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Data.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_plug.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_LIST.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_HAND.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_gbls.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_cons.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_type.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}extboot.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}mbr.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}mbr_os2.h

.\menu.obj: \
    E:\Evgen\LVM2\menu.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}constant.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}display.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}getkey.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}panels.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}user.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}alvm.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}edit.h

.\LVM_Cli.obj: \
    E:\Evgen\LVM2\LVM_Cli.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Logging.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_gbls.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}getkey.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}panels.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}user.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}strings.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Cli.h

.\lvm.obj: \
    E:\Evgen\LVM2\lvm.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Logging.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_gbls.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}constant.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}display.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}getkey.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}panels.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}user.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}strings.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}alvm.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Cli.h

.\logging.obj: \
    E:\Evgen\LVM2\logging.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}engine.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}dlist.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Logging.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}os2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Types.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_stru.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Data.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_plug.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_LIST.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_HAND.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_gbls.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_cons.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_type.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}alvm.h

.\interface.obj: \
    E:\Evgen\LVM2\interface.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Logging.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}constant.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}display.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}getkey.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}panels.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}user.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}edit.h

.\help.obj: \
    E:\Evgen\LVM2\help.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}constant.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}display.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}getkey.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}panels.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}user.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}strings.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}help.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}alvm.h

.\Handle_Manager.obj: \
    E:\Evgen\LVM2\Handle_Manager.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}dlist.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Handle_Manager.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_LIST.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_HAND.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_gbls.h

.\getsectors.obj: \
    E:\Evgen\LVM2\getsectors.c

.\engine.obj: \
    E:\Evgen\LVM2\engine.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}engine.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}dlist.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}diskio.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Constants.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Handle_Manager.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}partition_manager.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}CRC.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Bad_Block_Relocation.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Pass_Thru.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Logging.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Volume_Manager.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}os2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Types.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_stru.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Data.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_plug.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_LIST.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_HAND.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Bad_Block_Relocation_Feature.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_gbls.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_cons.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_type.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}BootManager.h

.\Drive_Linking.obj: \
    E:\Evgen\LVM2\Drive_Linking.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}engine.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}dlist.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}diskio.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Constants.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}drive_linking_feature.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}os2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Types.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_stru.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Data.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_plug.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_LIST.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_HAND.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_gbls.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_cons.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_type.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}constant.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}display.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}getkey.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}panels.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Drive_Linking.h

.\Dlist.obj: \
    E:\Evgen\LVM2\Dlist.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}dlist.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_LIST.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_gbls.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}poolman.h

.\DiskIO.obj: \
    E:\Evgen\LVM2\DiskIO.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}dlist.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}diskio.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Constants.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Logging.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Types.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_stru.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Data.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_LIST.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_gbls.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_cons.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_type.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lock.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_intr.h

.\disk.obj: \
    E:\Evgen\LVM2\disk.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Logging.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}constant.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}display.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}getkey.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}panels.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}user.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}strings.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}help.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}alvm.h

.\CRC.obj: \
    E:\Evgen\LVM2\CRC.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h

.\constant.obj: \
    E:\Evgen\LVM2\constant.c

.\BootManager.obj: \
    E:\Evgen\LVM2\BootManager.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}engine.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}dlist.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}diskio.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Constants.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Handle_Manager.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}partition_manager.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}os2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Types.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_stru.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Data.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_plug.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_LIST.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_HAND.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_gbls.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_cons.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_type.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}mbb.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}extboot.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}pboot.h

.\Bad_Block_Relocation.obj: \
    E:\Evgen\LVM2\Bad_Block_Relocation.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}engine.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}dlist.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Constants.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Bad_Block_Relocation.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}os2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Types.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_stru.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Data.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_plug.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_LIST.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_HAND.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Bad_Block_Relocation_Feature.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_gbls.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_cons.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_type.h

.\Volume_Manager.obj: \
    E:\Evgen\LVM2\Volume_Manager.c \
    {E:\Evgen\LVM2;include;$(INCLUDE);}engine.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}gbltypes.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}dlist.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}diskio.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Constants.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Interface.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Handle_Manager.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}partition_manager.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}CRC.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Bad_Block_Relocation.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Pass_Thru.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}drive_linking_feature.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Logging.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Volume_Manager.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}jfs_cntl.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}os2.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Types.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_stru.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_Data.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_plug.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_LIST.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}LVM_HAND.H \
    {E:\Evgen\LVM2;include;$(INCLUDE);}Bad_Block_Relocation_Feature.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_gbls.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_cons.h \
    {E:\Evgen\LVM2;include;$(INCLUDE);}lvm_type.h
