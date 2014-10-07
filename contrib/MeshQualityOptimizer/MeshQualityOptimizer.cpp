// TODO: Copyright

//#include "GModel.h"
#include "GEntity.h"
#include "MElement.h"
#include "MTriangle.h"
#include "MQuadrangle.h"
#include "qualityMeasures.h"
#include "MeshOptCommon.h"
#include "MeshOptObjContribFunc.h"
#include "MeshOptObjContrib.h"
#include "MeshOptObjContribScaledNodeDispSq.h"
#include "MeshQualityObjContribIdealJac.h"
#include "MeshQualityObjContribInvCond.h"
#include "MeshOptimizer.h"
#include "MeshQualityOptimizer.h"


struct QualPatchDefParameters : public MeshOptPatchDef
{
  QualPatchDefParameters(const MeshQualOptParameters &p);
  virtual ~QualPatchDefParameters() {}
  virtual double elBadness(MElement *el) const;
  virtual double maxDistance(MElement *el) const;
  virtual int inPatch(const SPoint3 &badBary,
                      double limDist, MElement *el) const;
private:
  bool _excludeHex, _excludePrism;
  double _idealJacMin;
  double _distanceFactor;
};


QualPatchDefParameters::QualPatchDefParameters(const MeshQualOptParameters &p)
{
  _excludeHex = p.excludeHex;
  _excludePrism = p.excludePrism;
  _idealJacMin = p.minTargetIdealJac;
  strategy = (p.strategy == 1) ? MeshOptParameters::STRAT_ONEBYONE :
                                        MeshOptParameters::STRAT_CONNECTED;
  minLayers = (p.dim == 3) ? 1 : 0;
  maxLayers = p.nbLayers;
  _distanceFactor = p.distanceFactor;
  if (strategy == MeshOptParameters::STRAT_CONNECTED)
    weakMerge = (p.strategy == 2);
  else {
    maxAdaptPatch = p.maxAdaptBlob;
    maxLayersAdaptFact = p.adaptBlobLayerFact;
    distanceAdaptFact = p.adaptBlobDistFact;
  }
}


double QualPatchDefParameters::elBadness(MElement *el) const
{
  const int typ = el->getType();
  if (_excludeHex && (typ == TYPE_HEX)) return 1.;
  if (_excludePrism && (typ == TYPE_PRI)) return 1.;
  double jMin, jMax;
  el->idealJacRange(jMin, jMax);
  return jMin-_idealJacMin;
}


double QualPatchDefParameters::maxDistance(MElement *el) const
{
  return _distanceFactor * el->maxEdge();
}


int QualPatchDefParameters::inPatch(const SPoint3 &badBary,
                                    double limDist, MElement *el) const
{
  const int typ = el->getType();
  if ((typ == TYPE_HEX) || (typ == TYPE_PRI)) return -1;
  return testElInDist(badBary, limDist, el) ? 1 : 0;
}


void MeshQualityOptimizer(GModel *gm, MeshQualOptParameters &p)
{
  Msg::StatusBar(true, "Optimizing mesh quality...");

  MeshOptParameters par;
  par.dim = p.dim;
  par.onlyVisible = p.onlyVisible;
  par.fixBndNodes = p.fixBndNodes;
  QualPatchDefParameters patchDef(p);
  par.patchDef = &patchDef;
  par.optDisplay = 20;
  par.verbose = 4;

  ObjContribScaledNodeDispSq<ObjContribFuncSimple> nodeDistFunc(p.weightFixed, p.weightFree,
                                                                Patch::LS_MINEDGELENGTH);
  ObjContribIdealJac<ObjContribFuncBarrierMovMin> minIdealJacBarFunc(1.);
  minIdealJacBarFunc.setTarget(p.minTargetIdealJac, 1.);

  MeshOptPass minJacPass;
  minJacPass.barrierIterMax = p.optPassMax;
  minJacPass.optIterMax = p.itMax;
  minJacPass.contrib.push_back(&nodeDistFunc);
  minJacPass.contrib.push_back(&minIdealJacBarFunc);
  par.pass.push_back(minJacPass);

  meshOptimizer(gm, par);

  p.CPU = par.CPU;
  p.minIdealJac = minIdealJacBarFunc.getMin();
  p.maxIdealJac = minIdealJacBarFunc.getMax();
}