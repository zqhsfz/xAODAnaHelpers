#include "xAODAnaHelpers/TrackHists.h"

#include <math.h>

#include "xAODAnaHelpers/tools/ReturnCheck.h"

TrackHists :: TrackHists (std::string name, std::string detailStr) :
  HistogramManager(name, detailStr)
{
}

TrackHists :: ~TrackHists () {}

StatusCode TrackHists::initialize() {

  // These plots are always made
  m_trk_Pt        = book(m_name, "pt",          "trk p_{T} [GeV]",  100, 0, 10);
  m_trk_Pt_l      = book(m_name, "pt_l",        "trk p_{T} [GeV]",  100, 0, 100);
  m_trk_Eta       = book(m_name, "eta",         "trk #eta",         80, -4, 4);
  m_trk_Phi       = book(m_name, "phi",         "trk Phi",120, -TMath::Pi(), TMath::Pi() );
  m_trk_d0        = book(m_name, "d0",          "d0[mm]",   100,  -2.0, 2.0 );
  m_trk_z0        = book(m_name, "z0",          "z0[mm]",   100,  -5.0, 5.0 );
  m_trk_z0sinT    = book(m_name, "z0sinT",           "z0xsin(#theta)[mm]",             100,  -2.0, 2.0 );

  m_trk_chi2Prob  = book(m_name, "chi2Prob",    "chi2Prob", 100,   -0.01,     1.0);
  m_trk_charge    = book(m_name, "charge" ,     "charge",   3,  -1.5,  1.5   );

  //
  //  IP Details
  //
  m_fillIPDetails = false;
  if(m_detailStr.find("IPDetails") != std::string::npos ){
    m_fillIPDetails = true;

    m_trk_d0Err        = book(m_name, "d0Err",            "d0Err[mm]",        100,  0, 0.4 );
    m_trk_d0_l         = book(m_name, "d0_l" ,            "d0[mm]",           100,  -10.0, 10.0 );
    m_trk_d0Sig        = book(m_name, "d0Sig",            "d0Sig",            240,  -20.0, 40.0 );

    m_trk_z0_l         = book(m_name, "z0_l" ,            "z0[mm]",                         100,  -600.0, 600.0 );
    m_trk_z0sinT_l     = book(m_name, "z0sinT_l",         "z0xsin(#theta)[mm]",             100,  -20.0, 20.0 );
    m_trk_z0Err        = book(m_name, "z0Err",            "z0Err[mm]",                      100,   0, 0.4 );
    m_trk_z0Sig        = book(m_name, "z0Sig",            "z0Sig",                          100,  -25.0, 25.0 );
    m_trk_z0SigsinT    = book(m_name, "z0SigsinT",        "z0 significance x sin(#theta)",  100,  -25.0, 25.0 );

    //m_trk_mc_prob      = book(m_name, "mc_prob",      "mc_prob",     100,  -0.1, 1.1 );
    //m_trk_mc_barcode   = book(m_name, "mc_barcode",   "mc_barcode",  100,  -0.1, 0.5e6 );
    //m_trk_mc_barcode_s = book(m_name, "mc_barcode_s", "mc_barcode",  100,  -0.1, 25e3 );
  }

  //
  //  Fill Hit Counts Details
  //
  m_fillHitCounts = false;
  if(m_detailStr.find("HitCounts") != std::string::npos ){
    m_fillHitCounts = true;
    m_trk_nSi        = book(m_name, "nSi",        "nSi",         30,   -0.5, 29.5 );
    m_trk_nSiAndDead = book(m_name, "nSiAndDead", "nSi(+Dead)",  30,   -0.5, 29.5 );
    m_trk_nSiDead    = book(m_name, "nSiDead",    "nSiDead",     10,   -0.5, 9.5 );
    m_trk_nSCT       = book(m_name, "nSCT",       "nSCTHits",    20,   -0.5, 19.5 );
    m_trk_nPix       = book(m_name, "nPix",       "nPix",        10,   -0.5, 9.5 );
    m_trk_nPixHoles  = book(m_name, "nPixHoles",  "nPixHoles",   10,   -0.5, 9.5 );
    m_trk_nBL        = book(m_name, "nBL",        "nBL",          3,   -0.5,  2.5 );
  }

  //
  //  TP Errors Details
  //
  m_fillTPErrors = false;
  if(m_detailStr.find("TPErrors") != std::string::npos ){
    m_fillTPErrors = true;
    //new TProfile(m_name, "chi2ndofvseta",  "chi2ndofvseta;    eta;       chi2",  20,  -0.1, 2.7, 0, 4, "s" );
    //new TProfile(m_name, "nhitsvseta",  "nhitsvseta;    eta;       nhits",  20,  -0.1, 2.7, 0, 15 , "s" );
    //new TProfile(m_name, "chi2ndofvspt",  "chi2ndofvseta;    pt;       chi2",  50,  -0.1, 20, 0 , 4, "s" );

    //  new TProfile(m_name, "d0Errvseta",      "d0Errvseta;    eta;       d0Err",  20,  -0.1, 2.7, 0, 0.4, "s" );
    //  new TProfile(m_name, "d0ErrvsIpt",      "d0ErrvsIpt;    Ipt;       d0Err",  20,  -6e-4, 6e4, 0, 0.4, "s" );
    //  new TProfile(m_name, "d0Errvsphi",      "d0Errvsphi;    phi;       d0Err",  1000,  -3.2, 3.2, 0, 0.4);
    //

    //  new TH2F(m_name, "z0d0"    ,    "z0d0;     z0[mm];  d0[mm];",  50,  -300.0, 300.0, 50, -4.0, 4.0  );
    //  new TH2F(m_name, "z0sinTd0",    "z0sinTd0;     z0xsin0[mm](signed);  d0[mm](signed);",  50,-2.0, 2.0, 50, -2.0, 2.0  );

    //  new TH2F(m_name, "d0vsPt"    ,    "d0vsPt;     d0[mm] (signed);  Pt[GeV];",  100,  -2,2.0, 50, 0, 10  );
    //  new TH2F(m_name, "d0SigvsPt"    ,    "d0SigvsPt;     d0Sig(signed);  Pt[GeV];",  240, -20, 40.0, 50, 0, 10  );
    m_trk_phiErr       = book(m_name, "phiErr"  ,   "phi Err[rad]",  100,  0, 0.01 );
    m_trk_thetaErr     = book(m_name, "thetaErr",   "theta Err",     100,  0, 0.01 );
    m_trk_qOpErr       = book(m_name, "qOpErr"  ,   "q/p Err",       100,  0, 1.0e-04);
  }

  //
  //  Chi2 Details
  //
  m_fillChi2Details = false;
  if(m_detailStr.find("Chi2Details") != std::string::npos ){
    m_fillChi2Details = true;
    m_trk_chi2Prob_l   = book(m_name, "chi2Prob_l",       "chi2Prob",  100,   -0.1,     1.1);
    m_trk_chi2Prob_s   = book(m_name, "chi2Prob_s",       "chi2Prob",  100,   -0.01,    0.1);
    m_trk_chi2Prob_ss  = book(m_name, "chi2Prob_ss",      "chi2Prob",  100,   -0.001,   0.01);
    m_trk_chi2ndof     = book(m_name, "chi2ndof",         "chi2ndof",  100,    0.0,     8.0 );
    m_trk_chi2ndof_l   = book(m_name, "chi2ndof_l",       "chi2ndof",  100,    0.0,     80.0 );
  }

  //
  // TrkDebugging
  //
  m_fillDebugging = false;
  if(m_detailStr.find("Debugging") != std::string::npos ){
    m_fillDebugging = true;
    m_trk_eta_vl      = book(m_name, "eta_vl",        "eta",       100,  -6,    6     );
    m_trk_z0_vl       = book(m_name, "z0_vl",         "z0[mm]",    100,  -10000.0, 10000.0 );
    m_trk_z0_m_raw    = book(m_name, "z0_m_raw",         "z0[mm]",   100,  -100.0,  100.0 );
    m_trk_z0_m        = book(m_name, "z0_m",         "z0[mm]",   100,  -100.0,  100.0 );
    m_trk_d0_vl       = book(m_name, "d0_vl",         "d0[mm]",    100,  -10000.0, 10000.0 );
    m_trk_pt_ss       = book(m_name, "pt_ss",         "Pt[GeV",    100,  0,     2.0  );
    m_trk_phiManyBins = book(m_name, "phiManyBins" ,  "phi",      1000,  -3.2,  3.2   );

  }

  // if worker is passed to the class add histograms to the output
  return StatusCode::SUCCESS;
}

