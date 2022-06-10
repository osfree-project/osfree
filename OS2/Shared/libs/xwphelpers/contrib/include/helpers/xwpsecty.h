
/*
 *@@sourcefile xwpsecty.h:
 *      public declarations which are shared between the various ring-3
 *      parts of XWorkplace Security (XWPSec).
 *
 *      XWPSec consists of the following modules:
 *
 *      1)  XWPSHELL.EXE, which is to be installed through RUNWORKPLACE
 *          in CONFIG.SYS and functions as both a WPS starter and a
 *          daemon for managing the security driver;
 *
 *      2)  XWPSEC32.SYS, which is a ring-0 driver performing
 *          authentification for OS/2 APIs like DosOpen, DosMove etc.
 *          through SES callouts.
 *
 *      Note that Security Enabling Services (SES) is NOT used
 *      for user logon and authentication. In other words, this
 *      program serves neither as an SES System Logon Authority
 *      (SLA) nor a User Identification Authority (UIA). I have
 *      found SES Logon Shell Services (LSS) too buggy to be useful,
 *      but maybe I was too dumb to find out how to use them. But
 *      then again, these APIs are too complicated and error-prone
 *      to be used by ordinary humans.
 *
 *      So instead, I have rewritten the LSS functionality for
 *      XWPShell. SES is only used for kernel interfaces (KPIs)
 *      in XWPSEC32.SYS. As a result, the SECURE.SYS text file
 *      in \OS2\SECURITY\SESDB is not used either.
 *
 *
 *      <B>Concepts:</B>
 *
 *      To implement XWPSec, I have reviewed the concepts of Linux,
 *      SES, and OS/2 LAN Server (Warp Server for e-Business).
 *      See subjects.c, users.c, and userdb.c for a discussion of
 *      security concepts which came out of this review.
 *
 *      As a summary, XWPSec follows the SES security models, with a
 *      few simplifications. To make processing more efficient, user
 *      IDs and group IDs have been added (like Linux uid and gid) to
 *      allow looking up data based on numbers instead of names.
 *
 *      <B>Recommended reading:</B>
 *
 *      -- "SES Developer's Guide" (IBM SG244668 redbook). This
 *         sucks, but it's the only source for SES information.
 *
 *      -- "Network Administrator Tasks" (BOOKS\A3AA3MST.INF
 *         on a WSeB installation). A very good book.
 *
 *      -- Linux manpages for group(5) and passwd(5); Linux info
 *         page for chmod (follow the "File permissions" node).
 *
 *
 *      <b>XWPSec Authentication Model</b>
 *
 *      Basically, there are two competing models of access permissions
 *      in the computing world, the POSIX model and the Windows/OS/2
 *      model. Since the POSIX model can be implemented as a subset of
 *      the OS/2 model, XWPSec follows the OS/2 model, with very
 *      fine-grained access permissions and different ACLs per resource.
 *
 *      XWPSec implements the usual stuff like users, groups, and
 *      privileged processes with increased access rights. As opposed
 *      to the POSIX model, group membership is an M:N relation, meaning
 *      that any user can be a member of an infinite number of groups.
 *      Also, while the POSIX model only allows controlling permissions
 *      for the owner, group, and "world" (rwxrwxrwx model), XWPSec has
 *      real ACLs which are unlimited in size per resource.
 *
 *      To implement this efficiently, XWPSec uses "subject handles",
 *      which is a concept borrowed from SES. However, XWPSec does not
 *      use the SES APIs for creating and managing those, but implements
 *      this functionality itself.
 *
 *      For authorizing events, XWPSec uses the XWPSUBJECTINFO and
 *      XWPSECURITYCONTEXT structures, plus an array of RESOURCEACL
 *      structs that forms the global system ACL table shared with
 *      the ring-0 device driver.
 *
 *
 *      <b>Logon</b>
 *
 *      XWPShell maintains a list of users that are currently
 *      logged on. There can be an infinite number of logged-on
 *      users. However, one user logon is special and is called
 *      the "local" logon: that user owns the shell, most
 *      importantly PM and the Workplace Shell, and processes
 *      started via the shell run on behalf of that user.
 *
 *      When a user logs on, XWPShell authenticates the credentials
 *      (normally, via user name and password), and creates one
 *      subject handle for that user plus one subject handle for
 *      every group that the user belongs to. XWPShell then rebuilds
 *      the system ACL table and sends it to the driver so it can
 *      perform authentication.
 *
 *      Logon can thus happen in two situations. Most frequently,
 *      the local user logs on via the logon dialog when XWPShell
 *      starts up together with the system. However, XWPShell also
 *      allows any application to change its own security context
 *      be re-logging on with different credentials.
 *
 *      This is the typical sequence of events when XWPShell is running:
 *
 *      1)  When XWPShell starts up, it scans the process list to figure
 *          out all processes that are already running because of system
 *          startup. It then contacts the ring-0 driver, passing it an
 *          array of these PIDs, and thus enables local security. The
 *          driver then creates root security contexts for those processes.
 *
 *          From that point on, all system events are authenticated until
 *          the system is shut down.
 *
 *          (Before that, the driver does not deny access in order not to
 *          hickup the startup sequence.)
 *
 *      2)  XWPShell displays the logon dialog.
 *
 *      3)  After the user has entered his user name and password,
 *          XWPShell authenticates the user against the user database.
 *
 *      4)  If the user was authenticated, XWPSec creates subjects
 *          for the user and his groups. With each subject creation,
 *          XWPShell rebuilds the system ACL list with the subject
 *          handles and sends them down to ring 0 so that the driver
 *          can perform authorization (see RESOURCEACL for details).
 *
 *      5)  XWPShell sets the security context of itself (XWPSHELL.EXE)
 *          to contain these subject handles by calling a ring-0 driver
 *          API. Whoever owns XWPSHELL.EXE is, by definition, the "local"
 *          user.
 *
 *      6)  XWPShell then changes the user profile (OS2.INI) via PrfReset,
 *          builds an environment for the Workplace Shell, and starts
 *          PMSHELL.EXE so that the WPS starts up with the user's Desktop.
 *          The WPS is started as a child of XWPSHELL.EXE and thus
 *          inherits its security context. Since the WPS uses WinStartApp
 *          to start applications, those will inherit the local user's
 *          security context as well.
 *
 *          The following environment variables are set:
 *
 *          --  USER is set to the user name;
 *          --  USERID is set to the user ID (uid);
 *          --  HOME is set to the user's home directory;
 *          --  OS2_INI is set to the user's OS2.INI file. This
 *              does not affect the profile (which has been
 *              changed using PrfReset before), but is still
 *              changed in case any program references this
 *              variable.
 *
 *          All processes started by the WPS will inherit this
 *          environment since the WPS always copies its environment
 *          to child processes.
 *
 *      7)  From then on, all system events will be authorized against
 *          the local user's security context, which contains the subject
 *          handles of the local user and his groups. For example, when
 *          an application opens a file, the driver will authorize this
 *          event against the ACLs stored in the user's subject infos
 *          and permit the operation or deny access.
 *
 *      8)  When the user logs off via the new menu item in the Desktop's
 *          context menu (implemented by XWorkplace), XWorkplace will then
 *          terminate all processes running on behalf of this user and
 *          terminate the WPS process via DosExit. It sets a special flag
 *          in shared memory to indicate to XWPShell that this was not
 *          just a WPS restart (or trap), but a logoff event to prevent
 *          XWPShell from restarting the WPS immediately.
 *
 *      9)  When XWPShell then realizes that the WPS has terminated and
 *          finds this flag, it logs off the user. Logging off implies
 *          again checking that no processes are running on behalf of
 *          the local user any more, changing the security context of
 *          XWPSHELL.EXE back to "noone", and deleting the subject
 *          handles that were created in (4.). Go back to (2.).
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_DOSSEMAPHORES
 *@@include #include <os2.h>
 *@@include #include "helpers\xwpsecty.h"
 */

