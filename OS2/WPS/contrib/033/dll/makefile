#
# Makefile pour le support des langues de XWorkplace.
#
# Un fichier makefile contient des informations indiquant comment plusieurs fichiers sources
# s'articulent entre eux, et qui pr‚cise quels modules de code doivent ˆtre mis … jour lorsque
# certains fichiers sources sont modifi‚s.
#
# Ce fichier doit ˆtre utilis‚ avec IBM NMAKE, composant de IBM C-Set/2 et de VisualAge C++.
# Ce fichier ne fonctionne probablement pas avec d'autres applications MAKE telles que
# GNU make ou DMAKE.
#
#       Copyright (C) 1997-2006 Ulrich M”ller.
#       Traduction (C) 2003-2005 quipe de traduction de XWorkplace : 
#       Laurent Catinaud, Aymeric Peyret, Ren‚ Louisor-Marchini,
#       Guillaume Gay. 
#       Ce fichier est la traduction d'un fichier source de XWorkplace, 
#       il suit les mˆmes rŠgles de distribution et de modification que
#       XWorkplace, pr‚cis‚es ci-aprŠs. 
#
#       XWorkplace est un logiciel libre ; vous pouvez le redistribuer
#       et/ou le modifier suivant les dispositions de la version 2 de la
#       "GNU General Public Licence", publi‚e par la Free Software 
#       Foundation, et pr‚sente dans le fichier "COPYING" de la
#       distribution principale de XWorkplace.
#       En l'absence de traduction officielle de la "GNU General Public
#       Licence" … l'adresse http://www.april.org/gnu/gpl_french.html,
#       la seule version faisant foi est la version Anglaise fournie
#       avec la distribution de XWorkplace
#
#       Ce programme est distribu‚ … toutes fins utiles, mais SANS
#       AUCUNE GARANTIE, ni explicite ni implicite, y compris les
#       garanties de commercialisation ou d'adaptation … un but
#       particulier. L'utilisateur assume l'ensemble des risques quant …
#       la qualit‚ et aux effets du programme. 
#       Reportez-vous aux clauses de la GNU General public licence pour
#       toute pr‚cision compl‚mentaire. 
#
# Modifications … effectuer pour la traduction : voir trois lignes plus bas
#

# Say hello to yourself.
!if [@echo +++++ Entering $(MAKEDIR)]
!endif

# include setup (compiler options etc.)
!include ..\..\config.in
!include ..\..\make\setup.in

# Ceci est le code linguistique … modifier pour la traduction de XWorkplace
# Modifiez-le et XWorplace prendra en charge votre langue.
# Le code DOIT ˆtre saisi en 3 chiffres.
LANGUAGE = 033

#
# NE PAS MODIFIER CE QUI SUIT ! (sauf eventuellement les commentaires...)
#

# Param‚trage de quelques variables d'environnement
#
#

# OUTPUTDIR pr‚cise le dossier dans lequel les fichiers .OBJ
# seront cr‚‚s.
OUTPUTDIR = $(XWP_OUTPUT_ROOT)

# cr‚ation du dossier de sortie
!if [@md $(OUTPUTDIR) 2> NUL]
!endif
!if [@md $(MODULESDIR) 2> NUL]
!endif

OBJS = $(OUTPUTDIR)\xfldr$(LANGUAGE).obj

XFLDHELP = $(XWPRUNNING)\help

# D‚finition des modules sur lesquels NMAKE travaillera.
# .SUFFIXES est un mot-cl‚ r‚serv‚ de NMAKE ("pseudotarget") pour
# d‚finir des extensions de fichiers que NMAKE reconnaŒtra.
.SUFFIXES: .txt .c .obj .dll .rc .res .html .ipf .hlp .inf

all: $(XWPRUNNING)\bin\xfldr$(LANGUAGE).dll
    @echo ----- Leaving $(MAKEDIR)

# Now define inference rules for what to do with certain file
# types, based on their file extension.
# The syntax we need here is ".fromext.toext".
# So whenever NMAKE encounters a .toext file, it
# executes what we specify here.

.c{$(OUTPUTDIR)}.obj:
        $(CC_DLL_MT) -I$(INCLUDE) -c /Fo$(OUTPUTDIR)\$(@B).obj $<

#
# Now define inference rules for the different file types.
#

# V1.00: the following has been removed
# xfldr$(LANGUAGE).msg: $$(@B).txt
#          mkmsgf $*.txt $*.msg

$(OUTPUTDIR)\xfldr$(LANGUAGE).obj: $(@B).c

$(OUTPUTDIR)\xfldr$(LANGUAGE).res: *.rc *.dlg *.ico ..\..\include\dlgids.h
         $(RC) -r -x2 $(@B).rc $(OUTPUTDIR)\$(@B).res

$(MODULESDIR)\xfldr$(LANGUAGE).dll: $(@B).def $(OBJS) $(OUTPUTDIR)\$(@B).res ..\..\include\dlgids.h
         $(LINK_BASE) $(@B).def $(OBJS) /OUT:$(MODULESDIR)\$(@B).dll
         $(RC) -p -x2 $(OUTPUTDIR)\$(@B).res $(MODULESDIR)\$(@B).dll

# main target
$(XWPRUNNING)\bin\xfldr$(LANGUAGE).dll: $(MODULESDIR)\xfldr$(LANGUAGE).dll
!ifdef XWP_UNLOCK_MODULES
         $(RUN_UNLOCK) $(XWPRUNNING)\bin\$(@B).dll
!endif
         $(COPY) $(MODULESDIR)\$(@B).dll $(XWPRUNNING)\bin


