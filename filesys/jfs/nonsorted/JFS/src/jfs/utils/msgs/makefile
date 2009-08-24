# $Id: makefile,v 1.1.1.1 2003/05/21 13:42:46 pasha Exp $ 

#       SCCSID = @(#)1.2  2/2/99 07:26:22 src/jfs/utils/msgs/makefile, jfsutil, w45.fs32, 990417.1
#----------------------------------------------------------------------------
#
#   Copyright (c) International Business Machines  Corp., 2000
#
#   This program is free software;  you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or 
#   (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY;  without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#   the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program;  if not, write to the Free Software 
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# MAKEFILE for the src\jfs\utils\msgs   directory
#
# COMPONENT_NAME: jfsutil
#
#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
# SUBSYS is a macro to define the name of the subsystem that this makefile is a
# part of.   It is the name of the subdirectory that is above all of the
# individual components.
#
# SUBSYS_RELATIVE is a macro to define the name of this component.
# SRC_RELATIVE is the full path for the current directory from the
# ROOT.
#
# This uses the substitution feature which takes the current directory of
# this makefile (MAKEDIR) and removes the relative part of the directory to
# get the TOP level directory.
#
# The other macros are used to build directories in other levels of the tree.
#------------------------------------------------------------------------------

SUBSYS          = jfs
SUBSYS_RELATIVE = utils\msgs
SRC_RELATIVE    = src\$(SUBSYS)\$(SUBSYS_RELATIVE)
#SRC_NL_RELATIVE = mri\$(NLV)\$(SUBSYS)\$(SUBSYS_RELATIVE)
SRC_NL_RELATIVE = mri\$(NLV)\$(SUBSYS)\utils


#------------------------------------------------------------------------------
#  Use built in macro "MAKEDIR" to establish "ROOT".  Due to case sensitivity
#
#  of nmake macro substitution, try all lowercase then all uppercase.  Fail if
#  current working directory path is mixed case.  This is a current nmake
#  limitation on macro substitutions but should not be a problem for most
#  people.  We are searching for a fix to this problem.
#------------------------------------------------------------------------------
# \src\mm\misc\mmpmcrt is an example of the path for a specific component
# in the tree structure. Please change the path as applied to your component.
# Refer to Nmake documentation, if you have any questions about MAKEDIR
#------------------------------------------------------------------------------

ROOT            = $(MAKEDIR:\src\jfs\utils\msgs=)
!if "$(ROOT)" == "$(MAKEDIR)"
ROOT            = $(MAKEDIR:\SRC\JFS\UTILS\MSGS=)
!endif
!if "$(ROOT)" == "$(MAKEDIR)"
!error Problem setting "ROOT"! Make sure current working directory is either all\
uppercase or all lowercase!
!endif

#-----------------------------------------------------------------
# Set up component/subsystem specific macros for use in header.mak
# This allows you establish a search order for header and lib paths.  A
# prefix would be a path that you would like to add to the beginning of
# the standard path which is set in header.mak.  A suffix tags onto the
# end of the standard path.
#-----------------------------------------------------------------


#CMP_HDR_PREFIX = <specify non-default path(s) for front of INCLUDE path>
#do NOT include a trailing "\" or ";" !!!
#

CMP_HDR_PREFIX  =

#CMP_HDR_SUFFIX = <specify non-default path(s) for back INCLUDE path>
CMP_HDR_SUFFIX  =

#CMP_LIB_PREFIX = <specify non-default path(s) for front of LIB path>
#CMP_LIB_SUFFIX = <specify non-default path(s) for back LIB path>

CMP_LIB_PREFIX  =
CMP_LIB_SUFFIX  =

#-----------------------------------------------------------------------
#INSTPATH = <specify the path for the files to be put in release tree>
#Instpath should be defined to get to the root of the install tree.
#For Example, suppose you want to place binaries in
#   \wpos\rel\386\retail\os2\toolkt\version1
# \wpos\rel\386\retail would be represented by $(R)
# Instpath = \os2\toolkt\version1
# Since REL=$(R)+$(INSTPATH), REL will become
# \wpos\rel\386\retail\os2\toolkt\version1
#-----------------------------------------------------------------------

