#include "xAODAnaHelpers/VtxHists.h"
#include <xAODTracking/TrackParticle.h>

#include <math.h>

#include "xAODAnaHelpers/tools/ReturnCheck.h"

VtxHists :: VtxHists (std::string name, std::string detailStr) :
  HistogramManager(name, detailStr)
{
}

VtxHists :: ~VtxHists () {}

StatusCode VtxHists::initialize() {

  // These plots are always made
  h_type        = book(m_name, "type",          "vtx type",  10,   -0.5,     9.5);
  h_nTrks       = book(m_name, "nTrks",         "nTrks",    100,   -0.5,    99.5);
  h_nTrks_l     = book(m_name, "nTrks_l",       "nTrks",    100,   -0.5,   499.5);


  //
  //  Trk details
  //
  m_fillTrkDetails = false;
  if(m_detailStr.find("TrkDetails") != std::string::npos ){
    m_fillTrkDetails = true;
    h_trk_Pt       = book(m_name, "trkPt",         "trkPt",    100,   -0.5,    9.5);
    h_trk_Pt_l     = book(m_name, "trkPt_l",       "trkPt",    100,   -0.5,   99.5);

    h_pt_miss_x       = book(m_name, "Pt_miss_x",       "Pt_miss_x",      100,   -5.5,    4.5);
    h_pt_miss_x_l     = book(m_name, "Pt_miss_x_l",     "Pt_miss_x_l",    100,   -50.5,   49.5);

    h_pt_miss_y       = book(m_name, "Pt_miss_y",       "Pt_miss_y",      100,   -5.5,    4.5);
    h_pt_miss_y_l     = book(m_name, "Pt_miss_y_l",     "Pt_miss_y_l",    100,   -50.5,   49.5);

    h_pt_miss         = book(m_name, "Pt_miss",         "Pt_miss",      100,   -0.5,    9.5);
    h_pt_miss_l       = book(m_name, "Pt_miss_l",       "Pt_miss_l",    100,   -0.5,   99.5);
  }

  //
  //  Iso Trk details
  //
  m_fillIsoTrkDetails = false;
  if(m_detailStr.find("IsoTrkDetails") != std::string::npos ){
    m_fillIsoTrkDetails = true;

    h_trkIsoAll       = book(m_name, "ptCone20All",         "ptCone20All",    100,   -0.5,    9.5);
    h_trkIso          = book(m_name, "ptCone20",            "ptCone20",       100,   -0.5,    9.5);

    h_dZ0Before        = book(m_name, "dZ0Before",            "dZ0Before",       100,   -0.1,    100);

    h_nIsoTrks       = book(m_name, "nIsoTrks",         "nIsoTrks",    100,   -0.5,    99.5);
    h_nIsoTrks_l     = book(m_name, "nIsoTrks_l",       "nIsoTrks",    100,   -0.5,   499.5);

    h_IsoTrk_Pt       = book(m_name, "isoTrkPt",         "IsoTrkPt",    100,   -0.5,    9.5);
    h_IsoTrk_Pt_l     = book(m_name, "isoTrkPt_l",       "IsoTrkPt",    100,   -0.5,   99.5);

    h_pt_miss_iso_x       = book(m_name, "Pt_miss_iso_x",       "Pt_miss_x",      100,   -5.5,    4.5);
    h_pt_miss_iso_x_l     = book(m_name, "Pt_miss_iso_x_l",     "Pt_miss_x_l",    100,   -50.5,   49.5);

    h_pt_miss_iso_y       = book(m_name, "Pt_miss_iso_y",       "Pt_miss_y",      100,   -5.5,    4.5);
    h_pt_miss_iso_y_l     = book(m_name, "Pt_miss_iso_y_l",     "Pt_miss_y_l",    100,   -50.5,   49.5);

    h_pt_miss_iso         = book(m_name, "Pt_miss_iso",         "Pt_miss",      100,   -0.5,    9.5);
    h_pt_miss_iso_l       = book(m_name, "Pt_miss_iso_l",       "Pt_miss_l",    100,   -0.5,   99.5);


    h_nIsoTrks1GeV       = book(m_name,  "nIsoTracks1GeV",   "nIsoTracks1GeV",  100,  -0.5,  99.5 );
    h_nIsoTrks2GeV       = book(m_name,  "nIsoTracks2GeV",   "nIsoTracks2GeV",  100,  -0.5,  99.5 );
    h_nIsoTrks5GeV       = book(m_name,  "nIsoTracks5GeV",   "nIsoTracks5GeV",  100,  -0.5,  99.5 );
    h_nIsoTrks10GeV      = book(m_name,  "nIsoTracks10GeV",  "nIsoTracks10GeV", 100,  -0.5,  99.5 );
    h_nIsoTrks15GeV      = book(m_name,  "nIsoTracks15GeV",  "nIsoTracks15GeV", 100,  -0.5,  99.5 );
    h_nIsoTrks20GeV      = book(m_name,  "nIsoTracks20GeV",  "nIsoTracks20GeV", 100,  -0.5,  99.5 );
    h_nIsoTrks25GeV      = book(m_name,  "nIsoTracks25GeV",  "nIsoTracks25GeV", 100,  -0.5,  99.5 );
    h_nIsoTrks30GeV      = book(m_name,  "nIsoTracks30GeV",  "nIsoTracks30GeV", 100,  -0.5,  99.5 );

    m_nLeadIsoTrackPts = 10;
    for(uint iLeadTrks = 0; iLeadTrks < m_nLeadIsoTrackPts; ++iLeadTrks){
      std::stringstream ss;
      ss << iLeadTrks;
      h_IsoTrk_max_Pt.push_back(       book(m_name, "IsoTrkPt_"+ss.str(),        "IsoTrkPt("+ss.str()+")",    100,   -0.5,    9.5)  );
      h_IsoTrk_max_Pt_l.push_back(     book(m_name, "IsoTrkPt_"+ss.str()+"_l",   "IsoTrkPt("+ss.str()+")",    100,   -0.5,   99.5)  );
    }

  }


  //
  //  Trk Pt details
  //
  m_fillTrkPtDetails = false;
  if(m_detailStr.find("TrkPtDetails") != std::string::npos ){
    m_fillTrkPtDetails = true;
    h_nTrks1GeV       = book(m_name,  "nTracks1GeV",   "nTracks1GeV",  100,  -0.5,  99.5 );
    h_nTrks2GeV       = book(m_name,  "nTracks2GeV",   "nTracks2GeV",  100,  -0.5,  99.5 );
    h_nTrks5GeV       = book(m_name,  "nTracks5GeV",   "nTracks5GeV",  100,  -0.5,  99.5 );
    h_nTrks10GeV      = book(m_name,  "nTracks10GeV",  "nTracks10GeV", 100,  -0.5,  99.5 );
    h_nTrks15GeV      = book(m_name,  "nTracks15GeV",  "nTracks15GeV", 100,  -0.5,  99.5 );
    h_nTrks20GeV      = book(m_name,  "nTracks20GeV",  "nTracks20GeV", 100,  -0.5,  99.5 );
    h_nTrks25GeV      = book(m_name,  "nTracks25GeV",  "nTracks25GeV", 100,  -0.5,  99.5 );
    h_nTrks30GeV      = book(m_name,  "nTracks30GeV",  "nTracks30GeV", 100,  -0.5,  99.5 );

    m_nLeadTrackPts = 10;
    for(uint iLeadTrks = 0; iLeadTrks < m_nLeadTrackPts; ++iLeadTrks){
      std::stringstream ss;
      ss << iLeadTrks;
      h_trk_max_Pt.push_back(       book(m_name, "trkPt_"+ss.str(),        "trkPt("+ss.str()+")",    100,   -0.5,    9.5)  );
      h_trk_max_Pt_l.push_back(     book(m_name, "trkPt_"+ss.str()+"_l",       "trkPt("+ss.str()+")",    100,   -0.5,   99.5)  );
    }

  }



  // if worker is passed to the class add histograms to the output
  return StatusCode::SUCCESS;
}

