/* This file should only be used and compiled if you want to use checker.
 * It contains wrappers für multi-threaded library calls for POSIX only.
 */
#include <pthread.h>
#include <stdlib.h>
#ifdef LIB_CRYPT
# include <crypt.h>
#endif
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dlfcn.h>
#include <checker_api.h>

#ifdef __CHECKER__
#  error This file can only be compiled without checkergcc. Use gcc for this file and checkergcc for the rest of your files!
#endif

#define CHECK_IF_INITIALIZED(ptr) stubs_chkr_check_addr(ptr,sizeof(*ptr),CHKR_TW,#ptr)
#define WILL_BE_WRITTEN(ptr) stubs_chkr_check_addr(ptr,sizeof(*ptr),CHKR_MW,#ptr)
#define WILL_BE_READ(ptr) stubs_chkr_check_addr(ptr,sizeof(*ptr),CHKR_RO,#ptr)
#define IS_WRITTEN(ptr) stubs_chkr_set_right(ptr,sizeof(*ptr),CHKR_RW)

#ifdef LIB_PTHREAD
extern void chkr_pthread_cleanup_pop(struct _pthread_cleanup_buffer *buffer,int execute) __asm__ ("chkr._pthread_cleanup_pop");
void chkr_pthread_cleanup_pop(struct _pthread_cleanup_buffer *buffer,int execute)
{
   CHECK_IF_INITIALIZED(buffer);
   _pthread_cleanup_pop(buffer,execute);
}

extern void chkr_pthread_cleanup_push(struct _pthread_cleanup_buffer *buffer,void (*routine)(void *),void *arg) __asm__ ("chkr._pthread_cleanup_push");
void chkr_pthread_cleanup_push(struct _pthread_cleanup_buffer *buffer,void (*routine)(void *),void *arg)
{
   WILL_BE_WRITTEN(buffer);
   _pthread_cleanup_push(buffer,routine,arg);
   IS_WRITTEN(buffer);
}

extern void *chkr_pthread_getspecific(pthread_key_t key) __asm__ ("chkr.pthread_getspecific");
void *chkr_pthread_getspecific(pthread_key_t key)
{
   return(pthread_getspecific(key));
}

extern int chkr_pthread_setspecific(pthread_key_t key,const void *ptr) __asm__ ("chkr.pthread_setspecific");
int chkr_pthread_setspecific(pthread_key_t key,const void *ptr)
{
   return(pthread_setspecific(key,ptr));
}

extern int chkr_pthread_key_create(pthread_key_t *key,void (*destructor)(void *buf)) __asm__ ("chkr.pthread_key_create");
int chkr_pthread_key_create(pthread_key_t *key,void (*destructor)(void *buf))
{
   int retval;
   
   WILL_BE_WRITTEN(key);
   retval = pthread_key_create(key,destructor);
   IS_WRITTEN(key);
   return(retval);
}

extern int chkr_pthread_mutex_lock(pthread_mutex_t *mutex) __asm__ ("chkr.pthread_mutex_lock");
int chkr_pthread_mutex_lock(pthread_mutex_t *mutex)
{
   CHECK_IF_INITIALIZED(mutex);
   return(pthread_mutex_lock(mutex));
}

extern int chkr_pthread_mutex_unlock(pthread_mutex_t *mutex) __asm__ ("chkr.pthread_mutex_unlock");
int chkr_pthread_mutex_unlock(pthread_mutex_t *mutex)
{
   CHECK_IF_INITIALIZED(mutex);
   return(pthread_mutex_unlock(mutex));
}

extern int chkr_pthread_once(pthread_once_t *once,void (*routine)(void)) __asm__ ("chkr.pthread_once");
int chkr_pthread_once(pthread_once_t *once,void (*routine)(void))
{
   CHECK_IF_INITIALIZED(once);
   return(pthread_once(once,routine));
}

extern pthread_t chkr_pthread_self(void) __asm__ ("chkr.pthread_self");
pthread_t chkr_pthread_self(void)
{
   return(pthread_self());
}

extern int chkr_pthread_create(pthread_t *thread,const pthread_attr_t *attr,void *(*starter)(void *),void *arg) __asm__ ("chkr.pthread_create");
int chkr_pthread_create(pthread_t *thread,const pthread_attr_t *attr,void *(*starter)(void *),void *arg)
{
   int retval;
   
   WILL_BE_WRITTEN(thread);
   CHECK_IF_INITIALIZED(attr);
   retval = pthread_create(thread,attr,starter,arg);
   if (retval == 0)
      IS_WRITTEN(thread);
   return(retval);
}

extern int chkr_pthread_join(pthread_t th,void **retval) __asm__ ("chkr.pthread_join");
int chkr_pthread_join(pthread_t th,void **retval)
{
   if (retval)
      CHECK_IF_INITIALIZED(retval); 
   return(pthread_join(th,retval));
}

extern void chkr_pthread_exit(void *buf) __asm__ ("chkr.pthread_exit");
void chkr_pthread_exit(void *buf)
{
   pthread_exit(buf);
}
#endif

extern int chkr_getgrgid_r(__gid_t gid,struct group *__restrict resultbuf,char *__restrict buf,size_t bufsize,struct group **__restrict result) __asm__ ("chkr.getgrgid_r");
int chkr_getgrgid_r(__gid_t gid,struct group *__restrict resultbuf,char *__restrict buf,size_t bufsize,struct group **__restrict result)
{
   int retval;
   
   WILL_BE_WRITTEN(resultbuf);
   stubs_chkr_check_addr(buf,bufsize,CHKR_MW,"buf");
   retval = getgrgid_r(gid,resultbuf,buf,bufsize,result);
   if (retval == 0)
   {
      IS_WRITTEN(resultbuf);
      stubs_chkr_set_right(buf,bufsize,CHKR_RW);
   }
   return(retval);
}

