
/*
 *@@sourcefile ring0api.h:
 *      public definitions for interfacing XWPSEC32.SYS, shared between
 *      ring 3 and ring 0.
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
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

#ifndef RING0API_HEADER_INCLUDED
    #define RING0API_HEADER_INCLUDED

    #pragma pack(1)

    /* ******************************************************************
     *
     *   IOCtl structures
     *
     ********************************************************************/

    /*
     *@@ TIMESTAMP:
     *      matches the first 20 bytes from the global infoseg
     *      declaration.
     *
     *@@added V1.0.1 (2003-01-10) [umoeller]
     */

    typedef struct _TIMESTAMP
    {
        ULONG   SIS_BigTime;    // Time from 1-1-1970 in seconds            4
        ULONG   SIS_MsCount;    // Freerunning milliseconds counter         8
        UCHAR   hours;          // Hours                                    9
        UCHAR   minutes;        // Minutes                                  10
        UCHAR   seconds;        // Seconds                                  11
        UCHAR   SIS_HunTime;    // Hundredths of seconds                    12
        USHORT  SIS_TimeZone;   // Timezone in min from GMT (Set to EST)    14
        USHORT  SIS_ClkIntrvl;  // Timer interval (units=0.0001 secs)       16
        UCHAR   day;            // Day-of-month (1-31)                      17
        UCHAR   month;          // Month (1-12)                             18
        USHORT  year;           // Year (>= 1980)                           20
    } TIMESTAMP, *PTIMESTAMP;

    /*
     *@@ CONTEXTINFO:
     *      matches the first 16 bytes from the local infoseg
     *      declaration.
     *
     *@@added V [umoeller]
     */

    typedef struct _CONTEXTINFO
    {
        USHORT  pid;            // Current process ID                       2
        USHORT  ppid;           // Process ID of parent                     4
        USHORT  prty;           // Current thread priority                  6
        USHORT  tid;            // Current thread ID                        8
        USHORT  LIS_CurScrnGrp; // Screengroup                              10
        UCHAR   LIS_ProcStatus; // Process status bits                      11
        UCHAR   LIS_fillbyte1;  // filler byte                              12
        USHORT  LIS_Fgnd;       // Current process is in foreground         14
        UCHAR   LIS_ProcType;   // Current process type                     15
        UCHAR   LIS_fillbyte2;  // filler byte                              16
    } CONTEXTINFO, *PCONTEXTINFO;

    /*
     *@@ EVENTLOGENTRY:
     *
     *@@added V1.0.1 (2003-01-10) [umoeller]
     */

    typedef struct _EVENTLOGENTRY
    {
        USHORT      cbStruct;       // total size of EVENTLOGENTRY plus data that follows
        USHORT      ulEventCode;    // EVENT_* code

        ULONG       idEvent;        // global event ID (continually raised with each event)
        ULONG       idContext;      // security context ID (actually a ring-0 pointer)

        CONTEXTINFO ctxt;           // 16 bytes

        TIMESTAMP   stamp;          // 20 bytes

        // event-specific data follows right afterwards;
        // as a convention, each data buffer has another
        // ULONG cbStruct as the first entry
    } EVENTLOGENTRY, *PEVENTLOGENTRY;

    #define LOGBUFSIZE          0xFFFF

    /*
     *@@ LOGBUF:
     *      logging buffer used in two contexts:
     *
     *      --  as the data packet used with the
     *          XWPSECIO_GETLOGBUF ioctl, which
     *          must be allocated LOGBUFSIZE (64K)
     *          in size by the ring-3 logging thread;
     *
     *      --  as a linked list of logging buffers
     *          in the driver.
     *
     *@@added V1.0.1 (2003-01-10) [umoeller]
     */

    typedef struct _LOGBUF
    {
        ULONG           cbUsed;             // total bytes used in the buffer
        struct _LOGBUF  *pNext;             // used only at ring 0 for linking bufs
        ULONG           idLogBuf;           // global log buf ID (continually raised)
        ULONG           cLogEntries;        // no. of EVENTLOGENTRY structs that follow

        // next follow cLogEntries EVENTLOGENTRY structs

    } LOGBUF, *PLOGBUF;

    /* ******************************************************************
     *
     *   Extra ring-0 error codes
     *
     ********************************************************************/

    /*
     *      OS/2 allows device drivers to return arbitrary error
     *      codes from ioctl request packets, as long as they fit
     *      into 8 bits. OS/2 then ORs these codes with 0xFF00
     *      before returning from DosDevIOCtl. So we define a few.
     */

    #define XWPERR_I24_NOT_ENOUGH_MEMORY         0x50

    #define XWPERR_I24_INVALID_ACL_FORMAT        0x51

    /* ******************************************************************
     *
     *   IOCtl functions
     *
     ********************************************************************/

    /*
     *@@ IOCTL_XWPSEC:
     *      category code for all XWPSEC32.SYS ioctl
     *      functions.
     */

    #define IOCTL_XWPSEC            0x8F

    /*
     *@@ SECIOREGISTER:
     *      structure to be passed with the XWPSECIO_REGISTER
     *      DosDevIOCtl function code to XWPSEC32.SYS.
     *
     *      With this structure, the ring-3 shell must
     *      pass down an array of process IDs that were
     *      running already at the time the shell was
     *      started. These PIDs will then be treated as
     *      trusted processes by the driver.
     */

    typedef struct _PROCESSLIST
    {
        ULONG       cbStruct;           // total bytes in structure
        USHORT      cTrusted;           // no. of PIDs in array
        USHORT      apidTrusted[1];     // array of PIDs
    } PROCESSLIST, *PPROCESSLIST;

    /*
     *@@ XWPSECIO_REGISTER:
     *      IOCTL_XWPSEC function code which enables
     *      local security.
     *
     *      After XWPShell has opened XWPSEC32.SYS,
     *      it must call this ioctl function with an
     *      array of PIDs that are currently running
     *      in a PROCESSLIST as the data packet. If
     *      this returns NO_ERROR, local security
     *      is enabled for the entire system.
     */

    #define XWPSECIO_REGISTER           0x50

    /*
     *@@ XWPSECIO_DEREGISTER:
     *      IOCTL_XWPSEC function code which disables
     *      local security.
     *
     *      After this, the system behaves as before
     *      XWPShell enabled local security.
     */

    #define XWPSECIO_DEREGISTER         0x51

    /*
     *@@ XWPSECIO_GETLOGBUF:
     *      ioctl function code for the ring-3 logging
     *      thread.
     *
     *      The shell can optionally start a thread to
     *      retrieve logging data from ring 0. This
     *      thread must do nothing but keep calling
     *      this ioctl function code with a pointer
     *      to a LOGBUFSIZE (64K) LOGBUF structure as
     *      the data packet.
     *
     *      On each ioctl, ring 0 will block the thread
     *      until logging data is available.
     *
     *      The buffer contains data only if NO_ERROR
     *      is returned, and LOGBUF.cbUsed bytes will
     *      have been filled. The pNext buffer contains
     *      a ring-0 pointer and must not be followed
     *      by the thread.
     *
     *      Ring 0 may return the following errors, ORed
     *      with 0xFF00:
     *
     *      --  ERROR_I24_INVALID_PARAMETER
     *
     *      --  XWPERR_I24_NOT_ENOUGH_MEMORY
     *
     *      --  ERROR_I24_GEN_FAILURE
     *
     *      --  ERROR_I24_CHAR_CALL_INTERRUPTED: ProcBlock
     *          returned with an error.
     */

    #define XWPSECIO_GETLOGBUF          0x52

    /*
     *@@ XWPSECIO_QUERYSTATUS:
     *      returns the current status of the driver.
     *
     *      Data packet must be a fixed-size XWPSECSTATUS
     *      structure.
     */

    #define XWPSECIO_QUERYSTATUS        0x53

    /*
     *@ SECIOCONTEXT:
     *      structure used with XWPSECIO_QUERYCONTEXT
     *      and XWPSECIO_SETCONTEXT.
     */

    typedef struct _SECIOCONTEXT
    {
        USHORT      pid;            // in: PID to query

        USHORT      cSubjects;      // in: size of aSubjects array (room in buffer)
                                    // out: no. of subject handles copied

        HXSUBJECT   aSubjects[1];   // out: array of cSubjects subject handles,
                                    // determining the permissions of this
                                    // process

    } SECIOCONTEXT, *PSECIOCONTEXT;

    /*
     *@@ XWPSECIO_QUERYCONTEXT:
     *      returns the security context of a given process.
     *
     *      Data packet must be a SECIOCONTEXT structure.
     *
     *      Ring 0 may return the following errors, ORed
     *      with 0xFF00:
     *
     *      --  ERROR_I24_INVALID_PARAMETER
     *
     *      --  ERROR_I24_BAD_UNIT: the given PID does not exist.
     *
     *      --  ERROR_I24_BAD_LENGTH: the given process has more
     *          subject handles than the caller has allocated in
     *          the input structure. Call again with a bigger
     *          aSubjects array.
     *
     *      --  ERROR_I24_CHAR_CALL_INTERRUPTED: ProcBlock
     *          returned with an error.
     */

    #define XWPSECIO_QUERYCONTEXT       0x54


    /*
     *@@ XWPSECIO_SETCONTEXT:
     *      sets a new security context for a given process.
     *
     *      Data packet must be a SECIOCONTEXT structure.
     *
     *      Ring 0 may return the following errors, ORed
     *      with 0xFF00:
     *
     *      --  ERROR_I24_INVALID_PARAMETER
     *
     *      --  ERROR_I24_BAD_UNIT: the given PID does not exist.
     *
     *      --  ERROR_I24_CHAR_CALL_INTERRUPTED: ProcBlock
     *          returned with an error.
     */

    #define XWPSECIO_SETCONTEXT         0x55

    /*
     *@@ XWPSECIO_SENDACLS:
     *      sends a new set of ACLs from the ring-3 shell
     *      to the driver.
     *
     *      Data packet must be a RING0BUF structure.
     *
     *      The driver will make a private copy of this
     *      structure at ring 0.
     *
     *      Ring 0 may return the following errors, ORed
     *      with 0xFF00:
     *
     *      --  XWPERR_I24_NOT_ENOUGH_MEMORY
     *
     *      --  XWPERR_I24_INVALID_ACL_FORMAT
     */

    #define XWPSECIO_SENDACLS           0x56

    /* ******************************************************************
     *
     *   Event logging codes
     *
     ********************************************************************/

    /*
     *@@ EVENT_OPEN_PRE:
     *      uses EVENTBUF_OPEN.
     */

    #define EVENT_OPEN_PRE              1

    /*
     *@@ EVENT_OPEN_POST:
     *      uses EVENTBUF_OPEN.
     */

    #define EVENT_OPEN_POST             2

    /*
     *@@ EVENT_LOADEROPEN:
     *      uses EVENTBUF_LOADEROPEN.
     */

    #define EVENT_LOADEROPEN            3

    /*
     *@@ EVENT_GETMODULE:
     *      uses EVENTBUF_FILENAME.
     */

    #define EVENT_GETMODULE             4

    /*
     *@@ EVENT_EXECPGM_PRE:
     *      uses EVENTBUF_FILENAME.
     */

    #define EVENT_EXECPGM_PRE           5

    /*
     *@@ EVENT_CREATEVDM_PRE:
     *      uses EVENTBUF_FILENAME.
     */

    #define EVENT_CREATEVDM_PRE         6

    /*
     *@@ EVENT_EXECPGM_ARGS:
     *      @@todo
     */

    #define EVENT_EXECPGM_ARGS          7

    #define EVENT_CREATEVDM_ARGS        8

    /*
     *@@ EVENT_EXECPGM_POST:
     *      uses EVENTBUF_FILENAME.
     */

    #define EVENT_EXECPGM_POST          9

    /*
     *@@ EVENT_CREATEVDM_POST:
     *      uses EVENTBUF_CLOSE (with SFN == RC).
     */

    #define EVENT_CREATEVDM_POST        10

    /*
     *@@ EVENT_CLOSE:
     *      uses EVENTBUF_CLOSE.
     */

    #define EVENT_CLOSE                 11

    /*
     *@@ EVENT_DELETE_PRE:
     *      uses EVENTBUF_FILENAME.
     */

    #define EVENT_DELETE_PRE            12

    /*
     *@@ EVENT_DELETE_POST:
     *      uses EVENTBUF_FILENAME.
     */

    #define EVENT_DELETE_POST           13

    /*
     *@@ EVENT_MAKEDIR:
     *      uses EVENTBUF_FILENAME.
     */

    #define EVENT_MAKEDIR               14

    /*
     *@@ EVENT_CHANGEDIR:
     *      uses EVENTBUF_FILENAME.
     */

    #define EVENT_CHANGEDIR             15

    /*
     *@@ EVENT_REMOVEDIR:
     *      uses EVENTBUF_FILENAME.
     */

    #define EVENT_REMOVEDIR             16

    /*
     *@@ EVENT_TRUSTEDPATH:
     *      uses EVENTBUF_CLOSE (really no data).
     */

    #define EVENT_TRUSTEDPATH           17

    /*
     *@@ EVENT_FINDFIRST:
     *      uses EVENTBUF_FILENAME
     */

    #define EVENT_FINDFIRST             18

    #define EVENT_FINDFIRST3X           19

    /*
     *@@ EVENT_FINDPERMISSIONS:
     *      uses EVENTBUF_FILENAME
     */

    #define EVENT_FINDPERMISSIONS       99

    /* ******************************************************************
     *
     *   Event-specific logging structures
     *
     ********************************************************************/

    /*
     *@@ EVENTBUF_FILENAME:
     *      event buffer used with EVENT_GETMODULE,
     *      EVENT_DELETE_PRE, EVENT_DELETE_POST,
     *      EVENT_EXECPGM_PRE, EVENT_EXECPGM_ARGS,
     *      and EVENT_EXECPGM_POST.
     *
     *@@added V1.0.1 (2003-01-13) [umoeller]
     */

    typedef struct _EVENTBUF_FILENAME
    {
        ULONG   rc;             // -- EVENT_LOADEROPEN, EVENT_GETMODULE, EVENT_DELETE_PRE:
                                //    authorization returned from callout,
                                //    either NO_ERROR or ERROR_ACCESS_DENIED
                                // -- EVENT_DELETE_POST: RC parameter passed in
                                // -- EVENT_EXECPGM_PRE: authorization returned from callout,
                                //    either NO_ERROR or ERROR_ACCESS_DENIED
                                // -- EVENT_EXECPGM_ARGS: always 0
                                // -- EXECPGM_POST: newly created process ID
                                // -- EVENT_FINDPERMISSIONS: testing permissions for a resource.
                                //    Always comes at least once: once with rc == -1 for "testing",
                                //    then with rc holding the ORed XWPACCESS_* flags if found.
        USHORT  fsRequired,     // XWPACCESS_* flags that were required
                fsGranted;      // XWPACCESS_* flags that this call was granted from ACLs
        ULONG   ulPathLen;      // length of szPath, excluding null terminator
        CHAR    szPath[1];      // filename
    } EVENTBUF_FILENAME, *PEVENTBUF_FILENAME;

    /*
     *@@ EVENTBUF_LOADEROPEN:
     *      event buffer used with EVENT_LOADEROPEN.
     *
     *@@added V1.0.1 (2003-01-13) [umoeller]
     */

    typedef struct _EVENTBUF_LOADEROPEN
    {
        ULONG   SFN;            // -- EVENT_LOADEROPEN: system file number;
                                // -- EVENT_GETMODULE, EVENT_DELETE_PRE, EVENT_DELETE_POST: not used
        ULONG   rc;             // -- EVENT_LOADEROPEN, EVENT_GETMODULE, EVENT_DELETE_PRE:
                                //    authorization returned from callout,
                                //    either NO_ERROR or ERROR_ACCESS_DENIED
                                // -- EVENT_DELETE_POST: RC parameter passed in
        ULONG   ulPathLen;      // length of szPath, excluding null terminator
        CHAR    szPath[1];      // filename
    } EVENTBUF_LOADEROPEN, *PEVENTBUF_LOADEROPEN;

    /*
     *@@ EVENTBUF_OPEN:
     *      event buffer used with EVENT_OPEN_PRE
     *      and EVENT_OPEN_POST.
     *
     *@@added V1.0.1 (2003-01-10) [umoeller]
     */

    typedef struct _EVENTBUF_OPEN
    {
        ULONG   fsOpenFlags;    // open flags
        ULONG   fsOpenMode;     // open mode
        ULONG   SFN;            // system file number
        ULONG   Action;         // OPEN_POST only: action taken
        APIRET  rc;             // -- with OPEN_PRE: authorization returned from callout,
                                //    either NO_ERROR or ERROR_ACCESS_DENIED
                                // -- with OPEN_POST: RC parameter passed in (if NO_ERROR, file is actually open now)
        union
        {
            struct
            {
                USHORT  fsRequired,     // XWPACCESS_* flags that were required
                        fsGranted;      // XWPACCESS_* flags that this call was granted from ACLs
            }   PRE;

            struct
            {
                LONG    cOpenFiles;     // successful OPEN_POST callouts for this security context
                                        // (including this one); valid ONLY if LOGBUF.idLogBuf != 0
            }   POST;
        };

        ULONG   ulPathLen;      // length of szPath, excluding null terminator
        CHAR    szPath[1];      // filename
    } EVENTBUF_OPEN, *PEVENTBUF_OPEN;

    /*
     *@@ EVENTBUF_CLOSE:
     *      event buffer used with EVENT_CLOSE.
     *
     *@@added V1.0.1 (2003-01-10) [umoeller]
     */

    typedef struct _EVENTBUF_CLOSE
    {
        ULONG   SFN;            // system file number
        LONG    cOpenFiles;     // successfull OPEN_POST callouts for this security context
                                // (excluding the file that was just closed);
                                // valid ONLY if LOGBUF.idLogBuf != 0
    } EVENTBUF_CLOSE, *PEVENTBUF_CLOSE;

    /* ******************************************************************
     *
     *   Ring-0 ACLs
     *
     ********************************************************************/

    /*
     *      Ring 0 interfaces required to be called from XWPShell:
     *
     *      --  Initialization: to be called exactly once when
     *          XWPShell starts up. This call enables local security
     *          and switches the driver into authorization mode:
     *          from then on, all system events are authenticated
     *          via the KPI callouts.
     *
     *          With this call, XWPShell must pass down an array of
     *          PIDs that were already running when XWPShell was
     *          started, including XWPShell itself. For these
     *          processes, the driver will create security contexts
     *          as trusted processes.
     *
     *          In addition, XWPShell sends down an array with all
     *          definitions of trusted processes so that the driver
     *          can create special security contexts for those.
     *
     *      --  Query security context: XWPShell needs to be able
     *          to retrieve the security context of a given PID
     *          from the driver to be able to authorize ring-3 API
     *          calls such as "create user" or changing permissions.
     *
     *      --  Set security context: changes the security context
     *          of an existing process. This is used by XWPShell
     *          to change its own context when the local user logs
     *          on. In addition, XWPShell will call this when a
     *          third party process has requested to change its
     *          context and this request was authenticated.
     *
     *      --  ACL table: Whenever subject handles are created or
     *          deleted, XWPShell needs to rebuild the system ACL
     *          table to contain the fresh subject handles and
     *          pass them all down to the driver.
     *
     *      --  Refresh process list: XWPShell needs to periodically
     *          call into the driver to pass it a list of processes
     *          that are currently running. Since there is no callout
     *          for when a process has terminated, the driver will
     *          end up with plenty of zombie PIDs after a while. This
     *          call will also be necessary before a user logs off
     *          after his processes have been terminated to make
     *          sure that subject handles are no longer in use.
     */

    /*
     *@@ ACCESS:
     *
     *@@added V1.0.1 (2003-01-05) [umoeller]
     */

    typedef struct _ACCESS
    {
        HXSUBJECT   hSubject;           // subject handle; this is -1 if an entry
                                        // exists for this resource but the user or
                                        // group is not currently in use (because no
                                        // such user is logged on)
        ULONG       flAccess;           // XWPACCESS_* flags
    } ACCESS, *PACCESS;

    #ifdef XWPTREE_INCLUDED

        /*
         *@@ RESOURCEACL:
         *      definition of a resource entry in the system access control
         *      list (ACL).
         *
         *      At ring 0, the driver has a list of all RESOURCEACL entries
         *      defined for the system. Each entry in turn has an array of
         *      ACCESS structs listing the subject handles for the resource,
         *      for example, defining that subject handle 1 (which could be
         *      representing a user) may read and write this resource, subject
         *      handle 2 (representing one of the groups the user belongs to)
         *      may execute, and so on.
         *
         *      There will only be one entry for any resource per subject.
         *      As a result, if the permissions for a resource are changed,
         *      the existing entry must be found and refreshed to avoid
         *      duplicates.
         *
         *      The global ACL table is build by XWPShell whenever it needs
         *      updating and passed down to the driver for future use. It
         *      will need rebuilding whenever a subject handle gets created
         *      or when access permissions are changed by an administrator.
         *
         *      The table will be build as follows by XWPShell:
         *
         *      1)  XWPShell loads the file defining the ACLs for the entire
         *          system.
         *
         *          For each definition in the file, it builds a RESOURCEACL
         *          entry. It checks the permissions defined for the resource
         *          in the file and sets up the array of ACCESS structures for
         *          the resource. If a permission was defined for a user for
         *          which a subject handle already exists (because the user
         *          is already logged on), that subject handle is stored.
         *          If a definition exists but none of the permissions apply
         *          to any of the current users (because those users are not
         *          logged on, or the groups are not in use yet), a dummy
         *          entry with a -1 subject handle is created to block access
         *          to the resource (see the algorithm description below).
         *
         *      2)  XWPShell then sends the system ACL list down to the driver.
         *
         *      During authorization, for any event, the driver first checks
         *      if a null ("root") subject handle exists in the process's
         *      security context. If so, access is granted unconditionally.
         *
         *      Otherwise, ACLs apply to all subdirectories too, unless a more
         *      specific ACL entry is encountered. In other words,
         *      the driver authorizes events bottom-up in the following order:
         *
         *      1)  It checks for whether an ACL entry for the given resource
         *          exists in the ACL table.
         *
         *          If any ACL entry was found for the resource, access is
         *          granted if any ACL entry allowed access for one of the
         *          subjects in the process's security context. Access is denied
         *          if ACL entries existed for the resource but none allowed access,
         *          which includes the "blocker" -1 entry described above.
         *
         *          In any case, the search stops if an ACL entry was found
         *          in the table, and access is either granted or denied.
         *
         *      2)  Only if no entry was found for the resource in any of the
         *          subject infos, we climb up to the parent directory and
         *          search all subject infos again. Go back to (1).
         *
         *      3)  After the root directory has been processed and still no
         *          entry exists, access is denied.
         *
         *      Examples:
         *
         *      User "dumbo" belongs to the groups "users" and "admins".
         *      The following ACLs are defined:
         *
         *      --  "users" may read "C:\DIR",
         *
         *      --  "admins" may read and write "C:\",
         *
         *      --  "admins" may create directories in "C:\DIR",
         *
         *      --  "otheruser" may read "C:\OTHERDIR".
         *
         *      Assuming that only "dumbo" is logged on presently and the following
         *      subject handles have thus been created:
         *
         *      --  1 for user "dumbo",
         *
         *      --  2 for group "users",
         *
         *      --  3 for group "admins",
         *
         *      the system ACL table will contain the following entries:
         *
         *      --  "C:\": 3 (group "admins") may read and write;
         *
         *      --  "C:\DIR": 2 (group "users") may read, 3 (group "admins) may
         *          create directories;
         *
         *      --  "C:\OTHERDIR": this will have a dummy -1 entry with no permissions
         *          because the only ACL defined is that user "otheruser" may read, and
         *          that user is not logged on.
         *
         *      1)  Assume a process running on behalf of "dumbo" wants to open
         *          C:\DIR\SUBDIR\TEXT.DOC for reading.
         *          Since the security context of "dumbo" has the three subject
         *          handles for user "dumbo" (1) and the groups "users" (2) and
         *          "admins" (3), the following happens:
         *
         *          a)  We check the system ACL table for "C:\DIR\SUBDIR\TEXT.DOC"
         *              and find no ACL entry.
         *
         *          b)  So we take the parent directory, "C:\DIR\SUBDIR",
         *              and again we find nothing.
         *
         *          c)  Taking the next parent, "C:\DIR\", we find the above two
         *              subject handles: since "users" (2) may read, and that is
         *              part of the security context, we grant access.
         *
         *      2)  Now assume that the same process wants to write the file back:
         *
         *          a)  Again, we find no ACL entries for "C:\DIR\SUBDIR\TEXT.DOC"
         *              or "C:\DIR\SUBDIR".
         *
         *          b)  Searching for "C:\DIR", we find that "users" (2) may only read,
         +              but not write. Also, "admins" (3) may create directories under
         *              "C:\DIR", which is not sufficient either. Since no other entries
         *              exist for "C:\DIR"  that would permit write, we deny access.
         *              That "admins" may write to "C:\" does not help since more
         *              specific entries exist for "C:\DIR".
         *
         *      3)  Now assume that the same process wants to create a new directory
         *          under "C:\DIR\SUBDIR".
         *
         *          a)  Again, we find no ACL entries for "C:\DIR\SUBDIR".
         *
         *          b)  Searching for "C:\DIR", we find that "users" may only read,
         *              which does not help. However, "admins" may create directories,
         *              so we grant access.
         *
         *      4)  Assume now that the process wants to create a new directory under
         *          "C:\OTHERDIR".
         *
         *          We find the ACL entry for "C:\OTHERDIR" and see only the -1
         *          subject handle (for user "otheruser", who's not logged on),
         *          and since no other permissions are set for us, we deny access.
         *
         *@@added V1.0.1 (2003-01-05) [umoeller]
         */

        typedef struct _RESOURCEACL
        {
            TREE        tree;               // tree management for ring 0
            ULONG       cbStruct;           // size of entire structure; this is
                                            //   sizeof(RESOURCEACL)
                                            // + cbName - 1
                                            // + cAccesses * sizeof(ACCESS)
            USHORT      cAccesses;          // no. of entries in array of ACCESS structs;
                                            // this comes right after szName, so its address
                                            // is szName + cbName
            USHORT      cbName;             // offset of array of ACCESS structs after szName
                                            // (includes null terminator and DWORD alignment
                                            // filler bytes)
            CHAR        szName[1];          // fully qualified filename of this resource
                                            // (zero-terminated)
        } RESOURCEACL, *PRESOURCEACL;

    #endif

    /*
     *@@ RING0BUF:
     *
     *@@added V1.0.1 (2003-01-05) [umoeller]
     */

    typedef struct _RING0BUF
    {
        ULONG       cbTotal;
        ULONG       cACLs;              // no. of RESOURCEACL structs (directly after this struct)
    } RING0BUF, *PRING0BUF;

    #pragma pack()

#endif

#if __cplusplus
}
#endif

