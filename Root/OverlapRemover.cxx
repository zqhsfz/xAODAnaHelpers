/*************************************************
 *
 * Interface to ASG overlap removal tool
 ( applying recommendations from Harmonisation TF ).
 *
 * M. Milesi (marco.milesi@cern.ch)
 *
 ************************************************/

// c++ include(s):
#include <iostream>
#include <sstream>

// EL include(s):
#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>

// EDM include(s):
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/Photon.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/Muon.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/Jet.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJet.h"
#include "AthContainers/ConstDataVector.h"
#include "xAODCore/ShallowCopy.h"

// package include(s):
#include "xAODAnaHelpers/OverlapRemover.h"
#include "xAODAnaHelpers/HelperFunctions.h"
#include "xAODAnaHelpers/HelperClasses.h"

#include <xAODAnaHelpers/tools/ReturnCheck.h>

// ROOT include(s):
#include "TEnv.h"
#include "TSystem.h"

using HelperClasses::ToolName;

// this is needed to distribute the algorithm to the workers
ClassImp(OverlapRemover)


OverlapRemover :: OverlapRemover (std::string className) :
    Algorithm(className),
    m_useElectrons(false),
    m_useMuons(false),
    m_usePhotons(false),
    m_useTaus(false),
    m_dummyElectronContainer(nullptr),
    m_dummyMuonContainer(nullptr),
    m_overlapRemovalTool(nullptr),
    m_el_cutflowHist_1(nullptr),
    m_mu_cutflowHist_1(nullptr),
    m_jet_cutflowHist_1(nullptr),
    m_ph_cutflowHist_1(nullptr),
    m_tau_cutflowHist_1(nullptr)
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().

  Info("OverlapRemover()", "Calling constructor");

  // read debug flag from .config file
  m_debug         = false;
  m_useCutFlow    = true;

  // input container(s) to be read from TEvent or TStore

  /* Muons */
  m_inContainerName_Muons       = "";
  m_inputAlgoMuons              = "";  // name of vector<string> of syst retrieved from TStore
  m_outputAlgoMuons             = "MuonCollection_OR_Algo";    // name of vector<string> of syst pushed in TStore
                                                               // NB: This is practically useless for the OR, since OR does not skim events
                                                               // (unlike the Selectors)!
                                                               // --> output syst set will be equal to input syst set, no matter what.
                                                               // KEEP THIS ATTRIBUTE ONLY FOR BW COMPATIBILITY

  /* Electrons */
  m_inContainerName_Electrons   = "";
  m_inputAlgoElectrons          = "";  // name of vector<string> of syst retrieved from TStore
  m_outputAlgoElectrons         = "ElectronCollection_OR_Algo";    // name of vector<string> of syst pushed in TStore
                                                                   // NB: This is practically useless for the OR, since OR does not skim events
                                                                   // (unlike the Selectors)!
                                                                   // --> output syst set will be equal to input syst set, no matter what.
                                                                   // KEEP THIS ATTRIBUTE ONLY FOR BW COMPATIBILITY

  /* Jets */
  m_inContainerName_Jets        = "";
  m_inputAlgoJets               = "";  // name of vector<string> of syst retrieved from TStore
  m_outputAlgoJets              = "JetCollection_OR_Algo";    // name of vector<string> of syst pushed in TStore
                                                              // NB: This is practically useless for the OR, since OR does not skim events
                                                              // (unlike the Selectors)!
                                                              // --> output syst set will be equal to input syst set, no matter what.
                                                              // KEEP THIS ATTRIBUTE ONLY FOR BW COMPATIBILITY

  /* Photons */
  m_inContainerName_Photons     = "";
  m_inputAlgoPhotons            = "";  // name of vector<string> of syst retrieved from TStore
  m_outputAlgoPhotons           = "PhotonCollection_OR_Algo";    // name of vector<string> of syst pushed in TStore

  /* Taus */
  m_inContainerName_Taus        = "";
  m_inputAlgoTaus               = "";  // name of vector<string> of syst retrieved from TStore
  m_outputAlgoTaus              = "TauCollection_OR_Algo";    // name of vector<string> of syst pushed in TStore
                                                              // NB: This is practically useless for the OR, since OR does not skim events
                                                              // (unlike the Selectors)!
                                                              // --> output syst set will be equal to input syst set, no matter what.
                                                              // KEEP THIS ATTRIBUTE ONLY FOR BW COMPATIBILITY

  // decorate selected objects that pass the cuts
  m_decorateSelectedObjects     = true;
  // additional functionality : create output container of selected objects
  //                            using the SG::View_Element option
  //                            decorating and output container should not be mutually exclusive
  m_createSelectedContainers    = false;

  m_useSelected = false;

  m_outContainerName_Electrons  = "";

  m_outContainerName_Muons      = "";

  m_outContainerName_Jets       = "";

  m_outContainerName_Photons    = "";

  m_outContainerName_Taus       = "";

}

