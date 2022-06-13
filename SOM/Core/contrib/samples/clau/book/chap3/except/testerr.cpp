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

#include "error.xh"
#include <iostream>
using namespace std;

int main()
{
  Environment   *ev;
  ErrorExample  *ex1;
  char          *exId;
  BadCall       *bc;
  StExcep       *stExVal;

  ev = somGetGlobalEnvironment();
  ex1 = new ErrorExample;
  ex1->execute(ev);

  switch(ev->_major)
  {
    case SYSTEM_EXCEPTION:
       exId = somExceptionId(ev);
       stExVal = (StExcep *) somExceptionValue(ev);
       cout << "Minor Code " << stExVal->minor;
       cout << "Status " << stExVal->completed;
       somExceptionFree(ev);
       break;
     
    case USER_EXCEPTION:
        exId = somExceptionId(ev);
        if (strcmp(exId, ex_ErrorExample_BadCall) == 0)
        {
           bc = (BadCall *) somExceptionValue(ev);
           cout << bc->errorCode << "\n";
           cout << bc->reason << "\n";
           somExceptionFree(ev);         // Free exception memory
        }
       break;
     
    case NO_EXCEPTION:
       break;
  } 
}
