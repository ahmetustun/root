
// Author: Anna Kreshuk, March 2007

/////////////////////////////////////////////////////////////////
//
//___A stress test for the TEntryList class and operations with it___
//   
//   The functions below test different properties of TEntryList
//   - Test1() - assembling entry lists for smaller chains from the lists
//               for bigger chains and vice versa + using them in TTree::Draw
//   - Test2() - adding and subtracting entry lists in different order
//               and using ">>+elist" in TTree::Draw
//   - Test3() - transforming TEventList objects into TEntryList objects for a TChain
//   - Test4() - same as Test3() but for a TTree 
//
//   To run in batch mode, do
//     stressEntryList
//     stressEntryList 1000
//     stressEntryList 1000 10
//   Here the 1st parameter is the number of entries in each TTree, 
//            2nd parameter is the number of created files
//   Default values are 10000 10
//
//   An example of output when all tests pass:
// **********************************************************************
// ***************Starting TEntryList stress test************************
// **********************************************************************
// **********Generating 10 data files, 2 trees of 10000 in each**********
// **********************************************************************
// Test1: Applying different entry lists to different chains --------- OK
// Test2: Adding and subtracting entry lists-------------------------- OK
// Test3: TEntryList and TEventList for TChain------------------------ OK
// Test4: TEntryList and TEventList for TTree------------------------- OK
// **********************************************************************
// *******************Deleting the data files****************************
// **********************************************************************

#include "TApplication.h"
#include "TEntryList.h"
#include "TEventList.h"
#include "TTree.h"
#include "TChain.h"
#include "TRandom.h"
#include "TH1F.h"
#include "TCut.h"
#include "TFile.h"
#include "TSystem.h"

Int_t stressEntryList(Int_t nentries = 10000, Int_t nfiles = 10);
void MakeTrees(Int_t nentries, Int_t nfiles);

