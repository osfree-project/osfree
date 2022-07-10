/* Load/Initialize the classed defined in this DLL */
#include "animal.ih"
#include "dog.ih"
#include "bdog.ih"
#include "ldog.ih"

#ifdef __IBMC__
#pragma linkage(SOMInitModule, system)
#endif

void SOMLINK SOMInitModule(integer4 majorVersion, integer4 minorVersion)
{
  AnimalNewClass(0, 0);     /* initialize the Animal    class.        */
  DogNewClass(0, 0);        /* initialize the Dog       class.        */
  BigDogNewClass(0, 0);     /* initialize the BigDog    class.        */
  LittleDogNewClass(0, 0);  /* initialize the LittleDog class.        */
}