#------ assume English is the default
!ifndef NLV
NLV = en
!endif
#EXTENDED_HDR_MSG =
#EXTENDED_HDR_MSG2=
#EXTENDED_HDR_HLP =
#EXTENDED_HDR_HLP2=

#------ Language definitions
!if "$(NLV)" == "br"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:055
IPFC_LANGUAGE   = /L:PTB
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L22,1
EXPLANATION=EXPLICAÄ«O
#Language          = Brazilian Portuguese "brasil"
#IBM Language code = PTB
#ISO 639           = pt  (locale=pt_BR)
#Country           = Brazil
#ISO 3166          = BR
#Country code      = 055
!endif

!if "$(NLV)" == "dk"
IPFC_CODEPAGE   = /C:850 /C:865
IPFC_COUNTRY    = /D:045
IPFC_LANGUAGE   = /L:DAN
MKMSGF_CODEPAGE = /P850 /P865
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L6,1
EXPLANATION=FORKLARING
#Language          = Danish "dansk"
#IBM Language code = DAN
#ISO 639           = da
#Country           = Denmark
#ISO 3166          = DK
#Country code      = 045
!endif

!if "$(NLV)" == "en"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:001
IPFC_LANGUAGE   = /L:ENU
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L9,1
EXPLANATION=EXPLANATION
#Language          = English
#IBM Language code = ENU
#ISO 639           = en
#Country           = United States
#ISO 3166          = US
#Country code      = 001
!endif

!if "$(NLV)" == "uk"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:044
IPFC_LANGUAGE   = /L:ENG
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L9,2
EXPLANATION=EXPLANATION
#Language          = English
#IBM Language code = ENG
#ISO 639           = en
#Country           = United Kingdom
#ISO 3166          = GB
#Country code      = 044
!endif

!if "$(NLV)" == "la"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:003
IPFC_LANGUAGE   = /L:ENG
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L9,2
EXPLANATION=EXPLANATION
#Language          = English
#IBM Language code = ENG        (ENG is really UK English)
#ISO 639           = en         (MkMsgf lang 9,2 is also UK)
#Country           = Latin America
#ISO 3166          = ??
#Country code      = 003
!endif

!if "$(NLV)" == "ap"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:099
IPFC_LANGUAGE   = /L:ENG
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L9,2
EXPLANATION=EXPLANATION
#Language          = English
#IBM Language code = ENG        (ENG is really UK English)
#ISO 639           = en         (MkMsgf lang 9,2 is also UK)
#Country           = Asian Pacific
#ISO 3166          = ??
#Country code      = 003
!endif

!if "$(NLV)" == "fr"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:033
IPFC_LANGUAGE   = /L:FRA
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L12,1
EXPLANATION=EXPLICATION
#Language          = French "francais"
#IBM Language code = FRA
#ISO 639           = fr
#Country           = France
#ISO 3166          = FR
#Country code      = 033
#...other variations...
#       FRB     12,2   Belgian French          Belgium
#       FRS     12,4   Swiss French            Switzerland
!endif

!if "$(NLV)" == "cf"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:002
IPFC_LANGUAGE   = /L:FRC
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L12,3
EXPLANATION=EXPLICATION
#Language          = Canadian French
#IBM Language code = FRC
#ISO 639           = fr
#Country           = Canada
#ISO 3166          = CA
#Country code      = 002
!endif

!if "$(NLV)" == "de"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:049
IPFC_LANGUAGE   = /L:DEU
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L7,1
EXPLANATION=ERKLéRUNG
#Language          = German "deutsch"
#IBM Language code = DEU
#ISO 639           = de
#Country           = Germany
#ISO 3166          = DE
#Country code      = 049
#...other variations...
#       DES      7,2   Swiss German            Switzerland
!endif

!if "$(NLV)" == "it"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:039
IPFC_LANGUAGE   = /L:ITA
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L16,1
EXPLANATION=SPIEGAZIONE
#Language          = Italian "italiano"
#IBM Language code = ITA
#ISO 639           = it
#Country           = Italy
#ISO 3166          = IT
#Country code      = 039
#...other variations...
#       ITS     16,2   Swiss Italian           Switzerland
!endif

