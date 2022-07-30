/**************************************************************************
 *
 *  Copyright 2022, Yuri Prokushev
 *
 *  This file is part of osFree project
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

// OPTLINK emulation for MSVC
#if defined(_WIN32) && defined(_M_IX86) && !defined(_WIN64)
        #define OPTLINK_3ARGS(a,b,c)            \
                        __asm { mov             a,eax   }       \
                        __asm { mov             b,edx   }       \
                        __asm { mov             c,ecx   }
        #define OPTLINK_2ARGS(a,b)            \
                        __asm { mov             a,eax   }       \
                        __asm { mov             b,edx   }       
        #define OPTLINK_1ARG(a)            \
                        __asm { mov             a,eax   }
        #define OPTLINK_DECL    __cdecl
#else
        #define OPTLINK_3ARGS(a,b,c)
        #define OPTLINK_2ARGS(a,b)
        #define OPTLINK_1ARG(a)
        #define OPTLINK_DECL
#endif
