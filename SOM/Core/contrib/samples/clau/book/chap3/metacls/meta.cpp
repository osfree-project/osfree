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

#include "animal.xh"
#include "robin.xh"
#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{
   Animal      *cat, *dog;
   Robin       *bird;
   SOMClass    *birdClass;
   SOMClass    *birdClassClass;

   MetaAnimal  *mcat;
   Environment *ev;

   ev = somGetGlobalEnvironment();

   //***************************************
   // Create the Animal class object
   //***************************************
   mcat = AnimalNewClass(0,0);

   //******************************************
   // Create the Animal object with name Spot
   //******************************************
   cat = mcat->createAnimal(ev, "Spot");
   cat->sleep(ev);
   somPrintf("Spot class is %s\n", cat->somGetClassName() );

   //*************************************************
   // Create another Animal object with name Lassie
   //*************************************************
   dog = mcat->createAnimal(ev,"Lassie");
   dog->sleep(ev);

   printf("total number of Animal objects: %d\n", mcat->_get_count(ev) );

   //**************************************** 
   // Create a Bird object
   //****************************************
   printf("\nCreate a Robin object\n");
   bird = new Robin;
   birdClass = bird->somGetClass();
   somPrintf("Robin meta class is: %s\n", birdClass->somGetClassName());
}