extern int chkr_getpwuid_r(__uid_t uid,struct passwd *__restrict resultbuf,char *__restrict buf,size_t bufsize,struct passwd **__restrict result) __asm__ ("chkr.getpwuid_r");
int chkr_getpwuid_r(__uid_t uid,struct passwd *__restrict resultbuf,char *__restrict buf,size_t bufsize,struct passwd **__restrict result)
{
   int retval;
   
   WILL_BE_WRITTEN(resultbuf);
   stubs_chkr_check_addr(buf,bufsize,CHKR_MW,"buf");
   retval = getpwuid_r(uid,resultbuf,buf,bufsize,result);
   if (retval == 0)
   {
      IS_WRITTEN(resultbuf);
      stubs_chkr_set_right(buf,bufsize,CHKR_RW);
   }
   return(retval);
}

extern struct tm *chkr_gmtime_r(__const time_t *__restrict timer,struct tm *__restrict tm) __asm__ ("chkr.gmtime_r");
struct tm *chkr_gmtime_r(__const time_t *__restrict timer,struct tm *__restrict tm)
{
   struct tm *retval;
   
   CHECK_IF_INITIALIZED(timer);
   WILL_BE_WRITTEN(tm);
   retval = gmtime_r(timer,tm);
   if (retval != NULL)
      IS_WRITTEN(tm);
   return(retval);
}

extern struct tm *chkr_localtime_r(__const time_t *__restrict timer,struct tm *__restrict tm) __asm__ ("chkr.localtime_r");
struct tm *chkr_localtime_r(__const time_t *__restrict timer,struct tm *__restrict tm)
{
   struct tm *retval;
   
   CHECK_IF_INITIALIZED(timer);
   WILL_BE_WRITTEN(tm);
   retval = localtime_r(timer,tm);
   if (retval != NULL)
      IS_WRITTEN(tm);
   return(retval);
}

extern int chkr_initstate_r(unsigned seed,__ptr_t __restrict statebuf,size_t statelen,struct random_data *__restrict buf) __asm__ ("chkr.initstate_r");
int chkr_initstate_r(unsigned seed,__ptr_t __restrict statebuf,size_t statelen,struct random_data *__restrict buf)
{
   int retval;
   
   stubs_chkr_check_addr(statebuf, statelen, CHKR_RO, "statebuf");
   WILL_BE_WRITTEN(buf);
   retval = initstate_r(seed,statebuf,statelen,buf);
   if (retval == 0)
      IS_WRITTEN(buf);
   return(retval);
}

extern int chkr_random_r(struct random_data *__restrict buf,int32_t *__restrict result) __asm__ ("chkr.random_r");
int chkr_random_r(struct random_data *__restrict buf,int32_t *__restrict result)
{
   int retval;
   
   CHECK_IF_INITIALIZED(buf);
   WILL_BE_WRITTEN(result);
   retval = random_r(buf,result);
   if (retval == 0)
      IS_WRITTEN(result);
   return(retval);
}

extern int chkr_srandom_r(unsigned seed,struct random_data *__restrict buf) __asm__ ("chkr.srandom_r");
int chkr_srandom_r(unsigned seed,struct random_data *__restrict buf)
{
   CHECK_IF_INITIALIZED(buf);
   return(srandom_r(seed,buf));
}

extern char *chkr_strerror_r(int errnum,char *buf,size_t buflen) __asm__ ("chkr.strerror_r");
char *chkr_strerror_r(int errnum,char *buf,size_t buflen)
{
   char *retval;
   
   stubs_chkr_check_addr(buf,buflen,CHKR_MW,"buf");
   retval = strerror_r(errnum,buf,buflen);
   if (retval != NULL)
      stubs_chkr_set_right(buf,buflen,CHKR_RW);
   return(retval);
}

#ifdef LIB_CRYPT
extern char *chkr_crypt(const char *key,const char *salt) __asm__ ("chkr.crypt");
char *chkr_crypt(const char *key,const char *salt)
{
   char *retval;
   
   stubs_chkr_check_str(key,CHKR_RO,"key");
   stubs_chkr_check_str(salt,CHKR_RO,"salt");
   
   retval = crypt(key,salt);
   if (retval)
      stubs_chkr_set_right(retval,strlen(retval),CHKR_RO);
   return(retval);
}
#endif

#ifdef LIB_DL
extern void *chkr_dlopen(const char *filename,int flag) __asm__ ("chkr.dlopen");
void *chkr_dlopen(const char *filename,int flag)
{
   stubs_chkr_check_str(filename,CHKR_RO,"filename");
   return(dlopen(filename,flag)); /* don't know anything about the retval */
}

extern void *chkr_dlsym(void *__restrict handle,const char *__restrict name) __asm__ ("chkr.dlsym");
void *chkr_dlsym(void *__restrict handle,const char *__restrict name)
{
   stubs_chkr_check_str(name,CHKR_RO,"name");
   return(dlsym(handle,name)); /* don't know anything about the retval */
}

extern char *chkr_dlerror(void) __asm__ ("chkr.dlerror");
char *chkr_dlerror(void)
{
   char *retval = dlerror();
   
   if (retval)
      stubs_chkr_set_right(retval,strlen(retval),CHKR_RO);
   return(retval);
}
#endif
