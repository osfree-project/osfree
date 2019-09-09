/* $Id: kswap_c.c 364 2002-04-02 18:13:06Z skaus $
	Defines the central pointer to the KSWAP primary data structure.
*/

#include "../config.h"
#include "../include/kswap.h"

#ifndef FEATURE_XMS_SWAP
kswap_p kswapContext = 0;
#endif
