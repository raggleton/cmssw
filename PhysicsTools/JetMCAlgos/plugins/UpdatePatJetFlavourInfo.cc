/**
  \class
  \brief    Update JetFlavourInfo in pat::Jet collection from a JetFlavourInfoMatchingCollection

  \author   Robin Aggleton, robin.aggleton@cern.ch
*/


#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/Common/interface/RefToPtr.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "SimDataFormats/JetMatching/interface/JetFlavourInfoMatching.h"


namespace pat {

  class UpdatePatJetFlavourInfo : public edm::stream::EDProducer<> {
  public:
    explicit UpdatePatJetFlavourInfo(const edm::ParameterSet & iConfig);
    ~UpdatePatJetFlavourInfo() override { }

    void produce(edm::Event & iEvent, const edm::EventSetup & iSetup) override;

  private:
    const edm::EDGetTokenT<edm::View<pat::Jet> > patJetsToken_;
    const edm::EDGetTokenT<edm::View<reco::Jet> > recoJetsToken_;
    const edm::EDGetTokenT<reco::JetFlavourInfoMatchingCollection> jetFlavourInfosToken_;
  };

}

pat::UpdatePatJetFlavourInfo::UpdatePatJetFlavourInfo(const edm::ParameterSet & iConfig) :
    patJetsToken_(consumes<edm::View<pat::Jet> >(iConfig.getParameter<edm::InputTag>("jetSrc"))),
    recoJetsToken_(consumes<edm::View<reco::Jet> >(iConfig.getParameter<edm::InputTag>("jetSrc"))),
    jetFlavourInfosToken_(consumes<reco::JetFlavourInfoMatchingCollection>(iConfig.getParameter<edm::InputTag>("jetFlavourInfos")))
{
    produces<pat::JetCollection>();
}

void
pat::UpdatePatJetFlavourInfo::produce(edm::Event & iEvent, const edm::EventSetup & iSetup) {
    using namespace edm;
    using namespace std;

    // we have 2 handles for the same collection
    // one is for a pat::Jet view, to get a copy of the pat::Jet
    // the other is a reco::Jet view, to get the ref used in the JetFlavourInfoMatchingCollection keys
    // (can't use the pat::Jet ref it seems)
    // this only works so long as the jetSrc used here is the same as the one
    // in the JetFlavourClustering module that made the JetFlavourInfoMatchingCollection
    Handle<View<pat::Jet> > patJets;
    iEvent.getByToken(patJetsToken_, patJets);

    Handle<View<reco::Jet> > recoJets;
    iEvent.getByToken(recoJetsToken_, recoJets);

    Handle<reco::JetFlavourInfoMatchingCollection> jetFlavourInfos;
    iEvent.getByToken(jetFlavourInfosToken_, jetFlavourInfos);

    std::auto_ptr< pat::JetCollection > updatedJets ( new pat::JetCollection() );
    updatedJets->reserve(patJets->size());

    for (unsigned int i=0; i<patJets->size(); ++i) {
      pat::Jet newJet(patJets->at(i));
      newJet.setJetFlavourInfo((*jetFlavourInfos)[recoJets->refAt(i)]);
      updatedJets->push_back(newJet);
    }
    iEvent.put(updatedJets);
}

#include "FWCore/Framework/interface/MakerMacros.h"
using namespace pat;
DEFINE_FWK_MODULE(UpdatePatJetFlavourInfo);
