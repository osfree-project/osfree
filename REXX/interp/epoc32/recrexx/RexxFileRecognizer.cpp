// RexxFileRecognizer.cpp
//
// Copyright (c) 1997-1999 Symbian Ltd.  All rights reserved.
// Copyright (c) 2001 Mark Hessling
//

#include <apmrec.h>
#include <apmstd.h>
#include "RexxFileRecognizer.h"

const TInt KMimeRexxRecognizerValue=0x101F401F;
const TUid KUidMimeRexxRecognizer={0x101F401F};
const TInt KMinBufferLength=42;  // minimum amount of file needed to determine a text file IF it's not called .TXT
const TInt KMaxBufferLength=1024; // maximum amount of buffer space we will ever use
_LIT8(KDataTypeTextRexx,"text/rexx");

CApaTextRecognizer::CApaTextRecognizer()
   :CApaDataRecognizerType(KUidMimeRexxRecognizer,CApaDataRecognizerType::ELow)
// Rexx files are low recognition - they don't have a clear signature
{
   iCountDataTypes=1;
}

TUint CApaTextRecognizer::PreferredBufSize()
{
   return KMaxBufferLength;
}

TDataType CApaTextRecognizer::SupportedDataTypeL(TInt aIndex) const
{
   __ASSERT_DEBUG(aIndex==0,User::Invariant());
   return TDataType(KDataTypeTextRexx);
}

void CApaTextRecognizer::DoRecognizeL(const TDesC& aName, const TDesC8& aBuffer)
{
   TBool nameRecognized = EFalse;
   TBool validChars = EFalse;
   TUint chr1,chr2;
   // Check file names
   if ( aName.Length() > 5 )
   {
      if ( aName.Right(5).CompareF(_L(".rexx")) == 0 )
         nameRecognized = ETrue;
   }
   if ( aName.Length() > 4 && !nameRecognized )
   {
      if ( aName.Right(4).CompareF(_L(".rex")) == 0 )
         nameRecognized = ETrue;
   }
   // Now determine if the file starts with a Rexx Comment
   TInt length = aBuffer.Length();
   if (length < KMinBufferLength && !nameRecognized)
      return;

   chr1=aBuffer[0];
   chr2=aBuffer[1];
   if ( chr1 == '/' && chr2 == '*' )
      validChars=ETrue;
   // Determine how confident we are that this is a Rexx program
   if ( !nameRecognized && !validChars )
      return;
   if ( !nameRecognized && validChars )
      iConfidence=EPossible;
   if ( nameRecognized && !validChars )
      iConfidence=EProbable;
   if ( nameRecognized && validChars )
      iConfidence = ECertain;
   iDataType = TDataType(KDataTypeTextRexx);
}



EXPORT_C CApaDataRecognizerType* CreateRecognizer()
// The gate function - ordinal 1
//
{
   CApaDataRecognizerType* thing=new CApaTextRecognizer();
   return thing; // NULL if new failed
}

GLDEF_C TInt E32Dll(TDllReason /*aReason*/)
//
// DLL entry point
//
{
   return KErrNone;
}