!if "$(NLV)" == "nl"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:031
IPFC_LANGUAGE   = /L:NLD
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L19,1
EXPLANATION=VERKLARING
#Language          = Dutch == nederlands "nederlan"
#IBM Language code = NLD
#ISO 639           = nl
#Country           = Netherlands
#ISO 3166          = NL
#Country code      = 031
#...other variations...
#       NLB     19,2   Belgian Dutch           Belgium
!endif

!if "$(NLV)" == "no"
IPFC_CODEPAGE   = /C:850 /C:865
IPFC_COUNTRY    = /D:047
IPFC_LANGUAGE   = /L:NOR
MKMSGF_CODEPAGE = /P850 /P865
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L20,1
EXPLANATION=FORKLARING
#Language          = Norwegian (Bokmal) "norsk"
#IBM Language code = NOR
#ISO 639           = no
#Country           = Norway
#ISO 3166          = NO
#Country code      = 047
#...other variations...
#       NON     20,2   Norwegian - Nynorsk     Norway
!endif

!if "$(NLV)" == "pt"
IPFC_CODEPAGE   = /C:850 /C:860
IPFC_COUNTRY    = /D:351
IPFC_LANGUAGE   = /L:PTG
MKMSGF_CODEPAGE = /P850 /P860
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L22,2
EXPLANATION=EXPLICAÄ«O
#Language          = Portuguese "portugue"
#IBM Language code = PTG
#ISO 639           = pt
#Country           = Portugal
#ISO 3166          = PT
#Country code      = 351
!endif

!if "$(NLV)" == "es"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:034
IPFC_LANGUAGE   = /L:ESP
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L10,1
EXPLANATION=EXPLICACI‡N
#Language          = Spanish (Castilian) "espanol"
#IBM Language code = ESP
#ISO 639           = es
#Country           = Spain
#ISO 3166          = ES
#Country code      = 034
#...other variations...
#       CAT      3,1   Catalan                 Spain
#       ESM     10,2   Mexican Spanish         Mexico
!endif

!if "$(NLV)" == "ca"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:034
IPFC_LANGUAGE   = /L:CAT
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L32,1
EXPLANATION=EXPLICACI‡
#Language          = Catalan
#IBM Language code = CAT
#ISO 639           = ca
#Country           = Spain
#ISO 3166          = ES
#Country code      = 034
#...other variations...
#       CAT      3,1   Catalan                 Spain
#       ESM     10,2   Mexican Spanish         Mexico
!endif

!if "$(NLV)" == "fi"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:358
IPFC_LANGUAGE   = /L:FIN
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L11,1
EXPLANATION=SELITYS
#Language          = Finish "suomi"
#IBM Language code = FIN
#ISO 639           = fi
#Country           = Finland
#ISO 3166          = FI
#Country code      = 358
!endif

!if "$(NLV)" == "sv"
IPFC_CODEPAGE   = /C:850 /C:437
IPFC_COUNTRY    = /D:046
IPFC_LANGUAGE   = /L:SVE
MKMSGF_CODEPAGE = /P850 /P437
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L29,1
EXPLANATION=Fîrklaring
#Language          = Swedish "svenska"
#IBM Language code = SVE
#ISO 639           = sv
#Country           = Sweden
#ISO 3166          = SE
#Country code      = 046
!endif

!if "$(NLV)" == "ja" || "$(NLV)" == "jp"
IPFC_CODEPAGE   = /C:932
IPFC_COUNTRY    = /D:081
IPFC_LANGUAGE   = /L:JPN
MKMSGF_CODEPAGE = /P942         #?/P932
MKMSGF_COUNTRY  = /D081
MKMSGF_LANGUAGE = /L17,1
EXPLANATION="** âê‡ **"
#EXTENDED_HDR_MSG = $(O_NLV)\Jmsg.txt
#EXTENDED_HDR_MSG2= $(SYSTEM_NLV)\OSO001.437  /P437 /P850
#EXTENDED_HDR_HLP = $(O_NLV)\Jhelp.txt
#EXTENDED_HDR_HLP2= $(SYSTEM_NLV)\OSO001H.437 /P437 /P850
#j-ea01A 2 lines
#EXTENDED_CP        = 437
#Language          = japanese "nihongo"
#IBM Language code = JPN
#ISO 639           = ja
#Country           = Japan
#ISO 3166          = JP
#Country code      = 081
!endif