EL::StatusCode  OverlapRemover :: configure ()
{

  if ( !getConfig().empty() ) {

    Info("configure()", "Configuing OverlapRemover Interface. User configuration read from : %s ", getConfig().c_str());

    TEnv* config = new TEnv(getConfig(true).c_str());

    // read debug flag from .config file
    m_debug         = config->GetValue("Debug" , m_debug);
    m_useCutFlow    = config->GetValue("UseCutFlow",  m_useCutFlow);

    // input container(s) to be read from TEvent or TStore

    /* Muons */
    m_inContainerName_Muons       = config->GetValue("InputContainerMuons",  m_inContainerName_Muons.c_str());
    m_inputAlgoMuons              = config->GetValue("InputAlgoMuons",  m_inputAlgoMuons.c_str());  // name of vector<string> of syst retrieved from TStore
    m_outputAlgoMuons             = config->GetValue("OutputAlgoMuons", m_outputAlgoMuons.c_str());    // name of vector<string> of syst pushed in TStore
    /* Electrons */
    m_inContainerName_Electrons   = config->GetValue("InputContainerElectrons",  m_inContainerName_Electrons.c_str());
    m_inputAlgoElectrons          = config->GetValue("InputAlgoElectrons",  m_inputAlgoElectrons.c_str());  // name of vector<string> of syst retrieved from TStore
    m_outputAlgoElectrons         = config->GetValue("OutputAlgoElectrons", m_outputAlgoElectrons.c_str());    // name of vector<string> of syst pushed in TStore
    /* Jets */
    m_inContainerName_Jets        = config->GetValue("InputContainerJets",  m_inContainerName_Jets.c_str());
    m_inputAlgoJets               = config->GetValue("InputAlgoJets",  m_inputAlgoJets.c_str());  // name of vector<string> of syst retrieved from TStore
    m_outputAlgoJets              = config->GetValue("OutputAlgoJets", m_outputAlgoJets.c_str());    // name of vector<string> of syst pushed in TStore
    /* Photons */
    m_inContainerName_Photons     = config->GetValue("InputContainerPhotons",  m_inContainerName_Photons.c_str());
    m_inputAlgoPhotons            = config->GetValue("InputAlgoPhotons",  m_inputAlgoPhotons.c_str());  // name of vector<string> of syst retrieved from TStore
    m_outputAlgoPhotons           = config->GetValue("OutputAlgoPhotons", m_outputAlgoPhotons.c_str());    // name of vector<string> of syst pushed in TStore
    /* Taus */
    m_inContainerName_Taus        = config->GetValue("InputContainerTaus",  m_inContainerName_Taus.c_str());
    m_inputAlgoTaus               = config->GetValue("InputAlgoTaus",  m_inputAlgoTaus.c_str());  // name of vector<string> of syst retrieved from TStore
    m_outputAlgoTaus              = config->GetValue("OutputAlgoTaus", m_outputAlgoTaus.c_str());    // name of vector<string> of syst pushed in TStore

    // decorate selected objects that pass the cuts
    m_decorateSelectedObjects     = config->GetValue("DecorateSelectedObjects", m_decorateSelectedObjects);
    // additional functionality : create output container of selected objects
    //                            using the SG::View_Element option
    //                            decorating and output container should not be mutually exclusive
    m_createSelectedContainers    = config->GetValue("CreateSelectedContainers", m_createSelectedContainers);

    m_useSelected = config->GetValue("UseSelected", m_useSelected);

    m_outContainerName_Electrons  = config->GetValue("OutputContainerElectrons", m_outContainerName_Electrons.c_str());

    m_outContainerName_Muons      = config->GetValue("OutputContainerMuons", m_outContainerName_Muons.c_str());

    m_outContainerName_Jets       = config->GetValue("OutputContainerJets", m_outContainerName_Jets.c_str());

    m_outContainerName_Photons    = config->GetValue("OutputContainerPhotons", m_outContainerName_Photons.c_str());

    m_outContainerName_Taus       = config->GetValue("OutputContainerTaus", m_outContainerName_Taus.c_str());

    config->Print();
    Info("configure()", "OverlapRemover Interface succesfully configured! ");

    delete config; config = nullptr;
  }

  if ( m_inContainerName_Jets.empty() ) {
    Error("configure()", "InputContainerJets is empty! Must have it to perform Overlap Removal! Exiting.");
    return EL::StatusCode::FAILURE;
  }

  // be more flexible w/ electrons, muons, photons and taus :)
  if ( !m_inContainerName_Electrons.empty() ) {
    m_useElectrons = true;
  } else{
    m_dummyElectronContainer = new xAOD::ElectronContainer();
  }

  if ( !m_inContainerName_Muons.empty() )     {
    m_useMuons     = true;
  } else {
    m_dummyMuonContainer = new xAOD::MuonContainer();
  }

  if ( !m_inContainerName_Photons.empty() )   { m_usePhotons   = true; }
  if ( !m_inContainerName_Taus.empty() )      { m_useTaus      = true; }
  m_outAuxContainerName_Electrons   = m_outContainerName_Electrons + "Aux."; // the period is very important!
  m_outAuxContainerName_Muons       = m_outContainerName_Muons + "Aux.";     // the period is very important!
  m_outAuxContainerName_Jets        = m_outContainerName_Jets + "Aux.";      // the period is very important!
  m_outAuxContainerName_Photons     = m_outContainerName_Photons + "Aux.";   // the period is very important!
  m_outAuxContainerName_Taus        = m_outContainerName_Taus + "Aux.";      // the period is very important!

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode OverlapRemover :: setupJob (EL::Job& job)
{
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.

  Info("setupJob()", "Calling setupJob");

  job.useXAOD ();
  xAOD::Init( "OverlapRemover" ).ignore(); // call before opening first file

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode OverlapRemover :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  RETURN_CHECK("xAH::Algorithm::algInitialize()", xAH::Algorithm::algInitialize(), "");
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode OverlapRemover :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode OverlapRemover :: changeInput (bool /*firstFile*/)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode OverlapRemover :: initialize ()
{
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.

  Info("initialize()", "Initializing OverlapRemover Interface... ");

  if ( setCutFlowHist() == EL::StatusCode::FAILURE ) {
    Error("initialize()", "Failed to setup cutflow histograms. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  m_event = wk()->xaodEvent();
  m_store = wk()->xaodStore();

  Info("initialize()", "Number of events in file: %lld ", m_event->getEntries() );

  if ( configure() == EL::StatusCode::FAILURE ) {
    Error("initialize()", "Failed to properly configure. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  if ( setCounters() == EL::StatusCode::FAILURE ) {
    Error("initialize()", "Failed to properly set event/object counters. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  // initialize ASG overlap removal tool
  //
  m_overlapRemovalTool = new OverlapRemovalTool( "OverlapRemovalTool" );
  m_overlapRemovalTool->msg().setLevel( MSG::INFO ); // VERBOSE, INFO, DEBUG

  // set input object "selection" decoration
  //
  const std::string selected_label = ( m_useSelected ) ? "passSel" : "";  // set with decoration flag you use for selected objects if want to consider only selected objects in OR, otherwise it will perform OR on all objects
  RETURN_CHECK( "OverlapRemover::initialize()", m_overlapRemovalTool->setProperty("InputLabel",  selected_label), "");
  RETURN_CHECK( "OverlapRemover::initialize()", m_overlapRemovalTool->setProperty("OverlapLabel", "overlaps"), "Failed to set property OverlapLabel"); // tool will decorate objects with 'overlaps' boolean if they overlap
  RETURN_CHECK( "OverlapRemover::initialize()", m_overlapRemovalTool->initialize(), "Failed to properly initialize the OverlapRemovalTool.");

  Info("initialize()", "OverlapRemover Interface succesfully initialized!" );

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode OverlapRemover :: execute ()
{
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.

  if ( m_debug ) { Info("execute()", "Applying Overlap Removal... "); }

  m_numEvent++;

  // get the collections from TEvent or TStore
  const xAOD::ElectronContainer* inElectrons (nullptr);
  const xAOD::MuonContainer* inMuons         (nullptr);
  const xAOD::JetContainer* inJets           (nullptr);
  const xAOD::PhotonContainer* inPhotons     (nullptr);
  const xAOD::TauJetContainer* inTaus        (nullptr);

  // --------------------------------------------------------------------------------------------
  //
  // always run the nominal case

  executeOR(inElectrons, inMuons, inJets, inPhotons, inTaus, NOMINAL);

  // look what do we have in TStore
  if ( m_verbose ) { m_store->print(); }

  // -----------------------------------------------------------------------------------------------
  //
  // if at least one of the m_inputAlgo* is not empty, and there's at least one non-empty syst name,
  // then perform OR for every non-empty systematic set

  // ****************** //
  //      Electrons     //
  // ****************** //

  if ( !m_inputAlgoElectrons.empty() ) {

    // -----------------------
    //
    // get the systematic sets:

    // get vector of string giving the syst names (rememeber: 1st element is a blank string: nominal case!)
    std::vector<std::string>* systNames_el(nullptr);
    RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(systNames_el, m_inputAlgoElectrons, 0, m_store, m_verbose) ,"");

    if ( HelperFunctions::found_non_dummy_sys(systNames_el) ) {
      executeOR(inElectrons, inMuons, inJets, inPhotons, inTaus,  ELSYST, systNames_el);
    }

  }

  // **************** //
  //      Muons       //
  // **************** //

  if ( !m_inputAlgoMuons.empty() ) {

    // -----------------------
    //
    // get the systematic sets:

    // get vector of string giving the syst names (rememeber: 1st element is a blank string: nominal case!)
    std::vector<std::string>* systNames_mu(nullptr);
    RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(systNames_mu, m_inputAlgoMuons, 0, m_store, m_verbose) ,"");

    if ( HelperFunctions::found_non_dummy_sys(systNames_mu) ) {
      executeOR(inElectrons, inMuons, inJets, inPhotons, inTaus,  MUSYST, systNames_mu);
    }

  }

  // **************** //
  //       Jets       //
  // **************** //

  if ( !m_inputAlgoJets.empty() ) {

    // -----------------------
    //
    // get the systematic sets:

    // get vector of string giving the syst names (rememeber: 1st element is a blank string: nominal case!)
    std::vector<std::string>* systNames_jet;
    RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(systNames_jet, m_inputAlgoJets, 0, m_store, m_verbose) ,"");

    if ( HelperFunctions::found_non_dummy_sys(systNames_jet) ) {
      executeOR(inElectrons, inMuons, inJets, inPhotons, inTaus,  JETSYST, systNames_jet);
    }

  }

  // **************** //
  //     Photons      //
  // **************** //

  if ( !m_inputAlgoPhotons.empty() ) {

    // -----------------------
    //
    // get the systematic sets:

    // get vector of string giving the syst names (rememeber: 1st element is a blank string: nominal case!)
    std::vector<std::string>* systNames_photon;
    RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(systNames_photon, m_inputAlgoPhotons, 0, m_store, m_verbose) ,"");

    executeOR(inElectrons, inMuons, inJets, inPhotons, inTaus,  PHSYST, systNames_photon);

  }

  // **************** //
  //       Taus       //
  // **************** //

  if ( !m_inputAlgoTaus.empty() ) {

    // -----------------------
    //
    // get the systematic sets:

    // get vector of string giving the syst names (rememeber: 1st element is a blank string: nominal case!)
    std::vector<std::string>* systNames_tau;
    RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(systNames_tau, m_inputAlgoTaus, 0, m_store, m_verbose) ,"");

    executeOR(inElectrons, inMuons, inJets, inPhotons, inTaus, TAUSYST, systNames_tau);

  }

  // look what do we have in TStore
  if ( m_verbose ) { m_store->print(); }

  return EL::StatusCode::SUCCESS;

}

EL::StatusCode OverlapRemover :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.

  if ( m_debug ) { Info("postExecute()", "Calling postExecute"); }

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode OverlapRemover :: finalize ()
{
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.

  Info("finalize()", "Deleting tool instances...");

  if ( m_overlapRemovalTool    ){ delete m_overlapRemovalTool;     m_overlapRemovalTool     = nullptr; }
  if ( m_dummyElectronContainer){ delete m_dummyElectronContainer; m_dummyElectronContainer = nullptr; }
  if ( m_dummyMuonContainer    ){ delete m_dummyMuonContainer;     m_dummyMuonContainer     = nullptr; }

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode OverlapRemover :: histFinalize ()
{
  // This method is the mirror image of histInitialize(), meaning it
  // gets called after the last event has been processed on the worker
  // node and allows you to finish up any objects you created in
  // histInitialize() before they are written to disk.  This is
  // actually fairly rare, since this happens separately for each
  // worker node.  Most of the time you want to do your
  // post-processing on the submission node after all your histogram
  // outputs have been merged.  This is different from finalize() in
  // that it gets called on all worker nodes regardless of whether
  // they processed input events.

  Info("histFinalize()", "Calling histFinalize");
  RETURN_CHECK("xAH::Algorithm::algFinalize()", xAH::Algorithm::algFinalize(), "");
  return EL::StatusCode::SUCCESS;
}


EL::StatusCode OverlapRemover :: fillObjectCutflow (const xAOD::IParticleContainer* objCont, const std::string& overlapFlag, const std::string& selectFlag)
{
  SG::AuxElement::ConstAccessor<char> selectAcc(selectFlag);
  SG::AuxElement::ConstAccessor<char> overlapAcc(overlapFlag);
  static SG::AuxElement::ConstAccessor< ElementLink<xAOD::IParticleContainer> > objLinkAcc("overlapObject");

  for ( auto obj_itr : *(objCont) ) {

    std::string type;

    // Safety check
    //
    if ( !overlapAcc.isAvailable( *obj_itr ) ) {
      Error("fillObjectCutflow()", "Overlap decoration missing for this object");
      return EL::StatusCode::FAILURE;
    }
    if ( !overlapAcc( *obj_itr ) ) {
      switch(obj_itr->type())
	{
	case xAOD::Type::Electron:
	  m_el_cutflowHist_1->Fill( m_el_cutflow_OR_cut, 1 );
	  type = "electron";
	  break;
	case xAOD::Type::Muon:
	  m_mu_cutflowHist_1->Fill( m_mu_cutflow_OR_cut, 1 );
	  type = "muon";
	  break;
	case xAOD::Type::Jet:
	  m_jet_cutflowHist_1->Fill( m_jet_cutflow_OR_cut, 1 );
	  type = "jet";
	  break;
	case xAOD::Type::Photon:
	  m_ph_cutflowHist_1->Fill( m_ph_cutflow_OR_cut, 1 );
	  type = "photon";
	  break;
	case xAOD::Type::Tau:
	  m_tau_cutflowHist_1->Fill( m_tau_cutflow_OR_cut, 1 );
	  type = "tau";
	  break;
	default:
	  Error("OverlapRemover::fillObjectCutflow()","Unsupported object");
	  return EL::StatusCode::FAILURE;
	  break;
	}
    }

    if ( m_debug ) {
      if ( selectAcc.isAvailable( *obj_itr) ){
        Info("fillObjectCutflow()", "  %s pt %6.2f eta %5.2f phi %5.2f selected %i overlaps %i ", type.c_str(), (obj_itr)->pt()/1000., (obj_itr)->eta(), (obj_itr)->phi(), selectAcc( *obj_itr ), overlapAcc( *obj_itr ) );
      } else {
        Info("fillObjectCutflow()", "  %s pt %6.2f eta %5.2f phi %5.2f overlaps %i ", type.c_str(), (obj_itr)->pt()/1000., (obj_itr)->eta(), (obj_itr)->phi(), overlapAcc( *obj_itr) );
      }
      // Check for overlap object link
      if ( objLinkAcc.isAvailable( *obj_itr ) && objLinkAcc( *obj_itr ).isValid() ) {
        const xAOD::IParticle* overlapObj = *objLinkAcc( *obj_itr );
        std::stringstream ss_or; ss_or << overlapObj->type();
        Info("fillObjectCutflow()", "	Overlap: type %s pt %6.2f", (ss_or.str()).c_str(), overlapObj->pt()/1e3);
      }
    }

  }

  return EL::StatusCode::SUCCESS;

}


EL::StatusCode OverlapRemover :: executeOR(  const xAOD::ElectronContainer* inElectrons, const xAOD::MuonContainer* inMuons, const xAOD::JetContainer* inJets,
					     const xAOD::PhotonContainer* inPhotons,   const xAOD::TauJetContainer* inTaus,
					     SystType syst_type, std::vector<std::string>* sysVec)
{

  // instantiate output container(s)
  //
  ConstDataVector<xAOD::ElectronContainer> *selectedElectrons   (nullptr);
  ConstDataVector<xAOD::MuonContainer>     *selectedMuons	(nullptr);
  ConstDataVector<xAOD::JetContainer>      *selectedJets	(nullptr);
  ConstDataVector<xAOD::PhotonContainer>   *selectedPhotons	(nullptr);
  ConstDataVector<xAOD::TauJetContainer>   *selectedTaus	(nullptr);

  // make a switch for systematics types
  //
  switch ( static_cast<int>(syst_type) )
  {

    case NOMINAL:  // this is the nominal case
    {
      if ( m_debug ) { Info("execute()",  "Doing nominal case"); }
      bool nomContainerNotFound(false);

      if( m_useElectrons ) {
	if ( m_store->contains<ConstDataVector<xAOD::ElectronContainer> >(m_inContainerName_Electrons) ) {
	  RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inElectrons, m_inContainerName_Electrons, m_event, m_store, m_verbose) ,"");
	} else {
	  nomContainerNotFound = true;
	  if ( m_numEvent == 1 ) { Warning("executeOR()", "Could not find nominal container %s in xAOD::TStore. Overlap Removal will not be done for the 'all-nominal' case...", m_inContainerName_Electrons.c_str());  }
	}
      } else{
	// Create an empty container
	inElectrons = m_dummyElectronContainer;
      }

      if( m_useMuons ) {
	if ( m_store->contains<ConstDataVector<xAOD::MuonContainer> >(m_inContainerName_Muons) ) {
	  RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inMuons, m_inContainerName_Muons, m_event, m_store, m_verbose) ,"");
	} else {
	  nomContainerNotFound = true;
	  if ( m_numEvent == 1 ) { Warning("executeOR()", "Could not find nominal container %s in xAOD::TStore. Overlap Removal will not be done for the 'all-nominal' case...", m_inContainerName_Muons.c_str()); }
	}
      } else{
	inMuons = m_dummyMuonContainer;
      }

      if ( m_store->contains<ConstDataVector<xAOD::JetContainer> >(m_inContainerName_Jets) ) {
	RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inJets, m_inContainerName_Jets, m_event, m_store, m_verbose) ,"");
      } else {
        nomContainerNotFound = true;
        if ( m_numEvent == 1 ) { Warning("executeOR()", "Could not find nominal container %s in xAOD::TStore. Overlap Removal will not be done for the 'all-nominal' case...", m_inContainerName_Jets.c_str()); }
      }

      if ( m_usePhotons ) {
         if ( m_store->contains<ConstDataVector<xAOD::PhotonContainer> >(m_inContainerName_Photons) ) {
	   RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inPhotons, m_inContainerName_Photons, m_event, m_store, m_verbose) ,"");
         } else {
           nomContainerNotFound = true;
           if ( m_numEvent == 1 ) { Warning("executeOR()", "Could not find nominal container %s in xAOD::TStore. Overlap Removal will not be done for the 'all-nominal' case...", m_inContainerName_Photons.c_str()); }
         }
      }

      if ( m_useTaus ) {
         if ( m_store->contains<ConstDataVector<xAOD::TauJetContainer> >(m_inContainerName_Taus) ) {
	   RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inTaus, m_inContainerName_Taus, m_event, m_store, m_verbose) ,"");
         } else {
           nomContainerNotFound = true;
	   if ( m_numEvent == 1 ) { Warning("executeOR()", "Could not find nominal container %s in xAOD::TStore. Overlap Removal will not be done for the 'all-nominal' case...", m_inContainerName_Taus.c_str()); }
	 }
      }

      if ( nomContainerNotFound ) {return EL::StatusCode::SUCCESS;}

      if ( m_debug ) {
	if ( m_useElectrons ) { Info("execute()",  "inElectrons : %lu", inElectrons->size()); }
	if ( m_useMuons )     { Info("execute()",  "inMuons : %lu", inMuons->size()); }
	Info("execute()",  "inJets : %lu", inJets->size() );
	if ( m_usePhotons )   { Info("execute()",  "inPhotons : %lu", inPhotons->size());  }
	if ( m_useTaus    )   { Info("execute()",  "inTaus : %lu",    inTaus->size());  }
      }

      // do the actual OR
      //
      if ( m_debug ) { Info("execute()",  "Calling removeOverlaps()"); }
      RETURN_CHECK( "OverlapRemover::execute()", m_overlapRemovalTool->removeOverlaps(inElectrons, inMuons, inJets, inTaus, inPhotons), "");

      std::string ORdecor("overlaps");
      if(m_useCutFlow){
        // fill cutflow histograms
        //
        if ( m_debug ) { Info("execute()",  "Filling Cut Flow Histograms"); }
        if ( m_useElectrons ) fillObjectCutflow(inElectrons);
        if ( m_useMuons )     fillObjectCutflow(inMuons);
        fillObjectCutflow(inJets);
        if ( m_usePhotons )   fillObjectCutflow(inPhotons);
        if ( m_useTaus )      fillObjectCutflow(inTaus);
      }

      // make a copy of input container(s) with selected objects
      //
      if ( m_createSelectedContainers ) {
	if ( m_debug ) { Info("execute()",  "Creating selected Containers"); }
        if( m_useElectrons ) selectedElectrons  = new ConstDataVector<xAOD::ElectronContainer>(SG::VIEW_ELEMENTS);
        if( m_useMuons )     selectedMuons      = new ConstDataVector<xAOD::MuonContainer>(SG::VIEW_ELEMENTS);
        selectedJets	    = new ConstDataVector<xAOD::JetContainer>(SG::VIEW_ELEMENTS);
        if ( m_usePhotons )  selectedPhotons	= new ConstDataVector<xAOD::PhotonContainer>(SG::VIEW_ELEMENTS);
        if ( m_useTaus )     selectedTaus	= new ConstDataVector<xAOD::TauJetContainer>(SG::VIEW_ELEMENTS);
      }

      // resize containers basd on OR decision:
      //
      // if an object has been flagged as 'overlaps', it won't be stored in the 'selected' container
      //
      if ( m_debug ) { Info("execute()",  "Resizing"); }
      if ( m_useElectrons ) { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inElectrons, selectedElectrons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
      if ( m_useMuons )     { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inMuons, selectedMuons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
      RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inJets, selectedJets, ORdecor.c_str(), ToolName::OVERLAPREMOVER), "");
      if ( m_usePhotons )   { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inPhotons, selectedPhotons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
      if ( m_useTaus )      { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inTaus, selectedTaus, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }

      if ( m_debug ) {
	if ( m_useElectrons) { Info("execute()",  "selectedElectrons : %lu", selectedElectrons->size()); }
	if ( m_useMuons )    { Info("execute()",  "selectedMuons : %lu",     selectedMuons->size()); }
	Info("execute()",  "selectedJets : %lu", selectedJets->size());
	if ( m_usePhotons )  { Info("execute()",  "selectedPhotons : %lu", selectedPhotons->size()); }
        if ( m_useTaus )     { Info("execute()",  "selectedTaus : %lu", selectedTaus->size() ); }
      }

      // add ConstDataVector to TStore
      //
      if ( m_createSelectedContainers ) {
	if ( m_debug ) { Info("execute()",  "Recording"); }
        if ( m_useElectrons ){ RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedElectrons,   m_outContainerName_Electrons ), "Failed to store const data container"); }
        if ( m_useMuons )    { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedMuons,	 m_outContainerName_Muons ), "Failed to store const data container"); }
        RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedJets,	 m_outContainerName_Jets ), "Failed to store const data container");
        if ( m_usePhotons )  { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedPhotons, m_outContainerName_Photons ), "Failed to store const data container"); }
        if ( m_useTaus )     { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedTaus, m_outContainerName_Taus ), "Failed to store const data container"); }
      }

      break;
    }
 case ELSYST : // electron syst
    {
      if ( m_debug ) { Info("execute()",  "Doing electron systematics"); }
      // just to check everything is fine
      if ( m_debug ) {
           Info("execute()","will consider the following ELECTRON systematics:" );
           for ( auto it : *sysVec ) {	Info("execute()" ,"\t %s ", it.c_str()); }
      }

      // these input containers won't change in the electron syst loop ...
      //
      if ( m_useMuons ) {
        if ( m_store->contains<ConstDataVector<xAOD::MuonContainer> >(m_inContainerName_Muons) ) {
          RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inMuons, m_inContainerName_Muons, m_event, m_store, m_verbose) ,"");
        } else {
          Error("executeOR()", "Attempt at running w/ electron systematics. Could not find nominal container %s in xAOD::TStore. Aborting", m_inContainerName_Muons.c_str());
      	  return EL::StatusCode::FAILURE;
        }
      }
      if ( m_store->contains<ConstDataVector<xAOD::JetContainer> >(m_inContainerName_Jets) ) {
        RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inJets, m_inContainerName_Jets, m_event, m_store, m_verbose) ,"");
      } else {
        Error("executeOR()", "Attempt at running w/ electron systematics. Could not find nominal container %s in xAOD::TStore. Aborting", m_inContainerName_Jets.c_str());
	return EL::StatusCode::FAILURE;
      }
      if ( m_usePhotons ) {
         if ( m_store->contains<ConstDataVector<xAOD::PhotonContainer> >(m_inContainerName_Photons) ) {
	   RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inPhotons, m_inContainerName_Photons, m_event, m_store, m_verbose) ,"");
         } else {
           Error("executeOR()", "Attempt at running w/ electron systematics. Could not find nominal container %s in xAOD::TStore. Aborting", m_inContainerName_Photons.c_str());
	   return EL::StatusCode::FAILURE;
         }
      }
      if ( m_useTaus ) {
         if ( m_store->contains<ConstDataVector<xAOD::TauJetContainer> >(m_inContainerName_Taus) ) {
	   RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inTaus, m_inContainerName_Taus, m_event, m_store, m_verbose) ,"");
         } else {
           Error("executeOR()", "Attempt at running w/ electron systematics. Could not find nominal container %s in xAOD::TStore. Aborting", m_inContainerName_Taus.c_str());
	   return EL::StatusCode::FAILURE;
	 }
      }

      for ( auto systName : *sysVec) {

	if ( systName.empty() ) continue;

        // ... instead, the electron input container will be different for each syst
	//
	std::string el_syst_cont_name = m_inContainerName_Electrons + systName;
        if ( m_store->contains<ConstDataVector<xAOD::ElectronContainer> >(el_syst_cont_name) ) {
          RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inElectrons, el_syst_cont_name, m_event, m_store, m_verbose) ,"");
        } else {
           Error("executeOR()", "Attempt at running w/ electron systematics. Could not find syst container %s in xAOD::TStore. Aborting", el_syst_cont_name.c_str());
	   return EL::StatusCode::FAILURE;
	}

	if ( m_debug ) { Info("execute()",  "inElectrons : %lu, inMuons : %lu, inJets : %lu", inElectrons->size(), inMuons->size(),  inJets->size() );  }

        // do the actual OR
	//
        RETURN_CHECK( "OverlapRemover::execute()", m_overlapRemovalTool->removeOverlaps(inElectrons, inMuons, inJets, inTaus, inPhotons), "");

        std::string ORdecor("overlaps");
        if(m_useCutFlow){
          // fill cutflow histograms
          //
          fillObjectCutflow(inElectrons);
          if ( m_useMuons )   fillObjectCutflow(inMuons);
          fillObjectCutflow(inJets);
          if ( m_usePhotons ) fillObjectCutflow(inPhotons);
          if ( m_useTaus )    fillObjectCutflow(inTaus);
	}

        // make a copy of input container(s) with selected objects
	//
        if ( m_createSelectedContainers ) {
          selectedElectrons   = new ConstDataVector<xAOD::ElectronContainer>(SG::VIEW_ELEMENTS);
          if ( m_useMuons )    selectedMuons	= new ConstDataVector<xAOD::MuonContainer>(SG::VIEW_ELEMENTS);
          selectedJets	      = new ConstDataVector<xAOD::JetContainer>(SG::VIEW_ELEMENTS);
          if ( m_usePhotons )  selectedPhotons	= new ConstDataVector<xAOD::PhotonContainer>(SG::VIEW_ELEMENTS);
          if ( m_useTaus )     selectedTaus	= new ConstDataVector<xAOD::TauJetContainer>(SG::VIEW_ELEMENTS);
        }

        // resize containers basd on OR decision
	//
        RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inElectrons, selectedElectrons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), "");
        if ( m_useMuons )  {  RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inMuons, selectedMuons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
        RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inJets, selectedJets, ORdecor.c_str(), ToolName::OVERLAPREMOVER), "");
        if ( m_usePhotons ){ RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inPhotons, selectedPhotons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
        if ( m_useTaus )   {	RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inTaus, selectedTaus, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }

        // add ConstDataVector to TStore
	//
        if ( m_createSelectedContainers ) {
          // a different syst varied container will be stored for each syst variation
          RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedElectrons, m_outContainerName_Electrons + systName ), "Failed to store const data container");
          if ( m_useMuons )  { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedMuons,     m_outContainerName_Muons + systName ), "Failed to store const data container"); }
          RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedJets,      m_outContainerName_Jets + systName ), "Failed to store const data container");
          if ( m_usePhotons ){ RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedPhotons, m_outContainerName_Photons + systName ), "Failed to store const data container"); }
          if ( m_useTaus )   { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedTaus, m_outContainerName_Taus + systName ), "Failed to store const data container"); }
        }
      } // close loop on systematic sets available from upstream algo (Electrons)

      break;
    }
 case MUSYST: // muon syst
    {
      if ( m_debug ) { Info("execute()",  "Doing  muon systematics"); }
      // just to check everything is fine
      if ( m_debug ) {
         Info("execute()","will consider the following MUON systematics:" );
         for ( auto it : *sysVec ){	Info("execute()" ,"\t %s ", it.c_str()); }
      }

      // these input containers won't change in the muon syst loop ...
      //
      if ( m_useElectrons ) {
        if ( m_store->contains<ConstDataVector<xAOD::ElectronContainer> >(m_inContainerName_Electrons) ) {
          RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inElectrons, m_inContainerName_Electrons, m_event, m_store, m_verbose) ,"");
        } else {
          Error("executeOR()", "Attempt at running w/ muon systematics. Could not find nominal container %s in xAOD::TStore. Aborting", m_inContainerName_Electrons.c_str());
      	  return EL::StatusCode::FAILURE;
        }
      }
      if ( m_store->contains<ConstDataVector<xAOD::JetContainer> >(m_inContainerName_Jets) ) {
        RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inJets, m_inContainerName_Jets, m_event, m_store, m_verbose) ,"");
      } else {
        Error("executeOR()", "Attempt at running w/ muon systematics. Could not find nominal container %s in xAOD::TStore. Aborting", m_inContainerName_Jets.c_str());
	return EL::StatusCode::FAILURE;
      }
      if ( m_usePhotons ) {
         if ( m_store->contains<ConstDataVector<xAOD::PhotonContainer> >(m_inContainerName_Photons) ) {
	   RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inPhotons, m_inContainerName_Photons, m_event, m_store, m_verbose) ,"");
         } else {
           Error("executeOR()", "Attempt at running w/ muon systematics. Could not find nominal container %s in xAOD::TStore. Aborting", m_inContainerName_Photons.c_str());
	   return EL::StatusCode::FAILURE;
         }
      }
      if ( m_useTaus ) {
         if ( m_store->contains<ConstDataVector<xAOD::TauJetContainer> >(m_inContainerName_Taus) ) {
	   RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inTaus, m_inContainerName_Taus, m_event, m_store, m_verbose) ,"");
         } else {
           Error("executeOR()", "Attempt at running w/ muon systematics. Could not find nominal container %s in xAOD::TStore. Aborting", m_inContainerName_Taus.c_str());
	   return EL::StatusCode::FAILURE;
	 }
      }

      for ( auto systName : *sysVec) {

	if ( systName.empty() ) continue;

	// ... instead, the muon input container will be different for each syst
	//
	std::string mu_syst_cont_name = m_inContainerName_Muons + systName;
        if ( m_store->contains<ConstDataVector<xAOD::MuonContainer> >(mu_syst_cont_name) ) {
          RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inMuons, mu_syst_cont_name, m_event, m_store, m_verbose) ,"");
        } else {
           Error("executeOR()", "Attempt at running w/ muon systematics. Could not find syst container %s in xAOD::TStore. Aborting",mu_syst_cont_name.c_str());
	   return EL::StatusCode::FAILURE;
	}

         if ( m_debug ) { Info("execute()",  "inElectrons : %lu, inMuons : %lu, inJets : %lu ", inElectrons->size(), inMuons->size(),  inJets->size() );  }

        // do the actual OR
	//
        RETURN_CHECK( "OverlapRemover::execute()", m_overlapRemovalTool->removeOverlaps(inElectrons, inMuons, inJets, inTaus, inPhotons), "");

        std::string ORdecor("overlaps");
        if(m_useCutFlow){
          // fill cutflow histograms
          //
          if ( m_useElectrons ) fillObjectCutflow(inElectrons);
          fillObjectCutflow(inMuons);
          fillObjectCutflow(inJets);
          if( m_usePhotons )    fillObjectCutflow(inPhotons);
          if( m_useTaus )       fillObjectCutflow(inTaus);
        }
        // make a copy of input container(s) with selected objects
	//
        if ( m_createSelectedContainers ) {
          if ( m_useElectrons ) selectedElectrons   = new ConstDataVector<xAOD::ElectronContainer>(SG::VIEW_ELEMENTS);
          selectedMuons	      = new ConstDataVector<xAOD::MuonContainer>(SG::VIEW_ELEMENTS);
          selectedJets	      = new ConstDataVector<xAOD::JetContainer>(SG::VIEW_ELEMENTS);
          if ( m_usePhotons )   selectedPhotons	= new ConstDataVector<xAOD::PhotonContainer>(SG::VIEW_ELEMENTS);
          if ( m_useTaus )      selectedTaus	= new ConstDataVector<xAOD::TauJetContainer>(SG::VIEW_ELEMENTS);
        }

        // resize containers based on OR decision
	//
        if ( m_useElectrons ) { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inElectrons, selectedElectrons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
        RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inMuons, selectedMuons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), "");
        RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inJets, selectedJets, ORdecor.c_str(), ToolName::OVERLAPREMOVER), "");
        if ( m_usePhotons )   { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inPhotons, selectedPhotons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
        if ( m_useTaus )      {	RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inTaus, selectedTaus, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }

        // add ConstDataVector to TStore
	//
        if ( m_createSelectedContainers ) {
          // a different syst varied container will be stored for each syst variation
	  //
          if ( m_useElectrons ) { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedElectrons, m_outContainerName_Electrons + systName ), "Failed to store const data container"); }
          RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedMuons,     m_outContainerName_Muons + systName ), "Failed to store const data container");
          RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedJets,      m_outContainerName_Jets + systName ), "Failed to store const data container");
          if ( m_usePhotons )   { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedPhotons, m_outContainerName_Photons + systName ), "Failed to store const data container"); }
          if ( m_useTaus )      { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedTaus, m_outContainerName_Taus + systName ), "Failed to store const data container"); }
        }

      } // close loop on systematic sets available from upstream algo (Muons)

      break;
    }
 case JETSYST: // jet systematics
    {
      if ( m_debug ) { Info("execute()",  "Doing  jet systematics"); }
      // just to check everything is fine
      if ( m_debug ) {
        Info("execute()","will consider the following JET systematics:" );
        for ( auto it : *sysVec ) { Info("execute()" ,"\t %s ", it.c_str());  }
      }

      // these input containers won't change in the jet syst loop ...
      //
      if ( m_useElectrons ) {
        if ( m_store->contains<ConstDataVector<xAOD::ElectronContainer> >(m_inContainerName_Electrons) ) {
          RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inElectrons, m_inContainerName_Electrons, m_event, m_store, m_verbose) ,"");
        } else {
          Error("executeOR()", "Attempt at running w/ jet systematics. Could not find nominal container %s in xAOD::TStore. Aborting", m_inContainerName_Electrons.c_str());
      	  return EL::StatusCode::FAILURE;
        }
      }
      if ( m_useMuons ) {
        if ( m_store->contains<ConstDataVector<xAOD::MuonContainer> >(m_inContainerName_Muons) ) {
          RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inMuons, m_inContainerName_Muons, m_event, m_store, m_verbose) ,"");
        } else {
          Error("executeOR()", "Attempt at running w/ jet systematics. Could not find nominal container %s in xAOD::TStore. Aborting", m_inContainerName_Muons.c_str());
      	  return EL::StatusCode::FAILURE;
        }
      }
      if ( m_usePhotons ) {
         if ( m_store->contains<ConstDataVector<xAOD::PhotonContainer> >(m_inContainerName_Photons) ) {
	   RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inPhotons, m_inContainerName_Photons, m_event, m_store, m_verbose) ,"");
         } else {
           Error("executeOR()", "Attempt at running w/ jet systematics. Could not find nominal container %s in xAOD::TStore. Aborting", m_inContainerName_Photons.c_str());
	   return EL::StatusCode::FAILURE;
         }
      }
      if ( m_useTaus ) {
         if ( m_store->contains<ConstDataVector<xAOD::TauJetContainer> >(m_inContainerName_Taus) ) {
	   RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inTaus, m_inContainerName_Taus, m_event, m_store, m_verbose) ,"");
         } else {
           Error("executeOR()", "Attempt at running w/ jet systematics. Could not find nominal container %s in xAOD::TStore. Aborting", m_inContainerName_Taus.c_str());
	   return EL::StatusCode::FAILURE;
	 }
      }

      for( auto systName : *sysVec ) {

	 if ( systName.empty() ) continue;

	 // ... instead, the jet input container will be different for each syst
	 //
	 std::string jet_syst_cont_name = m_inContainerName_Jets + systName;
         if ( m_store->contains<ConstDataVector<xAOD::JetContainer> >(jet_syst_cont_name) ) {
           RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inJets, jet_syst_cont_name, m_event, m_store, m_verbose) ,"");
         } else {
            Error("executeOR()", "Attempt at running w/ jet systematics. Could not find syst container %s in xAOD::TStore. Aborting",jet_syst_cont_name.c_str());
	    return EL::StatusCode::FAILURE;
	 }

         if ( m_debug ) { Info("execute()",  "inElectrons : %lu, inMuons : %lu, inJets : %lu ", inElectrons->size(), inMuons->size(),  inJets->size() );  }

         // do the actual OR
	 //
         RETURN_CHECK( "OverlapRemover::execute()", m_overlapRemovalTool->removeOverlaps(inElectrons, inMuons, inJets, inTaus, inPhotons), "");

         std::string ORdecor("overlaps");
         if(m_useCutFlow){
           // fill cutflow histograms
           //
           if ( m_useElectrons ) fillObjectCutflow(inElectrons);
           if ( m_useMuons )     fillObjectCutflow(inMuons);
           fillObjectCutflow(inJets);
           if( m_usePhotons )    fillObjectCutflow(inPhotons);
           if( m_useTaus )       fillObjectCutflow(inTaus);
        }

	 // make a copy of input container(s) with selected objects
	 //
	 if ( m_createSelectedContainers ) {
	   if ( m_useElectrons ) selectedElectrons   = new ConstDataVector<xAOD::ElectronContainer>(SG::VIEW_ELEMENTS);
	   if ( m_useMuons )     selectedMuons	    = new ConstDataVector<xAOD::MuonContainer>(SG::VIEW_ELEMENTS);
	   selectedJets	      = new ConstDataVector<xAOD::JetContainer>(SG::VIEW_ELEMENTS);
	   if ( m_usePhotons )   selectedPhotons	= new ConstDataVector<xAOD::PhotonContainer>(SG::VIEW_ELEMENTS);
	   if ( m_useTaus )      selectedTaus	= new ConstDataVector<xAOD::TauJetContainer>(SG::VIEW_ELEMENTS);
	 }

	 // resize containers basd on OR decision
	 //
	 if ( m_useElectrons ) { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inElectrons, selectedElectrons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
	 if ( m_useMuons )     { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inMuons, selectedMuons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
	 RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inJets, selectedJets, ORdecor.c_str(), ToolName::OVERLAPREMOVER), "");
	 if ( m_usePhotons )   { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inPhotons, selectedPhotons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
	 if ( m_useTaus )      { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inTaus, selectedTaus, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }

	 // add ConstDataVector to TStore
	 //
	 if ( m_createSelectedContainers ) {
	   // a different syst varied container will be stored for each syst variation
	   //
	   if ( m_useElectrons ) { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedElectrons, m_outContainerName_Electrons + systName ), "Failed to store const data container"); }
	   if ( m_useMuons )	 { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedMuons,     m_outContainerName_Muons + systName ), "Failed to store const data container"); }
	   RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedJets,      m_outContainerName_Jets + systName ), "Failed to store const data container");
	   if ( m_usePhotons )   { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedPhotons, m_outContainerName_Photons + systName ), "Failed to store const data container"); }
	   if ( m_useTaus )      { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedTaus, m_outContainerName_Taus + systName ), "Failed to store const data container"); }
	 }
      } // close loop on systematic sets available from upstream algo (Jets)

      break;
    }
 case PHSYST : // photon systematics
    {
      if ( m_debug ) { Info("execute()",  "Doing  photon systematics"); }
      // prepare a vector of the names of CDV containers
      // must be a pointer to be recorded in TStore
      // for now just copy the one you just retrieved in it!
      //
      std::vector< std::string >* vecOutContainerNames_photon = new std::vector< std::string >(*sysVec);
      // just to check everything is fine
      if ( m_debug ) {
        Info("execute()","output vector already contains the following PHOTON systematics:" );
        for ( auto it : *vecOutContainerNames_photon) { Info("execute()" ,"\t %s ", it.c_str());  }
      }

      // these input containers won't change in the photon syst loop ...
      //
      if ( m_useElectrons ) {
	RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inElectrons, m_inContainerName_Electrons, m_event, m_store, m_verbose) ,"");
      }else{
	// Create an empty container
	inElectrons = m_dummyElectronContainer;
      }

      if ( m_useMuons ) {
	RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inMuons, m_inContainerName_Muons, m_event, m_store, m_verbose) ,"");
      } else {
	inMuons = m_dummyMuonContainer;
      }

      RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inJets, m_inContainerName_Jets, m_event, m_store, m_verbose) ,"");
      if ( m_useTaus )     RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inTaus, m_inContainerName_Taus, m_event, m_store, m_verbose) ,"");

      for( auto systName : *sysVec ) {

	 if ( systName.empty() ) continue;

	 // ... instead, the photon input container will be different for each syst
	 //
	 std::string photon_syst_cont_name = m_inContainerName_Photons + systName;
         if ( m_store->contains<ConstDataVector<xAOD::PhotonContainer> >(photon_syst_cont_name) ) {
           RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inPhotons, photon_syst_cont_name, m_event, m_store, m_verbose) ,"");
         } else {
            Error("executeOR()", "Attempt at running w/ photon systematics. Could not find syst container %s in xAOD::TStore. Aborting",photon_syst_cont_name.c_str());
	    return EL::StatusCode::FAILURE;
	 }

         if ( m_debug ) {
	   Info("execute()",  "inElectrons : %lu, inMuons : %lu, inJets : %lu", inElectrons->size(), inMuons->size(),  inJets->size() );
	 }

         // do the actual OR
	 //
         RETURN_CHECK( "OverlapRemover::execute()", m_overlapRemovalTool->removeOverlaps(inElectrons, inMuons, inJets, inTaus, inPhotons), "");


         std::string ORdecor = std::string("overlaps");
         if(m_useCutFlow){
           // fill cutflow histograms
           //
           if( m_useElectrons ) fillObjectCutflow(inElectrons);
           if( m_useMuons     ) fillObjectCutflow(inMuons);
           fillObjectCutflow(inJets);
           fillObjectCutflow(inPhotons);
           if( m_useTaus )      fillObjectCutflow(inTaus);
        }

        // make a copy of input container(s) with selected objects
	//
        if ( m_createSelectedContainers ) {
          if( m_useElectrons ) selectedElectrons   = new ConstDataVector<xAOD::ElectronContainer>(SG::VIEW_ELEMENTS);
          if( m_useMuons     ) selectedMuons       = new ConstDataVector<xAOD::MuonContainer>(SG::VIEW_ELEMENTS);
          selectedJets	      = new ConstDataVector<xAOD::JetContainer>(SG::VIEW_ELEMENTS);
	  selectedPhotons     = new ConstDataVector<xAOD::PhotonContainer>(SG::VIEW_ELEMENTS);
          if ( m_useTaus )     selectedTaus	   = new ConstDataVector<xAOD::TauJetContainer>(SG::VIEW_ELEMENTS);
        }

        // resize containers based on OR decision
	//
        if( m_useElectrons ) { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inElectrons, selectedElectrons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
        if( m_useMuons )     { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inMuons, selectedMuons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
        RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inJets, selectedJets, ORdecor.c_str(), ToolName::OVERLAPREMOVER), "");
	RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inPhotons, selectedPhotons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), "");
        if ( m_useTaus )     { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inTaus, selectedTaus, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }

        // add ConstDataVector to TStore
	//
        if ( m_createSelectedContainers ) {
          // a different syst varied container will be stored for each syst variation
	  //
          if( m_useElectrons ){ RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedElectrons, m_outContainerName_Electrons + systName ), "Failed to store const data container"); }
          if( m_useMuons )    { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedMuons,     m_outContainerName_Muons + systName ), "Failed to store const data container"); }
          RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedJets,      m_outContainerName_Jets + systName ), "Failed to store const data container");
          RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedPhotons,   m_outContainerName_Photons + systName ), "Failed to store const data container");
          if ( m_useTaus )    { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedTaus, m_outContainerName_Taus + systName ), "Failed to store const data container"); }
        }
      } // close loop on systematic sets available from upstream algo (Photons)

      // add vector<string container_names_syst> to TStore
      //
      RETURN_CHECK( "OverlapRemover::execute()", m_store->record( vecOutContainerNames_photon, m_outputAlgoPhotons ), "Failed to record vector of output container names.");

      break;
    }
  case TAUSYST : // tau systematics
    {
      if ( m_debug ) { Info("execute()",  "Doing tau systematics"); }

      // just to check everything is fine
      if ( m_debug ) {
        Info("execute()","output vector already contains the following TAU systematics:" );
        for ( auto it : *sysVec ) { Info("execute()" ,"\t %s ", it.c_str());  }
      }

      // these input containers won't change in the tau syst loop ...
      //
      if ( m_useElectrons ) {
	RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inElectrons, m_inContainerName_Electrons, m_event, m_store, m_verbose) ,"");
      }else{
	// Create an empty container
	inElectrons = m_dummyElectronContainer;
      }

      if ( m_useMuons ) {
	RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inMuons, m_inContainerName_Muons, m_event, m_store, m_verbose) ,"");
      } else {
	inMuons = m_dummyMuonContainer;
      }

      RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inJets, m_inContainerName_Jets, m_event, m_store, m_verbose) ,"");

      if ( m_usePhotons ) {
	RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inPhotons, m_inContainerName_Photons, m_event, m_store, m_verbose) ,"");
      }

      for( auto systName : *sysVec ) {

	 if ( systName.empty() ) continue;

	 // ... instead, the tau input container will be different for each syst
	 //
	 std::string tau_syst_cont_name = m_inContainerName_Taus + systName;
         if ( m_store->contains<ConstDataVector<xAOD::TauJetContainer> >(tau_syst_cont_name) ) {
           RETURN_CHECK("OverlapRemover::execute()", HelperFunctions::retrieve(inTaus, tau_syst_cont_name, m_event, m_store, m_verbose) ,"");
         } else {
            Error("executeOR()", "Attempt at running w/ tau systematics. Could not find syst container %s in xAOD::TStore. Aborting", tau_syst_cont_name.c_str());
	    return EL::StatusCode::FAILURE;
	 }

         if ( m_debug ) {
	   Info("execute()",  "inElectrons : %lu, inMuons : %lu, inJets : %lu", inElectrons->size(), inMuons->size(),  inJets->size() );
	 }

         // do the actual OR
	 //
         RETURN_CHECK( "OverlapRemover::execute()", m_overlapRemovalTool->removeOverlaps(inElectrons, inMuons, inJets, inTaus, inPhotons), "");

         std::string ORdecor = std::string("overlaps");
         if(m_useCutFlow){
           // fill cutflow histograms
           //
           if( m_useElectrons ) fillObjectCutflow(inElectrons);
           if( m_useMuons     ) fillObjectCutflow(inMuons);
           fillObjectCutflow(inJets);
           if( m_usePhotons ) fillObjectCutflow(inPhotons);
           fillObjectCutflow(inTaus);
	 }

	 // make a copy of input container(s) with selected objects
	 //
	 if ( m_createSelectedContainers ) {
	   if( m_useElectrons ) selectedElectrons   = new ConstDataVector<xAOD::ElectronContainer>(SG::VIEW_ELEMENTS);
	   if( m_useMuons )     selectedMuons       = new ConstDataVector<xAOD::MuonContainer>(SG::VIEW_ELEMENTS);
	   selectedJets	      = new ConstDataVector<xAOD::JetContainer>(SG::VIEW_ELEMENTS);
	   if ( m_usePhotons )  selectedPhotons     = new ConstDataVector<xAOD::PhotonContainer>(SG::VIEW_ELEMENTS);
	   selectedTaus	      = new ConstDataVector<xAOD::TauJetContainer>(SG::VIEW_ELEMENTS);
        }

        // resize containers based on OR decision
	//
        if( m_useElectrons ) { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inElectrons, selectedElectrons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
        if( m_useMuons )     { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inMuons, selectedMuons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
        RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inJets, selectedJets, ORdecor.c_str(), ToolName::OVERLAPREMOVER), "");
	if ( m_usePhotons )  { RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inPhotons, selectedPhotons, ORdecor.c_str(), ToolName::OVERLAPREMOVER), ""); }
        RETURN_CHECK( "OverlapRemover::execute()", HelperFunctions::makeSubsetCont(inTaus, selectedTaus, ORdecor.c_str(), ToolName::OVERLAPREMOVER), "");

        // add ConstDataVector to TStore
	//
        if ( m_createSelectedContainers ) {
          // a different syst varied container will be stored for each syst variation
	  //
          if( m_useElectrons ) { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedElectrons, m_outContainerName_Electrons + systName ), "Failed to store const data container"); }
          if( m_useMuons )     { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedMuons,     m_outContainerName_Muons + systName ), "Failed to store const data container"); }
          RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedJets,      m_outContainerName_Jets + systName ), "Failed to store const data container");
          if ( m_usePhotons )  { RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedPhotons,   m_outContainerName_Photons + systName ), "Failed to store const data container"); }
          RETURN_CHECK( "OverlapRemover::execute()", m_store->record( selectedTaus, m_outContainerName_Taus + systName ), "Failed to store const data container");
        }
      } // close loop on systematic sets available from upstream algo (Taus)

      break;
    }
 default :
    {
      Error("OverlapRemover::execute()","Unknown systematics type. Aborting");
      return EL::StatusCode::FAILURE;
    }
  } // end of switch


  return EL::StatusCode::SUCCESS;

}

