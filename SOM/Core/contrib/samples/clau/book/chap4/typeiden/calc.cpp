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

#include <iostream>
#include "mgr.xh"
#include "prg.xh"

using namespace std;

void calc(Employee *emp)
{
   SOMClass    *mgrClass;
   SOMClass    *empClass;
   Environment *ev;

   ev = somGetGlobalEnvironment();

   //****************************************************
   // Use <className>ClassData.classObject to get
   // pointer to class object
   // Alternatively, use _Programmer
   //****************************************************
   if (emp->somIsA(ProgrammerClassData.classObject))
   {
      cout << "Programmer salary: " ;
      cout << (emp->salary(ev) + ( (Programmer *) emp)->overtime(ev));
   }

   //************************************************
   // Create a Manager Class Object
   //************************************************
   mgrClass = ManagerNewClass(0,0);
   if ( emp->somIsA(mgrClass) )
   {
      cout << "Manager salary: " ;
      cout << (emp->salary(ev) + ((Manager *) emp)->bonus(ev));
   }
   cout << "\n";
}

int main(int argc, char *argv[], char *envp[])
{
   Manager    *mgr;
   Programmer *prg;

   mgr = new Manager();
   prg = new Programmer();

   calc(mgr);
   calc(prg);
}
