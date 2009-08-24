#include <e32std.h>
#include <e32hal.h>
#include <d32snd.h>
#include <stdio.h>
#include <string.h>

#include "utsname.h"

extern "C"
{
int epoc32_uname(struct regina_utsname *name)
{
   TMachineInfoV1Buf machine;
   UserHal::MachineInfo (machine);

   memcpy( (char *)name->sysname, (char *)machine().iProcessorName.Ptr(), machine().iProcessorName.Length()  );
   name->sysname[machine().iProcessorName.Length()] = '\0';
   sprintf( name->version,"%d.%d",machine ().iRomVersion.iMajor, machine ().iRomVersion.iMinor );
   sprintf( name->release,"%d",machine ().iRomVersion.iBuild );
   memcpy( (char *)name->nodename, (char *)machine().iPsuName.Ptr(), machine().iPsuName.Length() );
   name->nodename[machine().iPsuName.Length()] = '\0';
   memcpy( (char *)name->machine, (char *)machine().iMachineName.Ptr(), machine().iMachineName.Length() );
   name->machine[machine().iMachineName.Length()] = '\0';

   return 0;
}
void beep( int freq, int dur )
{
   RDevSound snd;
   TInt ret;
   TRequestStatus status;
   TSoundConfig config;

   ret = snd.Open();
   config().iVolume = EVolumeMedium;
   snd.SetConfig( config );
   snd.PlaySineWave( status, freq, EVolumeMedium, dur );
   User::WaitForRequest( status );
   snd.Close();
}

}