Bool_t Test1()
{
   //Test the functionality of entry lists for chains:
   //making new entry lists out of parts of other entry lists
   //applying same entry lists to different chains, etc

   Int_t wrongentries1, wrongentries2, wrongentries3, wrongentries4, wrongentries5;
   TChain *bigchain = new TChain("bigchain", "bigchain");
   bigchain->Add("stressEntryListTrees*.root/tree1");
   bigchain->Add("stressEntryListTrees*.root/tree2");

   TChain *smallchain = new TChain("smallchain", "smallchain");
   smallchain->Add("stressEntryListTrees*.root/tree1");

   //create an entry list for the small chain
   TCut cut = "x<0 && y>0";
   smallchain->Draw(">>elist_small", cut, "entrylist");
   TEntryList *elist_small = (TEntryList*)gDirectory->Get("elist_small");

   //check if the entry list contains correct entries
   Int_t range = 100;
   TH1F *hx = new TH1F("hx", "hx", range, -range, range);
   smallchain->Draw("x >> hx", cut, "goff");
   TH1F *hcheck = new TH1F("hcheck", "hcheck", range, -range, range);

   smallchain->SetEntryList(elist_small);
   smallchain->Draw("x >> hcheck", "", "goff");
   wrongentries1 = 0;
   for (Int_t i=1; i<=range; i++){
      if (TMath::Abs(hx->GetBinContent(i)-hcheck->GetBinContent(i)) > 0.1){
         wrongentries1++;
      }
   }
   if (wrongentries1 >0)
      printf("\nsmall list and small chain: number of wrong bins=%d\n", wrongentries1);

   //set this small entry list to the big chain and check the results
   bigchain->SetEntryList(elist_small);
   bigchain->Draw("x >> hcheck_", "", "goff");
   wrongentries2 = 0;
   for (Int_t i=1; i<=range; i++){
      if (TMath::Abs(hx->GetBinContent(i)-hcheck->GetBinContent(i)) > 0.1){
         wrongentries2++;
      }
   }
   if (wrongentries2 >0)
      printf("\nsmall elist and big chain: number of wrong bins=%d\n", wrongentries2);

   smallchain->SetEntryList(0);
   bigchain->SetEntryList(0);

   //make an entry list for a big chain
   bigchain->Draw(">>elist_big", cut, "entrylist");
   TEntryList* elist_big = (TEntryList*)gDirectory->Get("elist_big");
   //make a small entry list by extracting the lists, corresponding to the trees in
   //the small chain, from the big entry list
   TEntryList *list_extracted = new TEntryList("list_extracted", "list_extracted");
   TEntryList *elist_temp;
   TList *lists = elist_big->GetLists();
   TIter next(lists);
   while ((elist_temp = (TEntryList*)next())){
      if (!strcmp(elist_temp->GetTreeName(),"tree1"))
         list_extracted->Add(elist_temp);
   }

   //compare this extracted list to the list, generated by smallchain->Draw()
   Long64_t entry1, entry2;
   Int_t n=list_extracted->GetN();
   wrongentries3 = 0;
   for (Int_t i=0; i<n; i++){
      entry1 = list_extracted->GetEntry(i);
      entry2 = elist_small->GetEntry(i);
      if (entry1 != entry2){
         if (wrongentries3<10) printf("wrong entry: %d list2=%lld elist_small=%lld\n", i, entry1, entry2);
         wrongentries3++;
      }
   }
   if (wrongentries3 >0)
      printf("\nsmall list and extracted list: number of wrong entries = %d, n=%d\n", wrongentries3,n);

   //add another entry list to the extracted list
   elist_temp = (TEntryList*)lists->Last();
   list_extracted->Add(elist_temp);
   smallchain->SetEntryList(list_extracted);
   smallchain->Draw("x>>hcheck", "", "goff");
   wrongentries4 = 0;
   for (Int_t i=1; i<=range; i++){
      if (TMath::Abs(hx->GetBinContent(i)-hcheck->GetBinContent(i)) > 0.1){
         //printf("%d hx: %f hcheck %f\n", i, hx->GetBinContent(i), hcheck->GetBinContent(i));
         wrongentries4++;
      }
   }
   if (wrongentries4 >0)
      printf("\nextracted list with 1 wrong: number of wrong bins=%d\n", wrongentries4);
   
   //set the big entry list to the small chain and compare the results with
   //the entry list, generated by smallchain->Draw()
   smallchain->SetEntryList(elist_big);
   smallchain->Draw("x >> hcheck", "", "goff");
   wrongentries5 = 0;
   for (Int_t i=1; i<=range; i++){
      if (TMath::Abs(hx->GetBinContent(i)-hcheck->GetBinContent(i)) > 0.1){
         //printf("i=%d hx(i)=%f, hcheck(i)=%f\n", i, hx->GetBinContent(i), hcheck->GetBinContent(i)); 
         wrongentries5++;
      }
   }
   if (wrongentries5 >0)
      printf("\nbig elist and small chain: number of wrong bins = %d\n", wrongentries5);

   delete bigchain;
   delete smallchain;
   delete hx;
   delete hcheck;
   delete elist_big;
   delete elist_small;
   delete list_extracted;

   if (wrongentries1>0 || wrongentries2>0 || wrongentries3>0 || wrongentries4>0 || wrongentries5>0)
      return kFALSE;
   return kTRUE;
}

