#define INCL_BASE
#define INCL_DDIDEFS
#include "F_platform.h"

#include "Fs_config.hpp"

// PPATH not defined. But seems to be PRECTL
#define PPATH PRECTL
// PPIPELINEINFO not defined. Info not found.
#define PPIPELINEINFO PVOID

#define INCL_GREALL
#define INCL_AVIOP
#define INCL_GRE_HFONT
#define INCL_WINPOINTERS
#define INCL_GRE_PATHS
#define INCL_DDIPATHS
#define INCL_DDIMISC
#include "pmddi.h"

#include "F_debug.h"

#include "Fs_dc.hpp"