!if "$(NLV)" == "ko"
IPFC_CODEPAGE   = /C:949
IPFC_COUNTRY    = /D:082
IPFC_LANGUAGE   = /L:KOR
MKMSGF_CODEPAGE = /P949         #?/P934
MKMSGF_COUNTRY  = /D082
MKMSGF_LANGUAGE = /L18,1
EXPLANATION="º≥∏Ì:"
#EXTENDED_HDR_MSG = $(O_NLV)\Jmsg.txt
#EXTENDED_HDR_MSG2= $(SYSTEM_NLV)\OSO001.437  /P437 /P850
#EXTENDED_HDR_HLP = $(O_NLV)\Jhelp.txt
#EXTENDED_HDR_HLP2= $(SYSTEM_NLV)\OSO001H.437 /P437 /P850
#j-ea01A 2 lines
#EXTENDED_CP        = 437
#Language          = Korean "hangul"
#IBM Language code = KOR
#ISO 639           = ko
#Country           = Korea (South)
#ISO 3166          = KR
#Country code      = 082
!endif

!if "$(NLV)" == "tw"
# <TWN> Change country information
IPFC_CODEPAGE   = /C:950
IPFC_COUNTRY    = /D:088
IPFC_LANGUAGE   = /L:CHT
MKMSGF_CODEPAGE = /P950         #?/P938
MKMSGF_COUNTRY  = /D088
MKMSGF_LANGUAGE = /L4,1
EXPLANATION="∏—ƒ¿:"
#EXTENDED_HDR_MSG = $(O_NLV)\Jmsg.txt
#EXTENDED_HDR_MSG2= $(SYSTEM_NLV)\OSO001.437  /P437 /P850
#EXTENDED_HDR_HLP = $(O_NLV)\Jhelp.txt
#EXTENDED_HDR_HLP2= $(SYSTEM_NLV)\OSO001H.437 /P437 /P850
#EXTENDED_CP        = 437
#Language          = Chinese (traditional) "chungwen"
#IBM Language code = CHT
#ISO 639           = zh
#Country           = R.O.C. (Taiwan)
#ISO 3166          = TW
#Country code      = 088
!endif

#@GWSSI - LiL modify  10-14-96
!if "$(NLV)" == "cn"
IPFC_CODEPAGE   = /C:1381
IPFC_COUNTRY    = /D:086
IPFC_LANGUAGE   = /L:PRC
MKMSGF_CODEPAGE = /P1381        #?/P938
MKMSGF_COUNTRY  = /D086
MKMSGF_LANGUAGE = /L4,2
EXPLANATION="Ω‚ Õ:"
#EXTENDED_HDR_MSG = $(O_NLV)\Jmsg.txt
#EXTENDED_HDR_MSG2= $(SYSTEM_NLV)\OSO001.437  /P437 /P850
#EXTENDED_HDR_HLP = $(O_NLV)\Jhelp.txt
#EXTENDED_HDR_HLP2= $(SYSTEM_NLV)\OSO001H.437 /P437 /P850
#j-ea01A 2 lines
#EXTENDED_CP        = 437
#Language          = Chinese (simplified) "chongwon"
#IBM Language code = CHS
#ISO 639           = zh
#Country           = P.R.C. (Mainland China)
#ISO 3166          = CN
#Country code      = 086
!endif

!if "$(NLV)" == "cx"
IPFC_CODEPAGE   = /C:1386
IPFC_COUNTRY    = /D:086
IPFC_LANGUAGE   = /L:PRC
MKMSGF_CODEPAGE = /P1386        #?/P938
MKMSGF_COUNTRY  = /D086
MKMSGF_LANGUAGE = /L4,2
EXPLANATION="Ω‚ Õ£∫"
#EXTENDED_HDR_MSG = $(O_NLV)\Jmsg.txt
#EXTENDED_HDR_MSG2= $(SYSTEM_NLV)\OSO001.437  /P437 /P850
#EXTENDED_HDR_HLP = $(O_NLV)\Jhelp.txt
#EXTENDED_HDR_HLP2= $(SYSTEM_NLV)\OSO001H.437 /P437 /P850
#j-ea01A 2 lines
#EXTENDED_CP        = 437
#Language          = Chinese (simplified) "chongwon"
#IBM Language code = CHS
#ISO 639           = zh
#Country           = P.R.C. (Mainland China)
#ISO 3166          = CN
#Country code      = 086
!endif

