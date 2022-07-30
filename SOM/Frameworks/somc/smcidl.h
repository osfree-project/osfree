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

#undef _case
#undef _switch
#undef _struct
#undef _typedef
#undef _name

#ifdef __SOMIDL__
    /*
     * The _case and _switch macro are for IDL Unions.
     */
    #define _case(n)                       case n:
    #define _switch(t)                     switch(t)
    #define _struct                        /* Nothing */
    #define _typedef                       /* Nothing */
    #define _name(name)                    /* Nothing */
#else
    #define _case(n)                       /* Nothing */
    #define _switch(t)                     /* Nothing */
    #define _struct                        struct
    #define _typedef                       typedef
    #define _name(name)                    name
#endif