Bool_t Test2()
{
   //Test adding and subtracting entry lists

   Int_t wrongentries1, wrongentries2, wrongentries3, wrongentries4, wrongentries5;
   TChain *chain = new TChain("chain", "chain");
   chain->Add("stressEntryListTrees_0.root/tree1");
   chain->Add("stressEntryListTrees_0.root/tree2");
   //chain->Add("stressEntryListTrees*.root/tree1");
   //chain->Add("stressEntryListTrees*.root/tree2");
   TCut cut1("cut1", "x>0");
   TCut cut2("cut2", "y<0.1 && y>-0.1");
   TEntryList *elist1 = new TEntryList("elist1", "elist1");
   chain->Draw(">>elist1", cut1, "entrylist");
   TEntryList *elist2 = new TEntryList("elist2", "elist2");
   chain->Draw(">>elist2", cut2, "entrylist");

   //add those 2 lists (1+2)
   TEntryList *elistsum = new TEntryList("elistsum", "elistsum");
   elistsum->Add(elist1);
   elistsum->Add(elist2);

   TEntryList *elistcheck = new TEntryList("elistcheck", "elistcheck");
   chain->Draw(">>elistcheck", cut1 || cut2, "entrylist");

   Int_t n=elistcheck->GetN();
   Long64_t entry1, entry2;
   wrongentries1=0;
   for (Int_t i=0; i<n; i++){
      entry1 = elistsum->GetEntry(i);
      entry2 = elistcheck->GetEntry(i);
      if (entry1 != entry2) {
         //printf("%d, sum=%lld, check=%lld\n", i, entry1, entry2);
         wrongentries1++;
      }
   }
   if (wrongentries1>0)
      printf("\nwrong entries (1+2)=%d\n", wrongentries1);
   
   //add in different order
   TEntryList *elistsum2 = new TEntryList("elistsum2", "elistsum2");
   elistsum2->Add(elist2);
   elistsum2->Add(elist1);
   wrongentries2 = 0;
   for (Int_t i=0; i<n; i++){
      entry1 = elistsum2->GetEntry(i);
      entry2 = elistcheck->GetEntry(i);
      if (entry1 != entry2) {
         //printf("%d, sum=%lld, check=%lld\n", i, entry1, entry2);
         wrongentries2++;
      }
    }
    if (wrongentries2>0)
      printf("\nwrong entries (2+1)=%d\n", wrongentries2);


   //add by using "+" in TTree::Draw
   TEntryList *elistsum3 = new TEntryList("elistsum3", "elistsum3");
   chain->Draw(">>elistsum3", cut1, "entrylist");
   chain->Draw(">>+elistsum3", cut2, "entrylist");
   wrongentries3 = 0;
   for (Int_t i=0; i<n; i++){
      entry1 = elistsum3->GetEntry(i);
      entry2 = elistcheck->GetEntry(i);
      if (entry1 != entry2) {
         //printf("%d, sum=%lld, check=%lld\n", i, entry1, entry2);
         wrongentries3++;
      }
    }
   if (wrongentries3>0)
      printf("\nwrong entries with \"+\" in TChain::Draw =%d\n", wrongentries3);

   //subtract the second list
   elistsum->Subtract(elist2);
   n = elistsum->GetN();
   TEntryList *elistcheck2 = new TEntryList("elistcheck2","elistcheck2");
   chain->Draw(">>elistcheck2", "x>0 && (y>0.1 || y<-0.1)", "entrylist");
   
   wrongentries4 = 0;
   for (Int_t i=0; i<n; i++){
      entry1 = elistsum->GetEntry(i);
      entry2 = elistcheck2->GetEntry(i);
      if (entry1 != entry2){
          //printf("%d elist1=%lld elistsum=%lld\n", i, entry1, entry2);
         wrongentries4++;
      }
   }
   if (wrongentries4>0)
      printf("\nwrong entries after subtract 2 = %d\n", wrongentries4);

   //subtract the first list
   elistsum2->Subtract(elist1);
   elistcheck2->Reset();
   chain->Draw(">>elistcheck2", "x<0 && y<0.1 && y>-0.1", "entrylist");
   wrongentries5 = 0;
   n = elistcheck2->GetN();
   for (Int_t i=0; i<n; i++){
      entry1 = elistsum2->GetEntry(i);
      entry2 = elistcheck2->GetEntry(i);
      if (entry1 != entry2){
         //printf("%d elist1=%lld elistsum=%lld\n", i, entry1, entry2);
         wrongentries5++;
      }
   }
   if (wrongentries5>0)
      printf("\nwrong entries after subtract 1 = %d\n", wrongentries5);

   delete elist1;
   delete elist2;
   delete elistsum;
   delete elistsum2;
   delete elistsum3;
   delete elistcheck;
   delete elistcheck2;

   if (wrongentries1>0 || wrongentries2>0 || wrongentries3>0 || wrongentries4>0 || wrongentries5>0)
      return kFALSE;
   return kTRUE;
}

