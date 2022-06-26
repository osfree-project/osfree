#
# makefile:
#       makefile pour le r‚pertoire 033\misc.
#       · utiliser avec IBM NMAKE, fourni avec les comilateurs IBM,
#       le Developer's Toolkit, et le DDK.
#
#       Tous les makefiles ont ‚t‚ restructur‚s avec la cersion 0.9.0.
#
#       Appel‚ depuis :    033
#
#       Ceci copie les fichier du r‚pertoire en cours dans les
#       sous-r‚pertoires XWPRUNNING si n‚cessaire.
#
# Traduction fran‡aise : Guillaume Gay 2005-11-03

# Dites Bonjour … vous-mˆme.
!if [@echo +++++ Entering $(MAKEDIR)]
!endif

# inclusion des paramŠtres (options du compilateur, etc.)
!include ..\..\config.in
!include ..\..\make\setup.in

# Ceci est le code linguistique … modifier pour la langue que la DLL de
# XWorkplace devra supporter. Modifiez-le et XWorplace prendra en charge
# votre langue.
# Le code DOIT ˆtre saisi en 3 chiffres.
LANGUAGE = 033

#
# NE PAS MODIFIER CE QUI SUIT ! (sauf eventuellement les commentaires...)
#

# D‚finit les suffixes des fichiers sur lesquels NMAKE va travailler.
# .SUFFIXES est un mot-cl‚ r‚serv‚ par NMAKE ("pseudo-cible") pour la
# d‚finition des extensions de fichier que NMAKE devra reconnaŒtre.
.SUFFIXES: .tmf

FILES_TO_COPY = \
    $(XWPRUNNING)\help\xfldr$(LANGUAGE).tmf \
!ifndef XWPLITE
    $(XWPRUNNING)\install\sound$(LANGUAGE).cmd \
    $(XWPRUNNING)\help\drvrs$(LANGUAGE).txt \
    $(XWPRUNNING)\help\xfcls$(LANGUAGE).txt
!endif

!ifdef XWPLITE
CROBJ_SOURCE = crobj$(LANGUAGE)_lite.cmd
!else
CROBJ_SOURCE = crobj$(LANGUAGE).cmd
!endif

!ifdef XWPLITE
INSTL_SOURCE = instl$(LANGUAGE)_lite.cmd
!else
INSTL_SOURCE = instl$(LANGUAGE).cmd
!endif

all: $(FILES_TO_COPY)
    $(COPY) $(CROBJ_SOURCE) $(XWPRUNNING)\install\crobj$(LANGUAGE).cmd
    $(COPY) $(INSTL_SOURCE) $(XWPRUNNING)\install\instl$(LANGUAGE).cmd
    @echo ----- Leaving $(MAKEDIR)

# copie du fichier tmf dans le r‚pertoire modulesdir en premier
$(XWPRUNNING)\help\xfldr$(LANGUAGE).tmf: $(MODULESDIR)\$(@B).tmf
    $(COPY) $(MODULESDIR)\$(@B).tmf $@

# composition d'un fichier tmf file dans modulesdir … l'aide de d‚iff‚rents fichiers tmf sources
!ifdef XWPLITE
$(MODULESDIR)\xfldr$(LANGUAGE).tmf: xfldr$(LANGUAGE).tmf xwplite$(LANGUAGE).tmf
    $(COPY) /b $(@B).tmf+xwplite$(LANGUAGE).tmf $@
!else
$(MODULESDIR)\xfldr$(LANGUAGE).tmf: xfldr$(LANGUAGE).tmf xwpfull$(LANGUAGE).tmf
    $(COPY) /b $(@B).tmf+xwpfull$(LANGUAGE).tmf $@
!endif

$(XWPRUNNING)\install\sound$(LANGUAGE).cmd: $(@B).cmd
    $(COPY) $(@B).cmd $(XWPRUNNING)\install

# fichiers *.txt

$(XWPRUNNING)\help\drvrs$(LANGUAGE).txt: $(@B).txt
    $(COPY) $(@B).txt $(XWPRUNNING)\help

$(XWPRUNNING)\help\xfcls$(LANGUAGE).txt: $(@B).txt
    $(COPY) $(@B).txt $(XWPRUNNING)\help