StatusCode VtxHists::execute( const xAOD::VertexContainer* vtxs, float eventWeight ) {
  for(auto vtx_itr :  *vtxs ) {
    RETURN_CHECK("VtxHists::execute()", this->execute( vtx_itr, eventWeight ), "");
  }

  return StatusCode::SUCCESS;
}

StatusCode VtxHists::execute( const xAOD::VertexContainer* vtxs, const xAOD::TrackParticleContainer* trks, float eventWeight ) {
  for(auto vtx_itr :  *vtxs ) {
    RETURN_CHECK("VtxHists::execute()", this->execute( vtx_itr, trks, eventWeight ), "");
  }

  return StatusCode::SUCCESS;
}

StatusCode VtxHists::execute( const xAOD::Vertex* vtx, const xAOD::TrackParticleContainer* trks, float eventWeight ) {
  RETURN_CHECK("VtxHists::execute()", this->execute( vtx, eventWeight), "");

  if(m_fillIsoTrkDetails){

    unsigned int nTrksAll = vtx->nTrackParticles();

    uint nIsoTracks1GeV  = 0;
    uint nIsoTracks2GeV  = 0;
    uint nIsoTracks5GeV  = 0;
    uint nIsoTracks10GeV = 0;
    uint nIsoTracks15GeV = 0;
    uint nIsoTracks20GeV = 0;
    uint nIsoTracks25GeV = 0;
    uint nIsoTracks30GeV = 0;

    std::vector<float> pt_iso_vec;

    float pt_miss_iso_x = 0;
    float pt_miss_iso_y = 0;

    for(uint iTrkItr = 0; iTrkItr< nTrksAll; ++iTrkItr){
      const xAOD::TrackParticle* thisTrk = vtx->trackParticle(iTrkItr);
      float trkPt = thisTrk->pt()/1e3;


      if(trkPt < 1) continue;

      float trk_pt_cone20 = getIso(thisTrk, trks);

      pt_miss_iso_x += thisTrk->p4().Px()/1e3;
      pt_miss_iso_y += thisTrk->p4().Py()/1e3;

      h_trkIsoAll      -> Fill( trk_pt_cone20,       eventWeight );

      if(trk_pt_cone20/trkPt > 0.1) continue;

      h_trkIso         -> Fill( trk_pt_cone20,       eventWeight );

      h_IsoTrk_Pt      -> Fill( trkPt,       eventWeight );
      h_IsoTrk_Pt_l    -> Fill( trkPt,       eventWeight );

      pt_iso_vec.push_back(trkPt);

      if(trkPt >  1) ++nIsoTracks1GeV;
      if(trkPt >  2) ++nIsoTracks2GeV;
      if(trkPt >  5) ++nIsoTracks5GeV;
      if(trkPt > 10) ++nIsoTracks10GeV;
      if(trkPt > 15) ++nIsoTracks15GeV;
      if(trkPt > 20) ++nIsoTracks20GeV;
      if(trkPt > 25) ++nIsoTracks25GeV;
      if(trkPt > 30) ++nIsoTracks30GeV;

    }


    // Sort track pts
    //std::sort(numbers.begin(), numbers.end(), std::greater<int>());
    std::sort(pt_iso_vec.begin(), pt_iso_vec.end(), std::greater<float>());

    // Leading track Pts
    for(uint iLeadTrks = 0; iLeadTrks < m_nLeadIsoTrackPts; ++iLeadTrks){
      float this_pt = (pt_iso_vec.size() > iLeadTrks) ? pt_iso_vec.at(iLeadTrks) : 0;
      h_IsoTrk_max_Pt.at(iLeadTrks)      -> Fill( this_pt,       eventWeight );
      h_IsoTrk_max_Pt_l.at(iLeadTrks)    -> Fill( this_pt,       eventWeight );
    }

    h_nIsoTrks1GeV       -> Fill( nIsoTracks1GeV,        eventWeight );
    h_nIsoTrks2GeV       -> Fill( nIsoTracks2GeV,        eventWeight );
    h_nIsoTrks5GeV       -> Fill( nIsoTracks5GeV,        eventWeight );
    h_nIsoTrks10GeV      -> Fill( nIsoTracks10GeV,       eventWeight );
    h_nIsoTrks15GeV      -> Fill( nIsoTracks15GeV,       eventWeight );
    h_nIsoTrks20GeV      -> Fill( nIsoTracks20GeV,       eventWeight );
    h_nIsoTrks25GeV      -> Fill( nIsoTracks25GeV,       eventWeight );
    h_nIsoTrks30GeV      -> Fill( nIsoTracks30GeV,       eventWeight );

    h_pt_miss_iso_x      -> Fill(pt_miss_iso_x ,       eventWeight );
    h_pt_miss_iso_x_l    -> Fill(pt_miss_iso_x ,       eventWeight );

    h_pt_miss_iso_y      -> Fill(pt_miss_iso_y ,       eventWeight );
    h_pt_miss_iso_y_l    -> Fill(pt_miss_iso_y ,       eventWeight );

    float pt_miss_iso = sqrt(pt_miss_iso_x*pt_miss_iso_x + pt_miss_iso_y*pt_miss_iso_y);
    h_pt_miss_iso      -> Fill(pt_miss_iso ,       eventWeight );
    h_pt_miss_iso_l    -> Fill(pt_miss_iso ,       eventWeight );

  }

  return StatusCode::SUCCESS;
}