!if "$(NLV)" == "ar"
IPFC_CODEPAGE   = /C:864
IPFC_COUNTRY    = /D:785
IPFC_LANGUAGE   = /L:ARA
MKMSGF_CODEPAGE = /P864
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L1,2
EXPLANATION=«‰‘—Õ
#Language          = Arabic "araby"
#IBM Language code = ARA
#ISO 639           = ar
#Country           = Arab countries
#ISO 3166          = AR
#Country code      = 785
!endif

!if "$(NLV)" == "he"
IPFC_CODEPAGE   = /C:862 /C:850
IPFC_COUNTRY    = /D:972
IPFC_LANGUAGE   = /L:HEB
MKMSGF_CODEPAGE = /P862 /P850
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L12,1
EXPLANATION=òÅëÑ
#EXPLANATION=ÑëÅò
#Language          = Hebrew
#IBM Language code = HEB
#ISO 639           = iw
#Country           = Israel
#ISO 3166          = IL
#Country code      = 972
!endif

!if "$(NLV)" == "bg"
IPFC_CODEPAGE   = /C:915 /C:850
IPFC_COUNTRY    = /D:359
IPFC_LANGUAGE   = /L:BGR
MKMSGF_CODEPAGE = /P915 /P850
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L2,1
EXPLANATION=æ±œ¡ΩµΩ∏µ
#Language          = Bulgarian
#IBM Language code = BGR
#ISO 639           = bg
#Country           = Bulgaria
#ISO 3166          = BG
#Country code      = 359
!endif

!if "$(NLV)" == "cz"
IPFC_CODEPAGE   = /C:852 /C:850
IPFC_COUNTRY    = /D:421
IPFC_LANGUAGE   = /L:CSY
MKMSGF_CODEPAGE = /P852 /P850
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L5,1
EXPLANATION=POPIS
#Language          = Czech
#IBM Language code = CSY
#ISO 639           = cz
#Country           = Czech Republic
#ISO 3166          = CZ
#Country code      = 421
!endif

!if "$(NLV)" == "ge"
IPFC_CODEPAGE   = /C:869 /C:813
IPFC_COUNTRY    = /D:030
IPFC_LANGUAGE   = /L:ENU
MKMSGF_CODEPAGE = /P869 /P813
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L8,1
EXPLANATION=EXPLANATION
#Language          = Greek English
#IBM Language code = ENU
#ISO 639           = el
#Country           = Greece
#ISO 3166          = GR
#Country code      = 030
!endif

!if "$(NLV)" == "gk"
IPFC_CODEPAGE   = /C:869 /C:813
IPFC_COUNTRY    = /D:030
IPFC_LANGUAGE   = /L:ELL
MKMSGF_CODEPAGE = /P869 /P813
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L8,1
EXPLANATION=≈Œ«√«”«
#Language          = Greek
#IBM Language code = ELL
#ISO 639           = el
#Country           = Greece
#ISO 3166          = GR
#Country code      = 030
!endif

!if "$(NLV)" == "hu"
IPFC_CODEPAGE   = /C:852 /C:850
IPFC_COUNTRY    = /D:036
IPFC_LANGUAGE   = /L:HUN
MKMSGF_CODEPAGE = /P852 /P850
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L13,1
EXPLANATION=MAGYARµZAT
#Language          = Hungarian
#IBM Language code = HUN
#ISO 639           = hu
#Country           = Hungary
#ISO 3166          = HU
#Country code      = 036
!endif

!if "$(NLV)" == "pl"
IPFC_CODEPAGE   = /C:852 /C:850
IPFC_COUNTRY    = /D:048
IPFC_LANGUAGE   = /L:PLK
MKMSGF_CODEPAGE = /P852 /P850
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L21,1
EXPLANATION=WYJAóNIENIE
#Language          = Polish
#IBM Language code = PLK
#ISO 639           = pl
#Country           = Poland
#ISO 3166          = PL
#Country code      = 048
!endif