Bool_t Test3()
{
   //Test correspondance of event lists and entry lists

   TChain *chain = new TChain("chain", "chain");
   chain->Add("stressEntryListTrees*.root/tree1");
   chain->Add("stressEntryListTrees*.root/tree2");

   TCut cut = "x<0 && y>0";
   chain->Draw(">>evlist", cut, "");
   TEventList *evlist = (TEventList*)gDirectory->Get("evlist");

   chain->Draw("x>>h1", cut, "goff");
   TH1F *h1 = (TH1F*)gDirectory->Get("h1");
   chain->SetEventList(evlist);
   chain->Draw("x>>h2", "", "goff");
   TH1F *h2 = (TH1F*)gDirectory->Get("h2");

   chain->SetEventList(0);
   chain->Draw(">>enlist", cut, "entrylist");
   TEntryList *enlist = (TEntryList*)gDirectory->Get("enlist");
   chain->SetEntryList(enlist);

   chain->Draw("x>>h3", "", "goff");
   TH1F *h3 = (TH1F*)gDirectory->Get("h3");

   Int_t wrongbins = 0;
   Int_t nbins1 = h1->GetNbinsX();

   Double_t bin1,bin2,bin3;
   for (Int_t i=0; i<nbins1; i++){
      bin1 = h1->GetBinContent(i);
      bin2 = h2->GetBinContent(i);
      bin3 = h3->GetBinContent(i);
      if (TMath::Abs(bin1-bin2) > 0.1 || TMath::Abs(bin1-bin3) || TMath::Abs(bin2-bin3) > 0.1) {
         //printf("bin1=%f, bin2=%f, bin3=%f\n", bin1, bin2, bin3);
         wrongbins++;
      }
   }
   if (wrongbins>0)
      printf("wrongbins=%d\n", wrongbins);

   delete chain;
   delete h1;
   delete h2;
   delete h3;
   delete evlist;
   delete enlist;
   if (wrongbins>0)
      return kFALSE;
   return kTRUE;
}

Bool_t Test4()
{
   //Like Test3() but for trees

   TFile f("stressEntryListTrees_0.root");
   TTree *tree = (TTree*)f.Get("tree1");
   TCut cut = "x<0 && y>0";
   tree->Draw(">>evlist", cut, "");
   TEventList *evlist = (TEventList*)gDirectory->Get("evlist");
   tree->Draw("x>>h1", cut, "goff");
   TH1F *h1 = (TH1F*)gDirectory->Get("h1");
   tree->SetEventList(evlist);
   tree->Draw("x>>h2", "", "goff");
   TH1F *h2 = (TH1F*)gDirectory->Get("h2");

   tree->SetEventList(0);
   tree->Draw(">>enlist", cut, "entrylist");
   TEntryList *enlist = (TEntryList*)gDirectory->Get("enlist");
   tree->SetEntryList(enlist);
   tree->Draw("x>>h3", "", "goff");
   TH1F *h3 = (TH1F*)gDirectory->Get("h3");
   Int_t wrongbins = 0;
   Int_t nbins1 = h1->GetNbinsX();

   Double_t bin1,bin2,bin3;
   for (Int_t i=0; i<nbins1; i++){
      bin1 = h1->GetBinContent(i);
      bin2 = h2->GetBinContent(i);
      bin3 = h3->GetBinContent(i);
      if (TMath::Abs(bin1-bin2) > 0.1 || TMath::Abs(bin1-bin3) || TMath::Abs(bin2-bin3) > 0.1) {
         //printf("bin1=%f, bin2=%f, bin3=%f\n", bin1, bin2, bin3);
         wrongbins++;
      }
   }
   if (wrongbins>0)
      printf("wrongbins=%d\n", wrongbins);

   delete h1;
   delete h2;
   delete h3;
   delete evlist;
   delete enlist;
   f.Close();

   if (wrongbins>0)
      return kFALSE;
   return kTRUE;
}


