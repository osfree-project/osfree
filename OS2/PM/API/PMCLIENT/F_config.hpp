/*
 $Id: F_config.hpp,v 1.5 2003/06/18 22:51:49 evgen2 Exp $
*/
/* F_config.hpp - PMCLIENT-local subset.
 *
 * PMCLIENT is the client transport facade. It needs only the shared
 * resource names (mutex semaphore, application identity) and does not
 * use GUI/server configuration structures. This local copy therefore
 * omits F_PresPar.hpp, GUI_WindowOptions and FreePM_Config, which
 * belong to the server side and pull in headers PMCLIENT does not
 * require.
 */

#ifndef FREEPMCONFIG
 #define FREEPMCONFIG

/*+---------------------------------+*/
/*| defines for config FreePM       |*/
/*+---------------------------------+*/
#define FREEPM_VERSION         "0.0.0.1"
#define FREEPM_SERVER_APPLICATION_NAME  "FreePM Server"
#define FREEPM_CLIENT_APPLICATION_NAME  "FreePM Client"

/* Ahtung!  the name of mutex smaphore shuld not be the same as VAC3
   project name !!!
*/
#define FREEPM_MUTEX_NAME            "\\SEM32\\_FREE_PM" /* Semaphore name */

#endif
   /* FREEPMCONFIG */