/*
 *      Copyright (C) 2000-2002 Ulrich M”ller.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef XWPSECTY_HEADER_INCLUDED
    #define XWPSECTY_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Global constants
     *
     ********************************************************************/

    #define XWPSEC_NAMELEN              32
    #define XWPSEC_FULLNAMELEN          80

    /* ******************************************************************
     *
     *   Subjects and security contexts
     *
     ********************************************************************/

    typedef unsigned long HXSUBJECT;

    typedef long XWPSECID;

    /*
     *@@ XWPSUBJECTINFO:
     *      describes a subject.
     *
     *      A subject represents either a user, a group, or a
     *      trusted process associated with an access control
     *      list.
     *
     *      Subject handles allow for reusing ACLs efficiently
     *      when multiple users are logged on and also isolate
     *      the ring-0 driver from user and group management.
     *
     *      Subject handle 0 is special and means "no restrictions".
     *      Only the superuser ("root") will have a subject handle
     *      of 0, regardless of the groups he's a member of. All
     *      other users will have nonzero subject handles.
     *
     *      When a user logs on, subject handles get created
     *      that represent the access permissions of that user.
     *      Every running process has a list of subject handles
     *      associated with it (the security context), the sum
     *      of which allow for authorizing events. The driver
     *      only sees subject handles, not users or groups.
     *      However, the user and group IDs are listed in the
     *      subject info structure to allow the driver to audit
     *      events.
     *
     *      Subject handles are created by XWPShell (ring 3)
     *      when users log on and then sent down to the ring-0
     *      driver together with a new system ACL list for future
     *      use.
     *
     *      Comparison with other concepts:
     *
     *      --  The concept of a "subject" has been taken from
     *          SES. However, with SES, a subject is much more
     *          complicated and not really well explained. The
     *          SES documentation sucks anyway.
     *
     *      --  Warp (LAN) Server bases ACL entries on users and
     *          groups also, however without calling them "subjects".
     *          However, if I understood that right, LAN server can
     *          define an infinite number of ACL entries for each
     *          resource.
     *
     *      --  Linux does not have the notion of a subject.
     *          Instead, all access rights are determined from
     *          the user id (uid) and group id (gid), apparently.
     *
     *      --  Java seems to have something similar; it bases
     *          ACL entries on "principals", where a principal
     *          can either be a user or a group. See the API
     *          docs for java.security.acl.Acl.
     *
     *@@added V0.9.5 (2000-09-23) [umoeller]
     */

    typedef struct _XWPSUBJECTINFO
    {
        HXSUBJECT   hSubject;
                // handle of this subject (unique),
                // simply raised with each subject creation
        XWPSECID    id;
                // ID related to this subject;
                // -- for a user subject: the user id (uid); 0 if root
                // -- for a group subject: the group id (gid)
                // -- for a process subject: the process id (pid)
        BYTE        bType;
                // one of:
                #define SUBJ_USER       1
                #define SUBJ_GROUP      2
                #define SUBJ_PROCESS    3

        ULONG       cUsage;
                // usage count (to allow for reuse)

    } XWPSUBJECTINFO, *PXWPSUBJECTINFO;

    /*
     *@@ XWPSECURITYCONTEXTCORE:
     *      describes the security context for a process.
     *
     *      For each process on the system, one security context
     *      defines its permissions via subject handles. Security
     *      contexts are created in ring 0 in the following
     *      situations:
     *
     *      --  Via the DosExecPgm callout, when a new process is
     *          started. The driver then creates a security context
     *          with the same subject handles as the new process's
     *          parent process. This way process permissions inherit
     *          from each other.
     *
     *      --  Via a ring-0 API that gets called by XWPShell to
     *          create security contexts for processes that are
     *          already running at startup, including XWPShell
     *          itself.
     *
     *      Each subject handle in the context represents either a user,
     *      a group, or a trusted process. A subject handle of "0" is
     *      special and indicates root access. If such a subject handle
     *      is found, the driver grants full access and does not reject
     *      anything.
     *
     *      As a result, the following are typical security contexts:
     *
     *      --  A process running on behalf of a certain user will
     *          usually contain at least two subject handles, one
     *          for the user (if any ACLs were defined for that user
     *          at all), and one for each group that the user belongs
     *          to.
     *
     *      --  Processes running on behalf of the superuser (root)
     *          will only have a single "0" subject handle.
     *
     *      --  A trusted process that was defined by an adnimistrator
     *          will contain an additional subject handle with the access
     *          permissions defined for it. For example, if the program
     *          "C:\MASTER\MASTER.EXE" is given specific permissions,
     *          there would be one subject handle representing the
     *          ACLs for that in addition to those of the user running
     *          it (if any).
     *
     *      --  For processes that were started during system startup,
     *          the driver creates security contexts with a single "0"
     *          (root) subject handle when XWPShell passes a list of
     *          PIDs to the driver with the "initialization" ring-0 API.
     *          As a result, all processes started through CONFIG.SYS
     *          are presently considered trusted processes.
     */

    typedef struct _XWPSECURITYCONTEXTCORE
    {
        USHORT      pidParent;      // ID of process parent

        USHORT      cSubjects;      // no. of subject handles in this context

        HXSUBJECT   aSubjects[1];   // array of cSubjects subject handles,
                                    // determining the permissions of this
                                    // process

    } XWPSECURITYCONTEXTCORE, *PXWPSECURITYCONTEXTCORE;

    /* ******************************************************************
     *
     *   Access control
     *
     ********************************************************************/

    /*
     *      The following global permissions exist:
     *
     */

    #define XWPPERM_CREATEUSER          0x00000100
                    // permission to create new users

    #define XWPPERM_CHANGEUSER          0x00000200
                    // permission to change user information

    #define XWPPERM_CHANGEUSERPERM      0x00000400
                    // permission to change permissions for a
                    // given user

    #define XWPPERM_DELETEUSER          0x00000800
                    // permission to delete existing users

    #define XWPPERM_CREATEGROUP         0x00001000
                    // permission to create new groups

    #define XWPPERM_CHANGEGROUP         0x00002000
                    // permission to change group information

    #define XWPPERM_CHANGEGROUPERM      0x00000400
                    // permission to change permissions for a
                    // given group

    #define XWPPERM_DELETEGROUP         0x00008000
                    // permission to delete existing groups

    #define XWPPERM_QUERYUSERINFO       0x00010000
                    // permission to retrieve global user, group, and membership info

    /*
     *      The following permissions are defined per resource:
     *
     */

    #define XWPACCESS_READ             0x01                // "R"
                    // For files: Permission to read data from a file and
                    // copy the file.
                    // For directories: Permission to view the directory's
                    // contents.
                    // For copying a file, both the file and its directory
                    // need "Read" permission.
    #define XWPACCESS_WRITE            0x02                // "W"
                    // For files: Permission to write data to a file.
                    // For directories: Permission to write to files
                    // in the directory, but not create files ("Create"
                    // is required for that).
                    // Should be used together with "Read", because
                    // "Write" alone is not terribly useful.
                    // Besides, "Attrib" permission will also be
                    // required.
    #define XWPACCESS_CREATE           0x04                // "C"
                    // Permission to create subdirectories and files
                    // in a directory. "Create" alone allows creation
                    // of the file only, but once it's closed, it
                    // cannot be changed any more.
    #define XWPACCESS_EXEC             0x08                // "X"
                    // Permission to run (not copy) an executable
                    // or command file.
                    // Note: For .CMD and .BAT files, "Read" permission
                    // is also required.
                    // -- for directories: XWPSec defines this as
                    //    with Unix, to change to a directory.
    #define XWPACCESS_DELETE           0x10                // "D"
                    // Permission to delete subdirectories and files.
    #define XWPACCESS_ATRIB            0x20                // "A"
                    // Permission to modify the attributes of a
                    // resource (read-only, hidden, system, and
                    // the date and time a file was last modified).
    #define XWPACCESS_PERM             0x40
                    // Permission to modify the permissions (read,
                    // write, create, execute, and delete) assigned
                    // to a resource for a user or application.
                    // This gives the user limited administration
                    // authority for a given resource.
    #define XWPACCESS_ALL              0x7F
                    // Permission to read, write, create, execute,
                    // or delete a resource, or to modify attributes
                    // or permissions.

    /*
     *@@ XWPSECSTATUS:
     *      structure representing the current status of XWPSec.
     *      Used with QUECMD_QUERYSTATUS.
     *
     *@@added V1.0.1 (2003-01-10) [umoeller]
     */

    typedef struct _XWPSECSTATUS
    {
        BOOL        fLocalSecurity;     // TRUE if XWPSEC32.SYS is active

        // the following fields are only set if fLocalSecurity is TRUE

        ULONG       cbAllocated;        // total fixed memory currently allocated in ring 0
        ULONG       cAllocations,       // no. of allocations made since startup
                    cFrees;             // no. of frees made since startup
        USHORT      cLogBufs,           // current 64K log buffers in use
                    cMaxLogBufs;        // max 64K log buffers that were ever in use
        ULONG       cLogged;            // no. of syscalls that were logged
        ULONG       cGranted,           // no. of syscalls where access was granted
                    cDenied;            // ... and denied
        LONG        cContexts;          // no. of currently allocated security contexts
                                        // (always >= the no. of running processes)
        ULONG       cbACLs;             // of cbAllocated, no. of bytes in use for ACLs
    } XWPSECSTATUS, *PXWPSECSTATUS;

    /* ******************************************************************
     *
     *   User Database
     *
     ********************************************************************/

    /*
     *@@ XWPGROUPDBENTRY:
     *      group entry in the user database.
     *      See userdb.c for details.
     */

    typedef struct _XWPGROUPDBENTRY
    {
        XWPSECID    gid;                            // group ID
        CHAR        szGroupName[XWPSEC_NAMELEN];    // group name
    } XWPGROUPDBENTRY, *PXWPGROUPDBENTRY;

    /*
     *@@ XWPUSERINFO:
     *      description of a user in the user database.
     *      See userdb.c for details.
     */

    typedef struct _XWPUSERINFO
    {
        XWPSECID    uid;                    // user's ID (unique); 0 for root
        CHAR        szUserName[XWPSEC_NAMELEN];
        CHAR        szFullName[XWPSEC_FULLNAMELEN];       // user's clear name
        CHAR        szUserShell[CCHMAXPATH];    // user shell (normally "X:\OS2\PMSHELL.EXE")
    } XWPUSERINFO, *PXWPUSERINFO;

    /*
     *@@ XWPMEMBERSHIP:
     *
     *@@added V1.0.1 (2003-01-05) [umoeller]
     */

    typedef struct _XWPMEMBERSHIP
    {
        ULONG       cGroups;
        XWPSECID    aGIDs[1];
    } XWPMEMBERSHIP, *PXWPMEMBERSHIP;

    /*
     *@@ XWPUSERDBENTRY:
     *
     *@@added V1.0.1 (2003-01-05) [umoeller]
     */

    typedef struct _XWPUSERDBENTRY
    {
        ULONG           cbStruct;
        XWPUSERINFO     User;
        CHAR            szPassword[XWPSEC_NAMELEN];
        XWPMEMBERSHIP   Membership;
    } XWPUSERDBENTRY, *PXWPUSERDBENTRY;

    /* ******************************************************************
     *
     *   Logons
     *
     ********************************************************************/

    /*
     *@@ XWPLOGGEDON:
     *      describes a current user (i.e. a user
     *      which is in the user database and is
     *      currently logged on, either locally
     *      or via network).
     */

    typedef struct _XWPLOGGEDON
    {
        ULONG       cbStruct;       // size of entire structure
        XWPSECID    uid;            // user's ID
        CHAR        szUserName[XWPSEC_NAMELEN];
        USHORT      cSubjects;      // no. of entries in aSubjects array
        HXSUBJECT   aSubjects[1];   // array of subject handles of this user; one "0" entry if root
    } XWPLOGGEDON, *PXWPLOGGEDON;

    /* ******************************************************************
     *
     *   Errors
     *
     ********************************************************************/

    #define ERROR_XWPSEC_FIRST          31000

    #define XWPSEC_INTEGRITY            (ERROR_XWPSEC_FIRST)
    #define XWPSEC_STRUCT_MISMATCH      (ERROR_XWPSEC_FIRST + 1)       // V1.0.1 (2003-01-05) [umoeller]
            // sizeof XWPSHELLQUEUEDATA does not match (different versions?)
    #define XWPSEC_INVALID_DATA         (ERROR_XWPSEC_FIRST + 2)
    #define XWPSEC_CANNOT_GET_MUTEX     (ERROR_XWPSEC_FIRST + 3)
    #define XWPSEC_CANNOT_START_DAEMON  (ERROR_XWPSEC_FIRST + 4)

    #define XWPSEC_INSUFFICIENT_AUTHORITY  (ERROR_XWPSEC_FIRST + 5)

    #define XWPSEC_HSUBJECT_EXISTS      (ERROR_XWPSEC_FIRST + 6)
    #define XWPSEC_INVALID_HSUBJECT     (ERROR_XWPSEC_FIRST + 7)

    #define XWPSEC_INVALID_PID          (ERROR_XWPSEC_FIRST + 10)
    #define XWPSEC_NO_CONTEXTS          (ERROR_XWPSEC_FIRST + 11)

    #define XWPSEC_USER_EXISTS          (ERROR_XWPSEC_FIRST + 20)
    #define XWPSEC_NO_USERS             (ERROR_XWPSEC_FIRST + 21)
    #define XWPSEC_NO_GROUPS            (ERROR_XWPSEC_FIRST + 22)
    #define XWPSEC_INVALID_ID           (ERROR_XWPSEC_FIRST + 23)
                // invalid user or group ID

    #define XWPSEC_NOT_AUTHENTICATED    (ERROR_XWPSEC_FIRST + 30)
    #define XWPSEC_NO_USER_PROFILE      (ERROR_XWPSEC_FIRST + 31)       // V0.9.19 (2002-04-02) [umoeller]
    #define XWPSEC_CANNOT_START_SHELL   (ERROR_XWPSEC_FIRST + 32)
    #define XWPSEC_INVALID_PROFILE      (ERROR_XWPSEC_FIRST + 33)
    #define XWPSEC_NO_LOGON             (ERROR_XWPSEC_FIRST + 34)

    #define XWPSEC_DB_GROUP_SYNTAX      (ERROR_XWPSEC_FIRST + 35)
    #define XWPSEC_DB_USER_SYNTAX       (ERROR_XWPSEC_FIRST + 36)
    #define XWPSEC_DB_INVALID_GROUPID   (ERROR_XWPSEC_FIRST + 37)

    #define XWPSEC_DB_ACL_SYNTAX        (ERROR_XWPSEC_FIRST + 40)
    #define XWPSEC_DB_ACL_INTEGRITY     (ERROR_XWPSEC_FIRST + 41)
    #define XWPSEC_DB_ACL_DUPRES        (ERROR_XWPSEC_FIRST + 42)
                // more than one line for the same resource in ACL DB

    #define XWPSEC_RING0_NOT_FOUND      (ERROR_XWPSEC_FIRST + 50)

    #define XWPSEC_QUEUE_INVALID_CMD    (ERROR_XWPSEC_FIRST + 51)

    #define ERROR_XWPSEC_LAST           (ERROR_XWPSEC_FIRST + 51)

    /* ******************************************************************
     *
     *   XWPShell Shared Memory
     *
     ********************************************************************/

    #define SHMEM_XWPSHELL        "\\SHAREMEM\\XWORKPLC\\XWPSHELL.DAT"
            // shared memory name of XWPSHELLSHARED structure

    /*
     *@@ XWPSHELLSHARED:
     *      shared memory structure allocated by XWPShell
     *      when it starts up. This can be requested by
     *      any process to receive more information and
     *      is also checked by XWorkplace at WPS startup
     *      to determine whether XWPShell is running or
     *      the WPS should be running in single-user
     *      mode.
     */

    typedef struct _XWPSHELLSHARED
    {
        BOOL        fNoLogonButRestart;
                // when the shell process terminates, it
                // can set this flag to TRUE to prevent
                // the logon prompt from appearing; instead,
                // the shell should be restarted with the
                // same user
    } XWPSHELLSHARED, *PXWPSHELLSHARED;

    /* ******************************************************************
     *
     *   XWPShell Queue (Ring 3 API)
     *
     ********************************************************************/

    /*
     *      Ring-3 APIs are implemented via a regular OS/2 Control
     *      Program queue. Programs can call these APIs by allocating
     *      a chunk of shared memory, writing the pointer to that
     *      with a command code into this queue, and blocking on an
     *      event semaphore, which gets posted by XWPShell when the
     *      command has been processed. XWPShell then writes a
     *      result code (NO_ERROR or an error code) and possibly
     *      result data back into that shared memory.
     *
     *      Since OS/2 reports the PID of the queue writer with the
     *      queue APIs, XWPShell can validate whether the writing
     *      process is authorized to perform a certain event. For
     *      example, when a process issues the "create new user"
     *      command, the queue thread in XWPShell will check the
     *      writer's PID and validate the security context of that
     *      process for whether the subjects of that process include
     *      sufficient permission for that command.
     *
     *      The following commands are necessary:
     *
     *      --  Query security context for a PID (no permissions required)
     *
     *      --  Create user (admin permission required)
     *
     *      --  Create group (admin permission required)
     *
     *      --  Set User membership in groups
     *
     *      --  Query user info
     *
     *      --  Query group info
     *
     *      --  Query permissions for a resource
     *
     *      --  Set permissions for a resource
     *
     */

    #define QUEUE_XWPSHELL        "\\QUEUES\\XWORKPLC\\XWPSHELL.QUE"
            // queue name of the master XWPShell queue

    /*
     *@@ QUEUEUNION:
     *
     *@@added V0.9.11 (2001-04-22) [umoeller]
     */

    typedef union _QUEUEUNION
    {
        #define QUECMD_QUERYSTATUS                  1

        XWPSECSTATUS    Status;

        #define QUECMD_QUERYLOCALUSER               2
            // return data for user that is
            // currently logged on locally.
            // Required authority: None.
            // If NO_ERROR is returned, pLocalUser has been
            // set to single XWPUSERDBENTRY structure
            // in shared memory given to the caller.
            // The caller must issue DosFreeMem.
            // Note that the szPassword field is always
            // nulled out.
            // Possible error codes: see slogQueryLocalUser.
        PXWPUSERDBENTRY pLocalUser;

        #define QUECMD_QUERYUSERS                   3
            // return info for all users that
            // are defined in the userdb.
            // Required authority: administrator.
            // Possible error codes: see sudbQueryUsers.
            // If NO_ERROR is returned, paUsers has been
            // set to an array of cUsers XWPUSERDBENTRY
            // structures as shared memory given to the
            // caller. The caller must issue DosFreeMem.
            // Note that the szPassword field for each
            // user is always nulled out.
        struct
        {
            ULONG               cUsers;
            PXWPUSERDBENTRY     paUsers;
        } QueryUsers;

        #define QUECMD_QUERYGROUPS                  4
            // return info for all groups that
            // are defined in the userdb.
            // Required authority: administrator.
            // Possible error codes: see sudbQueryGroups.
            // If NO_ERROR is returned, paGroups has been
            // set to an array of cGroups XWPGROUPDBENTRY
            // structures as shared memory given to the
            // caller. The caller must issue DosFreeMem.
        struct
        {
            ULONG               cGroups;
            PXWPGROUPDBENTRY    paGroups;
        } QueryGroups;

        #define QUECMD_QUERYUSERNAME                5

        struct
        {
            XWPSECID    uid;                            // in: user ID
            CHAR        szUserName[XWPSEC_NAMELEN];     // out: user name
        } QueryUserName;

        #define QUECMD_QUERYPROCESSOWNER            6
            // return the uid of the user who owns
            // the given process.
            // Required authority: XWPPERM_QUERYUSERINFO,
            // unless the given process is owned by the
            // same user who runs the query.
        struct
        {
            USHORT              pid;        // in: PID to query (or 0 for calling process)
            HXSUBJECT           hsubj0;     // out: hSubject of user (or privileged process)
            XWPSECID            uid;        // out: uid of owner, if NO_ERROR is returned
        } QueryProcessOwner;

        #define QUECMD_CREATEUSER                   7

        struct
        {
            CHAR        szUserName[XWPSEC_NAMELEN];         // user name
            CHAR        szFullName[XWPSEC_FULLNAMELEN];     // user's clear name
            CHAR        szPassword[XWPSEC_NAMELEN];         // password
            XWPSECID    uidCreated; // out: uid of new user
        } CreateUser;

        #define QUECMD_SETUSERDATA                  8

        XWPUSERINFO     SetUserData;

        #define QUECMD_DELETEUSER                   9

        XWPSECID        uidDelete;

        #define QUECMD_QUERYPERMISSIONS             10

        struct
        {
            CHAR        szResource[CCHMAXPATH];     // in: resource to query
            ULONG       flAccess;                   // out: XWPACCESS_* flags
        } QueryPermissions;

        #define QUECMD_SWITCHUSER                   11
            // change the credentials of the current process. This
            // allows a process to run on behalf of a different user
            // and can be used to implement a "su" command, since
            // processes started by the current process will inherit
            // those credentials.

        struct
        {
            CHAR        szUserName[XWPSEC_NAMELEN];
            CHAR        szPassword[XWPSEC_NAMELEN];
            XWPSECID    uid;                        // out: user id if NO_ERROR
        } SwitchUser;

    } QUEUEUNION, *PQUEUEUNION;

    /*
     *@@ XWPSHELLQUEUEDATA:
     *      structure used in shared memory to communicate
     *      with XWPShell.
     *
     *      A client process must use the following procedure
     *      to communicate with XWPShell:
     *
     *      1)  Open the public XWPShell queue.
     *
     *      2)  Allocate a giveable shared XWPSHELLQUEUEDATA
     *          and give it to XWPShell as read/write.
     *
     *      3)  Create a shared event semaphore in
     *          XWPSHELLQUEUEDATA.hev.
     *
     *      4)  Set XWPSHELLQUEUEDATA.ulCommand to one of the
     *          QUECMD_*  flags, specifying the data to be queried.
     *
     *      5)  Write the XWPSHELLQUEUEDATA pointer to the
     *          queue.
     *
     *      6)  Block on XWPSHELLQUEUEDATA.hevData, which gets
     *          posted by XWPShell when the data has been filled.
     *
     *      7)  Check XWPSHELLQUEUEDATA.arc. If NO_ERROR,
     *          XWPSHELLQUEUEDATA.Data union has been filled
     *          with data according to ulCommand.
     *
     *      8)  Close the event sem and free the shared memory.
     *
     *@@added V0.9.11 (2001-04-22) [umoeller]
     */

    typedef struct _XWPSHELLQUEUEDATA
    {
        ULONG       cbStruct;           // size of XWPSHELLQUEUEDATA struct (for safety)

        ULONG       ulCommand;          // in: one of the QUECMD_* values

        HEV         hevData;            // in: event semaphore posted
                                        // when XWPShell has produced
                                        // the data

        APIRET      arc;                // out: error code set by XWPShell;
                                        // if NO_ERROR, the following is valid

        QUEUEUNION  Data;               // out: data, format depends on ulCommand

    } XWPSHELLQUEUEDATA, *PXWPSHELLQUEUEDATA;

    /* ******************************************************************
     *
     *   APIs for interfacing XWPShell
     *
     ********************************************************************/

    APIRET xsecQueryStatus(PXWPSECSTATUS pStatus);

    APIRET xsecQueryLocalUser(PXWPUSERDBENTRY *ppLocalUser);

    APIRET xsecQueryAllUsers(PULONG pcUsers,
                             PXWPUSERDBENTRY *ppaUsers);

    APIRET xsecQueryGroups(PULONG pcGroups,
                           PXWPGROUPDBENTRY *ppaGroups);

    APIRET xsecQueryUserName(XWPSECID uid,
                             PSZ pszUserName);

    APIRET xsecQueryProcessOwner(USHORT pid,
                                 XWPSECID *puid);

    APIRET xsecCreateUser(PCSZ pcszUserName,
                          PCSZ pcszFullName,
                          PCSZ pcszPassword,
                          XWPSECID gid,
                          XWPSECID *puid);

    APIRET xsecSetUserData(XWPSECID uid,
                           PCSZ pcszUserName,
                           PCSZ pcszFullName);

    APIRET xsecDeleteUser(XWPSECID uid);

    APIRET xsecQueryPermissions(PCSZ pcszFilename,
                                PULONG pflAccess);

#endif

#if __cplusplus
}
#endif

