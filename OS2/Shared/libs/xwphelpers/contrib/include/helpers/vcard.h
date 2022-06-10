
/*
 *@@sourcefile vcard.h:
 *      header file for vcard.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #include <os2.h>
 *@@include #include "helpers\vcard.h"
 */

/*      Copyright (C) 2002 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef VCARD_HEADER_INCLUDED
    #define VCARD_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Strings
     *
     ********************************************************************/

    // DECLARE_VCARD_STRING is a handy macro which saves us from
    // keeping two string lists in both the .h and the .c file.
    // If this include file is included from the .c file, the
    // string is defined as a global variable. Otherwise
    // it is only declared as "extern" so other files can
    // see it.

    #ifdef INCLUDE_VCARD_PRIVATE
        #define DECLARE_VCARD_STRING(str, def) const char *str = def
    #else
        #define DECLARE_VCARD_STRING(str, def) extern const char *str;
    #endif

    // +//ISBN 1-887687-00-9::versit::PDI//vCard
    DECLARE_VCARD_STRING(VCF_FORMAL_IDENTIFIER,
                "+//ISBN 1-887687-00-9::versit::PDI//vCard");

    // common parameters
    DECLARE_VCARD_STRING(VCFPARAM_ENCODING, "ENCODING");
            // either BASE64, QUOTED-PRINTABLE, 8BIT
    DECLARE_VCARD_STRING(VCFPARAM_CHARSET, "CHARSET");
            // ADR;CHARSET=ISO-8859-8:...
    DECLARE_VCARD_STRING(VCFPARAM_LANGUAGE, "LANGUAGE");
            // ADR;LANGUAGE=fr-CA:...
    DECLARE_VCARD_STRING(VCFPARAM_VALUE, "VALUE");
            // INLINE (default)
            // CONTENT-ID or CID (in separate MIME entity)
            // URL

    // formatted name (what is displayed)
    DECLARE_VCARD_STRING(VCFPROP_FN, "FN");

    // name
    DECLARE_VCARD_STRING(VCFPROP_N, "N");
            #define VCF_VALUE_INDEX_N_FAMILY       0
            #define VCF_VALUE_INDEX_N_GIVEN        1       // first name
            #define VCF_VALUE_INDEX_N_ADDITIONAL   2       // additional (middle) names
            #define VCF_VALUE_INDEX_N_PREFIX       3       // prefix (Dr.)
            #define VCF_VALUE_INDEX_N_SUFFIX       4       // suffix

    // PHOTO
    DECLARE_VCARD_STRING(VCFPROP_PHOTO, "PHOTO");
        // parameters:
        DECLARE_VCARD_STRING(VCFPARAM_TYPE, "TYPE");
                     // one of GIF, CGM, WMF, BMP, MET, PMB, DIB,
                     // PICT, TIFF, PS, PDF, JPEG, MPEG, MPEG2,
                     // AVI, QTIME

    // birthday
    DECLARE_VCARD_STRING(VCFPROP_BDAY, "BDAY");

    // delivery address
    DECLARE_VCARD_STRING(VCFPROP_ADR, "ADR");
        // parameters:
        // DECLARE_VCARD_STRING(VCFPARAM_TYPE, "TYPE");
                     // one or several of:
        DECLARE_VCARD_STRING(VCF_TYPE_ADR_DOM, "DOM"); // domestic
        DECLARE_VCARD_STRING(VCF_TYPE_ADR_INTL, "INTL"); // international address
        DECLARE_VCARD_STRING(VCF_TYPE_ADR_POSTAL, "POSTAL"); // postal delivery address
        DECLARE_VCARD_STRING(VCF_TYPE_ADR_PARCEL, "PARCEL"); // parcel delivery address
        DECLARE_VCARD_STRING(VCF_TYPE_ADR_HOME, "HOME"); // home delivery address
        DECLARE_VCARD_STRING(VCF_TYPE_ADR_WORK, "WORK"); // work  delivery address
                    // default is INTL, WORK, POSTAL, PARCEL

            // ADR;DOM;HOME:P.O. Box 101;Suite 101;123 Main Street;Any Town;CA;91921-1234;
            #define VCF_VALUE_INDEX_ADR_POSTOFFICE 0       // post office, e.g. "P.O. Box 101"
            #define VCF_VALUE_INDEX_ADR_EXTENDED   1       // "Suite 101"
            #define VCF_VALUE_INDEX_ADR_STREET     2
            #define VCF_VALUE_INDEX_ADR_LOCALITY   3       // town
            #define VCF_VALUE_INDEX_ADR_REGION     4       // CA
            #define VCF_VALUE_INDEX_ADR_POSTALCODE 5       // zip code
            #define VCF_VALUE_INDEX_ADR_COUNTRY    6       // country

    // formatted delivery label
    DECLARE_VCARD_STRING(VCFPROP_LABEL, "LABEL");
        // parameters: TYPE as with ADR

    // telephone
    DECLARE_VCARD_STRING(VCFPROP_TEL, "TEL");
        // parameters:
        // DECLARE_VCARD_STRING(VCFPARAM_TYPE, "TYPE");
                     // one or several of:
        DECLARE_VCARD_STRING(VCF_TYPE_TEL_PREF, "PREF");        // preferred no.
        DECLARE_VCARD_STRING(VCF_TYPE_TEL_WORK, "WORK");        // work no.
        DECLARE_VCARD_STRING(VCF_TYPE_TEL_HOME, "HOME");        // home no.
        DECLARE_VCARD_STRING(VCF_TYPE_TEL_VOICE, "VOICE");      // voice no. (default)
        DECLARE_VCARD_STRING(VCF_TYPE_TEL_FAX, "FAX");          // fax no.
        DECLARE_VCARD_STRING(VCF_TYPE_TEL_MSG, "MSG");          // messaging service
        DECLARE_VCARD_STRING(VCF_TYPE_TEL_CELL, "CELL");        // cell phone
        DECLARE_VCARD_STRING(VCF_TYPE_TEL_PAGER, "PAGER");      // pager
        DECLARE_VCARD_STRING(VCF_TYPE_TEL_BBS, "BBS");          // bulletin board service
        DECLARE_VCARD_STRING(VCF_TYPE_TEL_MODEM, "MODEM");      // modem
        DECLARE_VCARD_STRING(VCF_TYPE_TEL_CAR, "CAR");          // car phone
        DECLARE_VCARD_STRING(VCF_TYPE_TEL_ISDN, "ISDN");        // isdn
        DECLARE_VCARD_STRING(VCF_TYPE_TEL_VIDEO, "VIDEO");      // video phone
                    // default is VOICE only

    // EMAIL
    DECLARE_VCARD_STRING(VCFPROP_EMAIL, "EMAIL");
        // parameters:
        // DECLARE_VCARD_STRING(VCFPARAM_TYPE, "TYPE");
                     // one or several of:
        DECLARE_VCARD_STRING(VCF_TYPE_EMAIL_AOL, "AOL");
        DECLARE_VCARD_STRING(VCF_TYPE_EMAIL_APPLELINK, "AppleLink");
        DECLARE_VCARD_STRING(VCF_TYPE_EMAIL_ATTMAIL, "ATTMail");
        DECLARE_VCARD_STRING(VCF_TYPE_EMAIL_CIS, "CIS");
        DECLARE_VCARD_STRING(VCF_TYPE_EMAIL_EWORLD, "eWorld");
        DECLARE_VCARD_STRING(VCF_TYPE_EMAIL_INTERNET, "INTERNET");
        DECLARE_VCARD_STRING(VCF_TYPE_EMAIL_IBMMAIL, "IBMMail");
        DECLARE_VCARD_STRING(VCF_TYPE_EMAIL_MCIMAIL, "MCIMail");
        DECLARE_VCARD_STRING(VCF_TYPE_EMAIL_POWERSHARE, "POWERSHARE");
        DECLARE_VCARD_STRING(VCF_TYPE_EMAIL_PRODIGY, "PRODIGY");
        DECLARE_VCARD_STRING(VCF_TYPE_EMAIL_TLX, "TLX");
        DECLARE_VCARD_STRING(VCF_TYPE_EMAIL_X400, "X400");

    // MAILER software
    DECLARE_VCARD_STRING(VCFPROP_MAILER, "MAILER");

    // timezone
    DECLARE_VCARD_STRING(VCFPROP_TZ, "TZ");

    // geographic position
    DECLARE_VCARD_STRING(VCFPROP_GEO, "GEO");

    // job title
    DECLARE_VCARD_STRING(VCFPROP_TITLE, "TITLE");

    // business role
    DECLARE_VCARD_STRING(VCFPROP_ROLE, "ROLE");

    // company logo
    DECLARE_VCARD_STRING(VCFPROP_LOGO, "LOGO");
        // parameters: TYPE as with PHOTO

    // organization name
    DECLARE_VCARD_STRING(VCFPROP_ORG, "ORG");
            #define VCF_VALUE_INDEX_ORG_NAME       0        // organization name
            #define VCF_VALUE_INDEX_ORG_UNIT       1        // organization unit (division)

    // comment
    DECLARE_VCARD_STRING(VCFPROP_NOTE, "NOTE");

    // when vCard was last modified
    DECLARE_VCARD_STRING(VCFPROP_REV, "REV");
            // ISO 8601
            // format: REV:19951031T222710
            // or      REV:1995-10-31T22:27:10Z

    // sound data
    DECLARE_VCARD_STRING(VCFPROP_SOUND, "SOUND");
        // DECLARE_VCARD_STRING(VCFPARAM_TYPE, "TYPE");
                     // one of WAVE, PCM, AIFF


    // where to find up-to-date information
    DECLARE_VCARD_STRING(VCFPROP_URL, "URL");

    // unique vCard identifier
    DECLARE_VCARD_STRING(VCFPROP_UID, "UID");

    // vCard version info (2.1)
    DECLARE_VCARD_STRING(VCFPROP_VERSION, "VERSION");
            // must be "2.1"

    // public key
    DECLARE_VCARD_STRING(VCFPROP_KEY, "KEY");
        // DECLARE_VCARD_STRING(VCFPARAM_TYPE, "TYPE");
        //          one of:
        // --   X509
        // --   PGP

    // X-*: anything starting with X- is an extension

    /* ******************************************************************
     *
     *   Declarations
     *
     ********************************************************************/

    #ifdef INCLUDE_VCARD_ALL

        /*
         *@@ VCFPROPERTY:
         *
         */

        typedef struct _VCFPROPERTY
        {
            XSTRING     strProperty;

            PLINKLIST   pllSubList;             // if != NULL, nested list of
                                                // PROPERTY structs

            ULONG       cParameters;
            PXSTRING    pastrParameters;        // array of cParameters XSTRINGs

            ULONG       cValues;
            PXSTRING    pastrValues;            // array of cValues XSTRINGs

        } VCFPROPERTY, *PVCFPROPERTY;

    #endif

    /*
     *@@ VCDATE:
     *
     */

    typedef struct _VCDATE
    {
        UCHAR       ucDay,
                    ucMonth;
        USHORT      usYear;
    } VCDATE, *PVCDATE;

    /*
     *@@ VCTIME:
     *
     */

    typedef struct _VCTIME
    {
        UCHAR       ucHour,
                    ucMinutes,
                    ucSeconds;
    } VCTIME, *PVCTIME;

    /*
     *@@ VCADDRESS:
     *
     */

    typedef struct _VCADDRESS
    {
        PCSZ        apcszAddress[7];
         /* #define VCF_VALUE_INDEX_ADR_POSTOFFICE 0       // post office, e.g. "P.O. Box 101"
            #define VCF_VALUE_INDEX_ADR_EXTENDED   1       // "Suite 101"
            #define VCF_VALUE_INDEX_ADR_STREET     2
            #define VCF_VALUE_INDEX_ADR_LOCALITY   3       // town
            #define VCF_VALUE_INDEX_ADR_REGION     4       // CA
            #define VCF_VALUE_INDEX_ADR_POSTALCODE 5       // zip code
            #define VCF_VALUE_INDEX_ADR_COUNTRY    6       // country
         */

        ULONG       fl;
                #define VCF_ADDRFL_DOM          0x0001  // domestic
                #define VCF_ADDRFL_INTL         0x0002  // international address
                #define VCF_ADDRFL_POSTAL       0x0004  // postal delivery address
                #define VCF_ADDRFL_PARCEL       0x0008  // parcel delivery address
                #define VCF_ADDRFL_HOME         0x0010  // home delivery address
                #define VCF_ADDRFL_WORK         0x0020  // work  delivery address
                    // default is INTL, WORK, POSTAL, PARCEL

    } VCADDRESS, *PVCADDRESS;

    /*
     *@@ VCLABEL:
     *
     */

    typedef struct _VCLABEL
    {
        PCSZ        pcszLabel;
        ULONG       fl;         // VCF_ADDRFL_* flags
    } VCLABEL, *PVCLABEL;

    /*
     *@@ VCPHONE:
     *
     */

    typedef struct _VCPHONE
    {
        PCSZ        pcszNumber;
        ULONG       fl;
                #define VCF_PHONEFL_PREF        0x0001  // preferred no.
                #define VCF_PHONEFL_WORK        0x0002  // work no.
                #define VCF_PHONEFL_HOME        0x0004  // home no.
                #define VCF_PHONEFL_VOICE       0x0008  // voice no. (default)
                #define VCF_PHONEFL_FAX         0x0010  // fax no.
                #define VCF_PHONEFL_MSG         0x0020  // messaging service
                #define VCF_PHONEFL_CELL        0x0040  // cell phone
                #define VCF_PHONEFL_PAGER       0x0080  // pager
                #define VCF_PHONEFL_BBS         0x0100  // bulletin board service
                #define VCF_PHONEFL_MODEM       0x0200  // modem
                #define VCF_PHONEFL_CAR         0x0400  // car phone
                #define VCF_PHONEFL_ISDN        0x0800  // isdn
                #define VCF_PHONEFL_VIDEO       0x1000  // video phone
    } VCPHONE, *PVCPHONE;

    /*
     *@@ VCARD:
     *
     *@@added V0.9.16 (2002-02-02) [umoeller]
     */

    typedef struct _VCARD
    {
        PCSZ        pcszFormattedName;

        PCSZ        apcszName[5];
            // VCF_VALUE_INDEX_N_FAMILY       0
            // VCF_VALUE_INDEX_N_GIVEN        1       // first name
            // VCF_VALUE_INDEX_N_ADDITIONAL   2       // additional (middle) names
            // VCF_VALUE_INDEX_N_PREFIX       3       // prefix (Dr.)
            // VCF_VALUE_INDEX_N_SUFFIX       4       // suffix

        VCDATE      vcdBirthday;

        ULONG       cDeliveryAddresses;
        PVCADDRESS  paDeliveryAddresses;    // array of cAddresses VCADDRESS structs

        ULONG       cLabels;
        PVCLABEL    paLabels;

        ULONG       cPhones;
        PVCPHONE    paPhones;

        PCSZ        pcszEmail;

        PCSZ        pcszTimeZone;

        PCSZ        pcszJobTitle;

        PCSZ        pcszBusinessRole;

        PCSZ        pcszOrganizationName,
                    pcszOrganizationUnit;

        PCSZ        pcszURL;

        VCDATE      vcdRevision;
        VCTIME      vctRevision;

        // private linked list of vCard properties
        PLINKLIST   pll;

    } VCARD, *PVCARD;

    APIRET vcfRead(PCSZ pcszFilename,
                   PVCARD *ppvCard);

    APIRET vcfFree(PVCARD *pphvCard);

#endif

#if __cplusplus
}
#endif