EL::StatusCode OverlapRemover :: setCutFlowHist( )
{

 if ( m_useCutFlow ) {

   // retrieve the file in which the cutflow hists are stored
   //
   TFile *file     = wk()->getOutputFile ("cutflow");

   // retrieve the object cutflow
   //
   m_el_cutflowHist_1	 = (TH1D*)file->Get("cutflow_electrons_1");
   m_el_cutflow_OR_cut   = m_el_cutflowHist_1->GetXaxis()->FindBin("OR_cut");
   m_mu_cutflowHist_1	 = (TH1D*)file->Get("cutflow_muons_1");
   m_mu_cutflow_OR_cut   = m_mu_cutflowHist_1->GetXaxis()->FindBin("OR_cut");
   m_jet_cutflowHist_1   = (TH1D*)file->Get("cutflow_jets_1");
   m_jet_cutflow_OR_cut  = m_jet_cutflowHist_1->GetXaxis()->FindBin("OR_cut");
   m_ph_cutflowHist_1	 = (TH1D*)file->Get("cutflow_photons_1");
   m_ph_cutflow_OR_cut   = m_ph_cutflowHist_1->GetXaxis()->FindBin("OR_cut");
   m_tau_cutflowHist_1   = (TH1D*)file->Get("cutflow_taus_1");
   m_tau_cutflow_OR_cut  = m_tau_cutflowHist_1->GetXaxis()->FindBin("OR_cut");
 }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode OverlapRemover :: setCounters( )
{
  m_numEvent      = 0;
  m_numObject     = 0;

  return EL::StatusCode::SUCCESS;
}
