// Gmsh - Copyright (C) 1997-2015 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <gmsh@geuz.org>.

#ifndef _QUALITY_MEASURES_JACOBIAN_H_
#define _QUALITY_MEASURES_JACOBIAN_H_

#include <vector>
#include "fullMatrix.h"

class GradientBasis;
class bezierBasis;
class MElement;

namespace jacobianBasedQuality {

void minMaxJacobianDeterminant(MElement *el, double &min, double &max);
double minScaledJacobian(MElement *el);
double minAnisotropyMeasure(MElement *el, int n = 5);
//double minSampledAnisotropyMeasure(MElement *el, int order,//fordebug
//                                   bool writeInFile = false);

class _CoeffData
{
protected:
  double _minL, _maxL; //Extremum of Jac at corners
  double _minB, _maxB; //Extremum of bezier coefficients
  int _depth;

public:
  _CoeffData(int depth) : _minL(0), _maxL(0), _minB(0), _maxB(0),
                         _depth(depth) {}
  virtual ~_CoeffData() {}

  inline double minL() const {return _minL;}
  inline double maxL() const {return _maxL;}
  inline double minB() const {return _minB;}
  inline double maxB() const {return _maxB;}

  virtual bool boundsOk(double minL, double maxL) const = 0;
  virtual void getSubCoeff(std::vector<_CoeffData*>&) const = 0;
};

struct _lessMinB {
  bool operator()(_CoeffData*, _CoeffData*) const;
};
struct _lessMaxB {
  bool operator()(_CoeffData*, _CoeffData*) const;
};

class _CoeffDataJac: public _CoeffData
{
private:
  const fullVector<double> _coeffs;
  const bezierBasis *_bfs;

public:
  _CoeffDataJac(fullVector<double> &v, const bezierBasis *bfs, int depth);
  ~_CoeffDataJac() {}

  bool boundsOk(double minL, double maxL) const;
  void getSubCoeff(std::vector<_CoeffData*>&) const;
};

class _CoeffDataScaledJac: public _CoeffData
{
private:
  const fullVector<double> _coeffsJacDet;
  const fullMatrix<double> _coeffsJacMat;
  const bezierBasis *_bfsDet, *_bfsMat;

public:
  _CoeffDataScaledJac(fullVector<double> &det,
                     fullMatrix<double> &mat,
                     const bezierBasis *bfsDet,
                     const bezierBasis *bfsMat,
                     int depth);
  ~_CoeffDataScaledJac() {}

  bool boundsOk(double minL, double maxL) const;
  void getSubCoeff(std::vector<_CoeffData*>&) const;

private:
  void _computeAtCorner(double &min, double &max) const;
  double _computeLowerBound() const;
};

class _CoeffDataAnisotropy: public _CoeffData
{
private:
  const fullVector<double> _coeffsJacDet;
  const fullMatrix<double> _coeffsMetric;
  const bezierBasis *_bfsDet, *_bfsMet;
  MElement *_elForDebug;
  int _numForDebug;
  static bool hasError;//fordebug
  static std::fstream file;//fordebug

public:
  static std::vector<double> mytimes;//fordebug
  static std::vector<int> mynumbers;//fordebug
  _CoeffDataAnisotropy(fullVector<double> &det,
                       fullMatrix<double> &metric,
                       const bezierBasis *bfsDet,
                       const bezierBasis *bfsMet,
                       int depth, MElement *, int num = 0);
  ~_CoeffDataAnisotropy() {}

  bool boundsOk(double minL, double maxL) const;
  void getSubCoeff(std::vector<_CoeffData*>&) const;

  static int metricOrder(MElement *el);
  static void fillMetricCoeff(const GradientBasis*,
                              const fullMatrix<double> &nodes,
                              fullMatrix<double> &coeff,
                              int dim, bool ideal);

  static bool noMoreComputationPlease() {return hasError;}//fordebug
  static void youReceivedAnError() {hasError = true;}//fordebug

private:
  void _computeAtCorner(double &min, double &max) const;
  double _computeLowerBound() const;

  double _computeLowerBoundA() const;
  double _computeLowerBoundK() const;
  void _computeBoundingCurve(double &beta, double gamma, bool lowerBound) const;

  static bool _moveInsideDomain(double &a, double &K, bool bottomleftCorner);
  static bool _computePseudoDerivatives(double a, double K,
                                        double &dRda, double &dRdK);
  static bool _computeDerivatives(double a, double K,
                                  double &dRda, double &dRdK,
                                  double &dRdaa, double &dRdaK, double &dRdKK);
  static bool _intersectionCurveLeftCorner(double beta, double gamma,
                                           double &mina, double &minK);
  static double _computeAbscissaMinR(double aStart, double K);

  static double _R2Dsafe(double q, double p);
  static double _R2Dsafe(double a);
  static double _R3Dsafe(double q, double p, double J);
  static double _R3Dsafe(double a, double K);
  static double _wSafe(double a, double K);
  static inline double _w(double a, double K) {
    return .5 * (K + (3-a*a)*a);
  }

public:
  void interpolateForMatlab(const fullMatrix<double> &nodes) const;//fordebug
  void statsForMatlab(MElement *el, int) const;//fordebug
};

//inline bool isValid(MElement *el) {
//  double min, max;
//  minMaxJacobianDeterminant(el, min, max);
//  return min > 0;
//}

double _computeBoundRational(const fullVector<double> &numerator,
                             const fullVector<double> &denominator,
                             bool lower, bool positiveDenom = true);

void _subdivideDomains(std::vector<_CoeffData*> &domains);

}

#endif
