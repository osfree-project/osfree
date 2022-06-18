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
#include <som.xh>    
#include <somobj.xh>
#include <somcm.xh>

using namespace std;

int main(int argc, char *argv[], char *envp[])
{
   SOMClass      *myClass;
   somId         classId;

   somEnvironmentNew();
   classId = somIdFromString(argv[1]);

   //******************************************************
   // The following path has to be modified depending on
   // where you install the "domestic" DLL
   //******************************************************
   myClass = SOMClassMgrObject->somFindClsInFile(classId,
                                   0,0,
                                   "domestic.dll");

   if (myClass)
   {
      SOMObject *myObj;
      Environment *ev;
 
      ev = somGetGlobalEnvironment();
      myObj = myClass->somNew();  // create an object

      if (myObj)
      {
//      somPrintf("class name %s\n", myObj->somGetClassName() );

        myObj->somDispatch((somToken*)0,
                           somIdFromString("display"),
                           myObj,
                           ev);
      }
   } 
   else
   {
      cout << "Can't load class " << argv[1] << "\n";
   } 
}
