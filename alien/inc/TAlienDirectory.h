// @(#)root/alien:$Name:  $:$Id: TAlienDirectory.h,v 1.1 2005/08/12 15:46:40 rdm Exp $
// Author: Jan Fiete Grosse-Oetringhaus   28/9/2004

/*************************************************************************
 * Copyright (C) 1995-2004, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TAlienDirectory
#define ROOT_TAlienDirectory

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlienDirectory                                                      //
//                                                                      //
// Class which creates Directory files for the AliEn middleware.        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_Tlist
#include "TList.h"
#endif
#ifndef ROOT_TBrowser
#include "TBrowser.h"
#endif
#ifndef ROOT_TNamed
#include "TNamed.h"
#endif


class TAlienDirectoryEntry : public TNamed {

private:
   TString fLfn;	// logical file name

public:
   TAlienDirectoryEntry(const char *lfn, const char *name) : TNamed(name,name) { fLfn = lfn; }
   virtual ~TAlienDirectoryEntry() { }
   Bool_t IsFolder() const { return kTRUE; }
   void Browse(TBrowser *b);

   ClassDef(TAlienDirectoryEntry,1)  // Creates Directory files entries for the AliEn middleware
};


class TAlienDirectory : public TNamed {

private:
   TList fEntries;   // directory entries

public:
   TAlienDirectory(const char *ldn, const char *name=0);
   virtual ~TAlienDirectory();

   Bool_t IsFolder() const { return kTRUE; }
   void   Browse(TBrowser *b);

   ClassDef(TAlienDirectory,1)  // Creates Directory files for the AliEn middleware
};

#endif