!if "$(NLV)" == "ru"
IPFC_CODEPAGE   = /C:866 /C:850
IPFC_COUNTRY    = /D:007
IPFC_LANGUAGE   = /L:RUS
MKMSGF_CODEPAGE = /P866 /P850
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L25,1
EXPLANATION=éÅöüëçÖçàÖ
#Language          = Russian
#IBM Language code = RUS
#ISO 639           = ru
#Country           = Russia
#ISO 3166          = RU
#Country code      = 007
!endif

!if "$(NLV)" == "sl"
IPFC_CODEPAGE   = /C:852 /C:850
IPFC_COUNTRY    = /D:386
IPFC_LANGUAGE   = /L:SLO
MKMSGF_CODEPAGE = /P852 /P850
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L34,1
EXPLANATION=RAZLAGA
#Language          = Slovene
#IBM Language code = SLO
#ISO 639           = sl
#Country           = Slovenia
#ISO 3166          = SL
#Country code      = 386
!endif

!if "$(NLV)" == "th"
IPFC_CODEPAGE   = /C:874 /C:850
IPFC_COUNTRY    = /D:066
IPFC_LANGUAGE   = /L:THA
MKMSGF_CODEPAGE = /P874 /P850
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L30,1
EXPLANATION=EXPLANATION
#Language          = Thai
#IBM Language code = THA
#ISO 639           = th
#Country           = Thailand
#ISO 3166          = TH
#Country code      = 066
!endif

!if "$(NLV)" == "tr"
IPFC_CODEPAGE   = /C:857 /C:850
IPFC_COUNTRY    = /D:090
IPFC_LANGUAGE   = /L:TRK
MKMSGF_CODEPAGE = /P857 /P850
MKMSGF_COUNTRY  =
MKMSGF_LANGUAGE = /L31,1
EXPLANATION=AÄIKLAMA
#Language          = Turkish
#IBM Language code = TRK
#ISO 639           = tr
#Country           = Turkey
#ISO 3166          = TR
#Country code      = 090
!endif

#-----------------------------------------------------------------------------
# PUBLIC can be one of "ibmmk", "cps","os2".  Please define the below macros
# if you want to include the public files in a different order.  These macros
# would be used if you have a preference as to which directory gets searched
# first.  For example, you would like to search the OS2 header or include
# files before you search the ibmmk header or include files.  In this case,
# you would specify PUBLIC1 = ibmmk, PUBLIC2= IBMMK\pns, and PUBLIC3 = ibmmk\pns_runtime
# These macros are then used in header.mak to set the path order.
#------------------------------------------------------------------------------

PUBLIC1 = os2
#PUBLIC2 = cps
#PUBLIC3 = ibmmk
#PUBLIC3 = ibmmk\pns
#PUBLIC4 = ibmmk\pns_runtime


#------------------------------------------------------------------------------
# HOST_PROC and HOST_OS will be set in the environment by the build lab.
# Set them in your private builds.
# Please don't set these variables in the public build.
# HOST_PROC=386
# HOST_OS=OS2
#-----------------------------------------------------------------------------

!ifndef HOST_PROC
HOST_PROC=386
HOST_OS=OS2
!endif


#------------------------------------------------------------------------------
# header.mak is the public makefile that sets up all paths and options.  It
# should not be altered.  Additions or changes to header.mak for a component
# can be overwritten in this makefile.  Additions or changes to header.mak
# for an entire subsystem can be overwritten in subsys.mak
#-----------------------------------------------------------------------------

!include $(ROOT)\header.mak

#-------------------------------------------------------------------------
# subsys.mak is used as a way for each subsystem to override any settings
# that were set in header.mak.  It is owned by development.
#-------------------------------------------------------------------------

#!include $(ROOT)\src\$(SUBSYS)\common\subsys.mak

#-------------------------------------------------------------------------
# Insert any paths or macros that weren't defined in header.mak or
# subsys.mak here.
#--------------------------------------------------------------------------

#----------------------------------------------------------------
# Set up the list of suffixes, if the suffixes in header.mak are not
# sufficient.  Be sure to include the entire list.
#-----------------------------------------------------------------

.SUFFIXES:
.SUFFIXES: .asm .bas .c .cbl .cpp .cxx .def .dll .for .h .inc .lib .o .obj .pas .rc .dir

#------------------------------------------------------------------------
# Please define all compiler, assembler, and linker flags here.
# Use if statement to define the flags for each type of builds.
# See $(ROOT)\src\$(SUBSYS)\common\subsys.mak for defines.
#------------------------------------------------------------------------

