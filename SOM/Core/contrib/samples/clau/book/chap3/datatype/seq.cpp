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

#include "employee.xh"
#include "company.xh"
#include <iostream>
using namespace std;

void testSequence()
{
  Environment *ev = somGetGlobalEnvironment();    
  Employee *emp1, *emp2;
  Company  *comp;
  _IDL_SEQUENCE_Employee list;
  short i;

  emp1 = new Employee;
  emp1->_set_name(ev, "Mary");

  emp2 = new Employee;
  emp2->_set_name(ev, "John");

  comp = new Company;
  comp->addEmployee(ev, emp1);
  comp->addEmployee(ev, emp2);
  
  list = comp->_get_empList(ev);
                                                 
  for (i=0; i < sequenceLength(list); i++)
  {
    cout << sequenceElement(list,i)->_get_name(ev) << "\n";
  }
}

int main(int argc, char *argv[], char *envp[])
{
   testSequence();
}