StatusCode VtxHists::execute( const xAOD::Vertex* vtx, float eventWeight ) {

  //basic
  h_type       -> Fill( vtx->vertexType(),            eventWeight );

  unsigned int nTrks = vtx->nTrackParticles();
  h_nTrks      -> Fill( nTrks,       eventWeight );
  h_nTrks_l    -> Fill( nTrks,       eventWeight );

  if(m_fillTrkDetails){

    uint nTracks1GeV = 0;
    uint nTracks2GeV = 0;
    uint nTracks5GeV = 0;
    uint nTracks10GeV = 0;
    uint nTracks15GeV = 0;
    uint nTracks20GeV = 0;
    uint nTracks25GeV = 0;
    uint nTracks30GeV = 0;

    std::vector<float> pt_vec;

    float pt_miss_x = 0;
    float pt_miss_y = 0;

    for(uint iTrkItr = 0; iTrkItr< nTrks; ++iTrkItr){
      const xAOD::TrackParticle* thisTrk = vtx->trackParticle(iTrkItr);
      float trkPt = thisTrk->pt()/1e3;

      h_trk_Pt      -> Fill( trkPt,       eventWeight );
      h_trk_Pt_l    -> Fill( trkPt,       eventWeight );

      if(!m_fillTrkDetails) continue;

      if(trkPt > 1) pt_vec.push_back(trkPt);

      pt_miss_x += thisTrk->p4().Px()/1e3;
      pt_miss_y += thisTrk->p4().Py()/1e3;

      if(trkPt >  1) ++nTracks1GeV;
      if(trkPt >  2) ++nTracks2GeV;
      if(trkPt >  5) ++nTracks5GeV;
      if(trkPt > 10) ++nTracks10GeV;
      if(trkPt > 15) ++nTracks15GeV;
      if(trkPt > 20) ++nTracks20GeV;
      if(trkPt > 25) ++nTracks25GeV;
      if(trkPt > 30) ++nTracks30GeV;

    }

    if(m_fillTrkPtDetails){

      // Sort track pts
      //std::sort(numbers.begin(), numbers.end(), std::greater<int>());
      std::sort(pt_vec.begin(), pt_vec.end(), std::greater<float>());

      // Leading track Pts
      for(uint iLeadTrks = 0; iLeadTrks < m_nLeadTrackPts; ++iLeadTrks){
	float this_pt = (pt_vec.size() > iLeadTrks) ? pt_vec.at(iLeadTrks) : 0;
	h_trk_max_Pt.at(iLeadTrks)      -> Fill( this_pt,       eventWeight );
	h_trk_max_Pt_l.at(iLeadTrks)    -> Fill( this_pt,       eventWeight );
      }

      h_nTrks1GeV       -> Fill( nTracks1GeV,        eventWeight );
      h_nTrks2GeV       -> Fill( nTracks2GeV,        eventWeight );
      h_nTrks5GeV       -> Fill( nTracks5GeV,        eventWeight );
      h_nTrks10GeV      -> Fill( nTracks10GeV,       eventWeight );
      h_nTrks15GeV      -> Fill( nTracks15GeV,       eventWeight );
      h_nTrks20GeV      -> Fill( nTracks20GeV,       eventWeight );
      h_nTrks25GeV      -> Fill( nTracks25GeV,       eventWeight );
      h_nTrks30GeV      -> Fill( nTracks30GeV,       eventWeight );

      h_pt_miss_x      -> Fill(pt_miss_x ,       eventWeight );
      h_pt_miss_x_l    -> Fill(pt_miss_x ,       eventWeight );

      h_pt_miss_y      -> Fill(pt_miss_y ,       eventWeight );
      h_pt_miss_y_l    -> Fill(pt_miss_y ,       eventWeight );

      float pt_miss = sqrt(pt_miss_x*pt_miss_x + pt_miss_y*pt_miss_y);
      h_pt_miss      -> Fill(pt_miss ,       eventWeight );
      h_pt_miss_l    -> Fill(pt_miss ,       eventWeight );
    }

  }

  return StatusCode::SUCCESS;

}

float VtxHists::getIso( const xAOD::TrackParticle *inTrack,            const xAOD::TrackParticleContainer* trks, float z0_cut , float cone_size)
{
  float iso = 0;

  for(auto trk_itr :  *trks ) {

    float dZ0 = abs(trk_itr->z0() - inTrack->z0());
    h_dZ0Before->Fill(dZ0, 1.0);
    if(dZ0 > z0_cut) continue;

    float dR = trk_itr->p4().DeltaR(inTrack->p4());
    if(dR > cone_size) continue;
    if(dR == 0) continue;
    iso += trk_itr->pt()/1e3;
  }

  return iso;
}
