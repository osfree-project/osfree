// RexxFileRecognizer.h
//
// Copyright (c) 1997-1999 Symbian Ltd.  All rights reserved.
// Copyright (c) 2001 Mark Hessling
//

#if !defined(__RECREXX_H__)
#define __RECREXX_H__

#if !defined(__APMREC_H__)
#include <apmrec.h>
#endif

class CApaTextRecognizer : public CApaDataRecognizerType
{
   public: // from CApaDataRecognizerType
      CApaTextRecognizer();
      TUint PreferredBufSize();
      TDataType SupportedDataTypeL(TInt aIndex) const;
   private: // from CApaDataRecognizerType
      void DoRecognizeL(const TDesC& aName, const TDesC8& aBuffer);
};

#endif