StatusCode TrackHists::execute( const xAOD::TrackParticleContainer* trks, const xAOD::Vertex *pvx, float eventWeight ) {
  xAOD::TrackParticleContainer::const_iterator trk_itr = trks->begin();
  xAOD::TrackParticleContainer::const_iterator trk_end = trks->end();
  for( ; trk_itr != trk_end; ++trk_itr ) {
    RETURN_CHECK("TrackHists::execute()", this->execute( (*trk_itr), pvx, eventWeight ), "");
  }

  return StatusCode::SUCCESS;
}

StatusCode TrackHists::execute( const xAOD::TrackParticle* trk, const xAOD::Vertex *pvx, float eventWeight ) {

  //basic
  float        trkPt       = trk->pt()/1e3;
  float        chi2        = trk->chiSquared();
  float        ndof        = trk->numberDoF();
  float        chi2Prob    = TMath::Prob(chi2,ndof);
  float        d0          = trk->d0();
  float        z0          = trk->z0() + trk->vz();
  if(pvx)      z0          -= pvx->z();

  float        sinT        = sin(trk->theta());

  m_trk_Pt       -> Fill( trkPt,            eventWeight );
  m_trk_Pt_l     -> Fill( trkPt,            eventWeight );
  m_trk_Eta      -> Fill( trk->eta(),       eventWeight );
  m_trk_Phi      -> Fill( trk->phi(),       eventWeight );
  m_trk_d0       -> Fill( d0,               eventWeight );
  m_trk_z0       -> Fill( z0,               eventWeight );
  m_trk_z0sinT   -> Fill(z0*sinT,           eventWeight );

  m_trk_chi2Prob -> Fill( chi2Prob ,        eventWeight );
  m_trk_charge   -> Fill( trk->charge() ,   eventWeight );

  if(m_fillIPDetails){
    float d0Err = sqrt((trk->definingParametersCovMatrixVec().at(0)));
    float d0Sig = (d0Err > 0) ? d0/d0Err : -1 ;
    m_trk_d0_l         -> Fill(d0    , eventWeight );
    m_trk_d0Err        -> Fill(d0Err , eventWeight );
    m_trk_d0Sig        -> Fill(d0Sig , eventWeight );

    float z0Err = sqrt((trk->definingParametersCovMatrixVec().at(2)));
    float z0Sig = (z0Err > 0) ? z0/z0Err : -1 ;

    m_trk_z0_l         -> Fill(z0         , eventWeight );
    m_trk_z0sinT_l     -> Fill(z0*sinT,     eventWeight );
    m_trk_z0Err        -> Fill(z0Err      , eventWeight );
    m_trk_z0Sig        -> Fill(z0Sig      , eventWeight );
    m_trk_z0SigsinT    -> Fill(z0Sig*sinT , eventWeight );

  }

  if(m_fillHitCounts){

    uint8_t nBL       = -1;
    uint8_t nPix      = -1;
    uint8_t nPixDead  = -1;
    uint8_t nPixHoles = -1;
    uint8_t nSCT      = -1;
    uint8_t nSCTDead  = -1;

    if(!trk->summaryValue(nBL,       xAOD::numberOfBLayerHits))       Error("TrackHists::execute()", "BLayer hits not filled");
    if(!trk->summaryValue(nPix,      xAOD::numberOfPixelHits))        Error("TrackHists::execute()", "Pix hits not filled");
    if(!trk->summaryValue(nPixDead,  xAOD::numberOfPixelDeadSensors)) Error("TrackHists::execute()", "Pix Dead not filled");
    if(!trk->summaryValue(nPixHoles, xAOD::numberOfPixelHoles))       Error("TrackHists::execute()", "Pix holes not filled");
    if(!trk->summaryValue(nSCT,      xAOD::numberOfSCTHits))          Error("TrackHists::execute()", "SCT hits not filled");
    if(!trk->summaryValue(nSCTDead,  xAOD::numberOfSCTDeadSensors))   Error("TrackHists::execute()", "SCT Dead not filled");

    uint8_t nSi     = nPix     + nSCT;
    uint8_t nSiDead = nPixDead + nSCTDead;
    m_trk_nBL        -> Fill( nBL         , eventWeight );
    m_trk_nSi        -> Fill( nSi         , eventWeight );
    m_trk_nSiAndDead -> Fill( nSi+nSiDead , eventWeight );
    m_trk_nSiDead    -> Fill( nSiDead     , eventWeight );
    m_trk_nSCT       -> Fill( nSCT        , eventWeight );
    m_trk_nPix       -> Fill( nPix        , eventWeight );
    m_trk_nPixHoles  -> Fill( nPixHoles   , eventWeight );

  }

  if(m_fillTPErrors){
    //m_trk_phiErr       -> Fill( sqrt((trk->definingParametersCovMatrixVec().at(6))) , eventWeight );
    //m_trk_thetaErr     -> Fill( sqrt((trk->definingParametersCovMatrixVec().at(10))) , eventWeight );
    //m_trk_qOpErr       -> Fill( sqrt((trk->definingParametersCovMatrixVec().at(15))) , eventWeight );
  }

  if(m_fillChi2Details){
    float chi2NDoF     = (ndof > 0) ? chi2/ndof : -1;
    m_trk_chi2Prob_l   -> Fill(chi2Prob   , eventWeight );
    m_trk_chi2Prob_s   -> Fill(chi2Prob   , eventWeight );
    m_trk_chi2Prob_ss  -> Fill(chi2Prob   , eventWeight );
    m_trk_chi2ndof     -> Fill(chi2NDoF   , eventWeight );
    m_trk_chi2ndof_l   -> Fill(chi2NDoF   , eventWeight );
  }

  if(m_fillDebugging){
    m_trk_eta_vl      -> Fill( trk->eta(), eventWeight );
    m_trk_z0_vl       -> Fill( z0,         eventWeight );
    m_trk_z0_m        -> Fill( z0,         eventWeight );
    m_trk_z0_m_raw    -> Fill( trk->z0(),  eventWeight );
    m_trk_d0_vl       -> Fill( d0,         eventWeight );
    m_trk_pt_ss       -> Fill( trkPt,      eventWeight );
    m_trk_phiManyBins -> Fill( trk->phi(), eventWeight );
  }

  return StatusCode::SUCCESS;

}