void MakeTrees(Int_t nentries, Int_t nfiles)
{
   //Creates nfiles files with 2 trees of nentries each

   TFile *f1;
   TTree *tree1, *tree2;

   Double_t x, y, z;
   Int_t range = nentries/100;

   char buffer[50];
   for (Int_t ifile=0; ifile<nfiles; ifile++){
      sprintf(buffer, "stressEntryListTrees_%d.root", ifile);
      f1 = new TFile(buffer, "UPDATE");
      tree1 = new TTree("tree1", "tree1");
      tree1->Branch("x", &x, "x/D");
      tree1->Branch("y", &y, "y/D");
      tree1->Branch("z", &z, "z/D");
      tree2 = new TTree("tree2", "tree2");
      tree2->Branch("x", &x, "x/D");
      tree2->Branch("y", &y, "y/D");
      tree2->Branch("z", &z, "z/D");
      for (Int_t i=0; i<nentries; i++){
         x = gRandom->Uniform(-range, range);
         y = gRandom->Uniform(-range, range);
         z = gRandom->Uniform(-range, range);
         tree1->Fill();
         x = gRandom->Uniform(-range, range);
         y = gRandom->Uniform(-range, range);
         z = gRandom->Uniform(-range, range);
         tree2->Fill();
      }
      tree1->Write();
      tree2->Write();

      f1->Write();
      f1->Close();
   }

}

void CleanUp(Int_t nfiles)
{
   char buffer[50];
   for (Int_t i=0; i<nfiles; i++){
      sprintf(buffer, "stressEntryListTrees_%d.root", i);
      gSystem->Unlink(buffer);
   }
}

Int_t stressEntryList(Int_t nentries, Int_t nfiles)
{
   //Int_t nentries = 1000;
   //Int_t nfiles = 10;
   MakeTrees(nentries, nfiles);
   printf("**********************************************************************\n");
   printf("***************Starting TEntryList stress test************************\n");
   printf("**********************************************************************\n");
   printf("**********Generating %d data files, 2 trees of %d in each**********\n", nfiles, nentries);
   printf("**********************************************************************\n");

   Bool_t ok1=kTRUE;
   Bool_t ok2=kTRUE;
   Bool_t ok3=kTRUE;
   Bool_t ok4=kTRUE;

   ok1 = Test1();
   if (ok1)
      printf("Test1: Applying different entry lists to different chains --------- OK\n");
   else{
      printf("Test1: Applying different entry lists to different chains --------- FAILED\n"); 

   }
   ok2 = Test2();
   if (ok2)
      printf("Test2: Adding and subtracting entry lists-------------------------- OK\n");
   else{
      printf("Test1: Adding and subtracting entry lists-------------------------- FAILED\n");
   }

   ok3 = Test3();
   if (ok3)
      printf("Test3: TEntryList and TEventList for TChain------------------------ OK\n");
   else{
      printf("Test3: TEntryList and TEventList for TChain------------------------ FAILED\n");
   }

   ok4 = Test4();
   if (ok4)  
      printf("Test4: TEntryList and TEventList for TTree------------------------- OK\n");
   else{
      printf("Test4: TEntryList and TEventList for TTree------------------------- FAILED\n");
   }

   printf("**********************************************************************\n");
   printf("*******************Deleting the data files****************************\n");
   printf("**********************************************************************\n");
   CleanUp(nfiles);
   return 0;
}
//_____________________________batch only_____________________
#ifndef __CINT__

int main(int argc, char *argv[]) 
{
   TApplication theApp("App", &argc, argv);
   Int_t nentries = 10000;
   Int_t nfiles = 10;
   if (argc > 1) nentries = atoi(argv[1]);
   if (argc > 2) nfiles = atoi(argv[2]);
   stressEntryList(nentries, nfiles);
   return 0;
}

#endif