#--------------------------------------------------------------------------
# If the Inference rules defined in header.mak are not sufficient, then
# define the inference rules necessary at this point.
# The following inference rules are just an example.
# You can delete this section if you don't need to add more inference rules.
#-------------------------------------------------------------------------


#***********************************************************************
#  Which Objects / Sources / Targets to make ?
#***********************************************************************

#------ Build all NLV languages, one at a time
all: $(NLV)
#        $(MAKE) -$(MAKEFLAGS) NLV=en    en    $(MAKEOPTS)
#        $(MAKE) -$(MAKEFLAGS) NLV=dk    dk    $(MAKEOPTS)
#        $(MAKE) -$(MAKEFLAGS) NLV=de    de    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=es    es    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=fi    fi    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=fr    fr    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=it    it    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=nl    nl    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=no    no    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=pt    pt    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=br    br    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=sv    sv    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=cf    cf    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=uk    uk    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=la    la    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=ap    ap    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=ja    ja    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=ko    ko    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=tw    tw    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=zh    zh    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=ar    ar    $(MAKEOPTS)
#       $(MAKE) -$(MAKEFLAGS) NLV=he    he    $(MAKEOPTS)

#------ This is the main target for each NLV language
$(NLV): $(REL_NLV)\os2\jfs.msg $(REL_NLV)\os2\jfsh.msg


#------ Extended header dependancies
#@IBM j-ea02D $(O_NLV)\Jmsg.txt $(O_NLV)\Jhelp.txt: en\jfs.txt
#@IBM j-ea02A 1 line
#$(O_NLV)\Jmsg.txt $(O_NLV)\Jhelp.txt: $(ENGLISH_BASE)\jfs.txt
#        echo ----[create Extended header files]----
#        copy $(ENGLISH_BASE)\jfs.txt \
#                $(O_NLV)\Jjfs.txt
#        msgsplit /S EXPLANATION $(O_NLV)\Jjfs.txt \
#                $(O_NLV)\Jhelp.txt $(O_NLV)\Jmsg.txt

#------ create the .MSG files in the target REL tree
$(REL_NLV)\os2\jfs.msg: $(O_NLV)\msg.tmp
	$(CREATE_PATH)
        echo ----[MkMsgf: $*]----
        echo $(O_NLV)\msg.tmp $@ $(MKMSGF_CODEPAGE) $(MKMSGF_COUNTRY) \
                $(MKMSGF_LANGUAGE) >$(O_NLV)\def.tmp
        mkmsgf @$(O_NLV)\def.tmp

$(REL_NLV)\os2\jfsh.msg: $(O_NLV)\hlp.tmp
	$(CREATE_PATH)
        echo ----[MkMsgf: $*]----
        echo $(O_NLV)\hlp.tmp $@ $(MKMSGF_CODEPAGE) $(MKMSGF_COUNTRY) \
                $(MKMSGF_LANGUAGE) >$(O_NLV)\defh.tmp
        mkmsgf @$(O_NLV)\defh.tmp

#------ split message text and help text into separate files
$(O_NLV)\msg.tmp $(O_NLV)\hlp.tmp: $(ROOT)\$(SRC_NL_RELATIVE)\jfs.txt
	$(CREATE_PATH)
        echo ----[MsgSplit: $*]----
        msgsplit /S $(EXPLANATION) /H "JFSnnnnx: " \
                 $(ROOT)\$(SRC_NL_RELATIVE)\jfs.txt \
                 $(O_NLV)\hlp.tmp $(O_NLV)\msg.tmp

#***************************************************************************
# Include Footer.mak at this point.
#***************************************************************************

!include $(ROOT)\footer.mak

#****************************************************************************
#  Dependency generation and Checking
#****************************************************************************

# $(O)\depend.mak:  $(O)\dephold
#         -$(CREATE_PATH)
#         -$(TOUCH) $(O)\depchk
#         -echo generate depend.mak
#          makedep -A -DPS -TP $(O) $(HCINC:-I=-I ) -o $@ *.c
#         -del $(O)\depchk
#
# $(O)\dephold:
#         -$(TOUCH) $@

clean:
        -echo y | del $(O)\*

###############################################################################
