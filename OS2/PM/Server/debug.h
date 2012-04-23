/* debug support */
#define FPM_SERVER

extern "C" int _FreePM_db_level;
extern "C" int _FreePM_debugLevels[MAX_DEBUG_SECTIONS];
extern "C" void APIENTRY _db_print(const char *format,...);

/*
#undef  db_print
#undef  debug

#define debug(SECTION, LEVEL) \
        ((_FreePM_db_level = (LEVEL)) > _FreePM_debugLevels[SECTION]) ? (void) 0 : _db_print

#define db_print _dbprint
 */
