/**************************************************************************
 *
 *  Copyright 1994, Christina Lau
 *
 *  This file is part of Object-Orientated Programming using SOM and DSOM.
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

#include <stdlib.h>
#include <string.h>
#include <somd.xh>
#include <implrep.xh>
#include <stdio.h>

int main(int argc, char **argv)
{
  Environment       *ev;
  ImplementationDef *implDef;
  sequence(string)  classes;
  short             i;

  if (argc != 2) 
  {
     printf("You must specify an Implementation alias (e.g. Chris)\n");
     return 1;
  } 
  
  ev = SOM_CreateLocalEnvironment();
  SOMD_Init(ev);

  implDef = SOMD_ImplRepObject->find_impldef_by_alias(
                                                  ev, argv[1]);
  if (implDef) 
  {
    classes = SOMD_ImplRepObject->find_classes_by_impldef(ev,
                                        implDef->_get_impl_id(ev));

    printf("Server : %s\n", argv[1]);
    printf("Program Name: %s\n", implDef->_get_impl_program(ev));
    printf("Server Class: %s\n", implDef->_get_impl_server_class(ev));
    printf("Host Name: %s\n", implDef->_get_impl_hostname(ev));
 
    for (i=0; i < sequenceLength(classes); i++)
    {
        printf("%s\n", sequenceElement(classes,i) );
    }
  }
  else
  {
     printf("Implementation Alias not found in Implementation Repository\n");
  }

  SOMD_Uninit(ev);
  SOM_UninitEnvironment(ev);
  return 0;
}
