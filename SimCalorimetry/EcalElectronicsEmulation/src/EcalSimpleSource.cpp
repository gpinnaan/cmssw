#include "SimCalorimetry/EcalElectronicsEmulation/interface/EcalSimpleSource.h"
#include "TFormula.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/EcalDigi/interface/EBDataFrame.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Utilities/interface/Exception.h"

#include <iostream>
#include <string>

using namespace std;

bool EcalSimpleSource::produce(edm::Event& evt){
  if(formula_.get()!=0){
    auto_ptr<EBDigiCollection> digis
      = auto_ptr<EBDigiCollection>(new EBDigiCollection);

    digis->reserve(170*360);

    const int nSamples = 10;
    const int ievt = event();
    for(int iEta0=0; iEta0<170; ++iEta0){
      for(int iPhi0=0; iPhi0<360; ++iPhi0){
	int iEta1 = cIndex2iEta(iEta0);
	int iPhi = cIndex2iPhi(iPhi0);
	if(verbose_) cout << "(" << iEta0 << "," << iPhi0 << "): ";
	
       EBDataFrame dframe(EBDetId(iEta1,iPhi));
       //cIndex2iPhi(iPhi0)));
       dframe.setSize(nSamples);
       
       for(int t = 0; t < nSamples; ++t){
	 uint16_t encodedAdc = (uint16_t)formula_->Eval(iEta0, iPhi0, ievt-1, t);
	 
	 if(verbose_) cout << encodedAdc << ((t<(nSamples-1))?"\t":"\n");
	 dframe.setSample(t, EcalMGPASample(encodedAdc));
       }
       digis->push_back(dframe);
      }
   }
    evt.put(digis);
  }
  if(tpFormula_.get()!=0){
    auto_ptr<EcalTrigPrimDigiCollection> tps
      = auto_ptr<EcalTrigPrimDigiCollection>(new EcalTrigPrimDigiCollection);
    tps->reserve(56*72);
    const int nSamples = 5;
    const int ievt = event();
    for(int iTtEta0=0; iTtEta0<56; ++iTtEta0){
      for(int iTtPhi0=0; iTtPhi0<72; ++iTtPhi0){
	int iTtEta1 = cIndex2iTtEta(iTtEta0);
	int iTtPhi = cIndex2iTtPhi(iTtPhi0);

	if(verbose_) cout << "(" << iTtEta0 << "," << iTtPhi0 << "): ";
	int zside = iTtEta1<0?-1:1;
	EcalTriggerPrimitiveDigi
	  tpframe(EcalTrigTowerDetId(zside, EcalTriggerTower,
				     abs(iTtEta1), iTtPhi));
	
	tpframe.setSize(nSamples);
	
       if(verbose_) cout << "TP: ";
       for(int t = 0; t < nSamples; ++t){
	 uint16_t encodedTp = (uint16_t)tpFormula_->Eval(iTtEta0, iTtPhi0, ievt-1, t);
	 
	 if(verbose_) cout << "TP(" << iTtEta0 << "," << iTtPhi0 << ") = "
			  << encodedTp << ((t<(nSamples-1))?"\t":"\n");
	 tpframe.setSample(t, EcalTriggerPrimitiveSample(encodedTp));
       }
       tps->push_back(tpframe);
      }
   }
    evt.put(tps);
  }
  return true;
}

EcalSimpleSource::EcalSimpleSource(const edm::ParameterSet& pset,
				   const edm::InputSourceDescription& sdesc):
  GeneratedInputSource(pset, sdesc){
  string formula = pset.getParameter<string>("formula");
  string tpFormula = pset.getParameter<string>("tpFormula");
  verbose_ = pset.getUntrackedParameter<bool>("verbose", false);
  replaceAll(formula, "itt0", "(ieta0%85)*20+((iphi0+10)%20)");
  replaceAll(formula, "ieta0", "x");
  replaceAll(formula, "iphi0", "y");
  replaceAll(formula, "ievt0", "z");
  replaceAll(formula, "isample0", "t");
  //  cout << "----------> " << formula << endl;

  replaceAll(tpFormula, "itt0", "((ieta0-11+17)%17)*4+(iphi0+2)%4");
  replaceAll(tpFormula, "ieta0", "x");
  replaceAll(tpFormula, "iphi0", "y");
  replaceAll(tpFormula, "ievt0", "z");
  replaceAll(tpFormula, "isample0", "t");
  //cout << "----------> " << tpFormula << endl;

  if(formula.size()!=0){
    formula_ = auto_ptr<TFormula>(new TFormula("f", formula.c_str()));
    Int_t err = formula_->Compile();
    if(err!=0){
      throw cms::Exception("Error in EcalSimpleSource 'formula' config.");
    }
    produces<EBDigiCollection>();
  }
  if(tpFormula.size()!=0){
    tpFormula_ = auto_ptr<TFormula>(new TFormula("f", tpFormula.c_str()));
    Int_t err = tpFormula_->Compile();
    if(err!=0){
      throw cms::Exception("Error in EcalSimpleSource 'tpFormula' config.");
    }
    produces<EcalTrigPrimDigiCollection>();
  }
};

void EcalSimpleSource::replaceAll(std::string& s, const std::string& from,
				  const std::string& to) const{
  string::size_type pos = 0;
  //  cout << "replaceAll(" << s << "," << from << "," << to << ")\n";
  while((pos=s.find(from, pos))!=string::npos){
    // cout << "replace(" << pos << "," << from.size() << "," << to << ")\n";
    s.replace(pos, from.size(), to);
    //   cout << " -> " << s << "\n"; 
  }
}
