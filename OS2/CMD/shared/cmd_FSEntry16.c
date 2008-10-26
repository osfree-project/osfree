/*!
   $Id: cmd_FSEntry16.c,v 1.1.1.1 2003/10/04 08:24:20 prokushev Exp $

   @file cmd_FSEntry16.c
   @brief workaround for Watcom's behaviour/bug when calling 16bit dynamically
   loaded entrypoints, sumplementary for cmd_ExecFSEntry

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Michael Neceasek <MichalN@prodigy.net>
*/

unsigned short _far16 _pascal FSENTRY16(void _far16 *funcptr, short argc, char _far16 * _far16 *argv,
                                 char _far16 * _far16 *envp)
{
    unsigned short _pascal (* _far16 func16)();

    func16 = funcptr;

    return func16(argc, argv, envp);
};

