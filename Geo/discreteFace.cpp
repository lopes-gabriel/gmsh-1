// Gmsh - Copyright (C) 1997-2019 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// issues on https://gitlab.onelab.info/gmsh/gmsh/issues.

#include <stdlib.h>
#include <queue>
#include "GmshMessage.h"
#include "discreteFace.h"
#include "GModelIO_GEO.h"
#include "Geo.h"
#include "Context.h"
#include "MPoint.h"

#include "MElementOctree.h"
#include "Octree.h"

discreteFace::discreteFace(GModel *model, int num) : GFace(model, num)
{
  Surface *s = CreateSurface(num, MSH_SURF_DISCRETE);
  Tree_Add(model->getGEOInternals()->Surfaces, &s);
  meshStatistics.status = GFace::DONE;
  _current_parametrization = -1;
}

void discreteFace::setBoundEdges(const std::vector<int> &tagEdges)
{
  for(unsigned int i = 0; i != tagEdges.size(); i++) {
    GEdge *ge = model()->getEdgeByTag(tagEdges[i]);
    if(ge) {
      l_edges.push_back(ge);
      l_dirs.push_back(1);
      ge->addFace(this);
    }
    else {
      Msg::Error("Unknown curve %d in discrete surface %d",
                 tagEdges[i], tag());
    }
  }
}

void discreteFace::setBoundEdges(const std::vector<int> &tagEdges,
                                 const std::vector<int> &signEdges)
{
  if(signEdges.size() != tagEdges.size()) {
    Msg::Error("Wrong number of edge signs in discrete surface %d", tag());
    setBoundEdges(tagEdges);
  }
  for(std::vector<int>::size_type i = 0; i != tagEdges.size(); i++) {
    GEdge *ge = model()->getEdgeByTag(tagEdges[i]);
    if(ge) {
      l_edges.push_back(ge);
      l_dirs.push_back(signEdges[i]);
      ge->addFace(this);
    }
    else {
      Msg::Error("Unknown curve %d in discrete surface %d",
                 tagEdges[i], tag());
    }
  }
}

int discreteFace::trianglePosition(double par1, double par2, double &u,
                                   double &v) const
{
#if defined(HAVE_HXT)
  if(_parametrizations.empty()) return 0;

  double xy[3] = {par1, par2, 0};
  double uv[3];
  const MElement *e =
    _parametrizations[_current_parametrization].oct->find(par1, par2, 0.0);
  if(!e) return -1;
  e->xyz2uvw(xy, uv);
  int position =
    (int)((MTriangle *)e - &_parametrizations[_current_parametrization].t2d[0]);
  u = uv[0];
  v = uv[1];
  return position;
#else
  return 0;
#endif
}

GPoint discreteFace::point(double par1, double par2) const
{
#if defined(HAVE_HXT)
  if(_parametrizations.empty()) return 0;

  double xy[3] = {par1, par2, 0};
  double uv[3];
  const MElement *e =
    _parametrizations[_current_parametrization].oct->find(par1, par2, 0.0);
  if(!e) {
    GPoint gp = GPoint(1.e21, 1.e21, 1.e21, this, xy);
    gp.setNoSuccess();
    return gp;
  }
  e->xyz2uvw(xy, uv);
  int position =
    (int)((MTriangle *)e - &_parametrizations[_current_parametrization].t2d[0]);
  const MTriangle &t3d =
    _parametrizations[_current_parametrization].t3d[position];

  double X = 0, Y = 0, Z = 0;
  double eval[3] = {1. - uv[0] - uv[1], uv[0], uv[1]};

  for(int io = 0; io < 3; io++) {
    X += t3d.getVertex(io)->x() * eval[io];
    Y += t3d.getVertex(io)->y() * eval[io];
    Z += t3d.getVertex(io)->z() * eval[io];
  }
  return GPoint(X, Y, Z, this, xy);
#else
  Msg::Error("Cannot evaluate point on discrete surface without HXT");
  return GPoint();
#endif
}

class dfWrapper {
public:
  SPoint3 _p;
  double _distance;
  SPoint3 _closestPoint;
  MTriangle *_t3d, *_t2d;
  dfWrapper(const SPoint3 &p) : _p(p), _distance(1.e22), _t3d(NULL), _t2d(NULL)
  {
  }
};

#if defined(HAVE_HXT)

static SVector3 _NORMAL_(const MTriangle &t3d)
{
  SVector3 v31(t3d.getVertex(2)->x() - t3d.getVertex(0)->x(),
               t3d.getVertex(2)->y() - t3d.getVertex(0)->y(),
               t3d.getVertex(2)->z() - t3d.getVertex(0)->z());
  SVector3 v21(t3d.getVertex(1)->x() - t3d.getVertex(0)->x(),
               t3d.getVertex(1)->y() - t3d.getVertex(0)->y(),
               t3d.getVertex(1)->z() - t3d.getVertex(0)->z());
  SVector3 n = crossprod(v31, v21);

  n.normalize();
  return n;
}

#endif

bool discreteFace_rtree_callback(std::pair<MTriangle *, MTriangle *> *t,
                                 void *w)
{
  dfWrapper *wrapper = static_cast<dfWrapper *>(w);

  SPoint3 closePt;
  double d;
  //  printf("distance %p %p\n", t->first, t->second);
  signedDistancePointTriangle(
    SPoint3(t->first->getVertex(0)->x(), t->first->getVertex(0)->y(),
            t->first->getVertex(0)->z()),
    SPoint3(t->first->getVertex(1)->x(), t->first->getVertex(1)->y(),
            t->first->getVertex(1)->z()),
    SPoint3(t->first->getVertex(2)->x(), t->first->getVertex(2)->y(),
            t->first->getVertex(2)->z()),
    wrapper->_p, d, closePt);
  //  printf("%g\n",d);
  if(fabs(d) < wrapper->_distance) {
    wrapper->_distance = fabs(d);
    wrapper->_closestPoint = closePt;
    wrapper->_t3d = t->first;
    wrapper->_t2d = t->second;
  }

  return true;
}

GPoint discreteFace::closestPoint(const SPoint3 &queryPoint, double maxDistance,
                                  SVector3 *normal) const
{
#if defined(HAVE_HXT)
  if(_parametrizations.empty()) return GPoint();
  if(_current_parametrization == -1) return GPoint();

  dfWrapper wrapper(queryPoint);
  do {
    wrapper._distance = 1.e22;
    double MIN[3] = {queryPoint.x() - maxDistance, queryPoint.y() - maxDistance,
                     queryPoint.z() - maxDistance};
    double MAX[3] = {queryPoint.x() + maxDistance, queryPoint.y() + maxDistance,
                     queryPoint.z() + maxDistance};
    _parametrizations[_current_parametrization].rtree3d.Search(
      MIN, MAX, discreteFace_rtree_callback, &wrapper);
    maxDistance *= 2.0;
  } while(!wrapper._t3d);

  if(normal) {
    SVector3 t1(
      wrapper._t3d->getVertex(1)->x() - wrapper._t3d->getVertex(0)->x(),
      wrapper._t3d->getVertex(1)->y() - wrapper._t3d->getVertex(0)->y(),
      wrapper._t3d->getVertex(1)->z() - wrapper._t3d->getVertex(0)->z());
    SVector3 t2(
      wrapper._t3d->getVertex(2)->x() - wrapper._t3d->getVertex(0)->x(),
      wrapper._t3d->getVertex(2)->y() - wrapper._t3d->getVertex(0)->y(),
      wrapper._t3d->getVertex(2)->z() - wrapper._t3d->getVertex(0)->z());
    *normal = crossprod(t1, t2);
    normal->normalize();
  }

  double xyz[3] = {wrapper._closestPoint.x(), wrapper._closestPoint.y(),
                   wrapper._closestPoint.z()};
  double uvw[3];
  wrapper._t3d->xyz2uvw(xyz, uvw);
  const MVertex *v0 = wrapper._t2d->getVertex(0);
  const MVertex *v1 = wrapper._t2d->getVertex(1);
  const MVertex *v2 = wrapper._t2d->getVertex(2);
  double U = 1 - uvw[0] - uvw[1];
  double V = uvw[0];
  double W = uvw[1];
  SPoint2 pp(U * v0->x() + V * v1->x() + W * v2->x(),
             U * v0->y() + V * v1->y() + W * v2->y());
  return GPoint(xyz[0], xyz[1], xyz[2], this, pp);
#else
  return GPoint();
#endif
}

GPoint discreteFace::closestPoint(const SPoint3 &queryPoint,
                                  const double initialGuess[2]) const
{
#if defined(HAVE_HXT)
  return closestPoint(queryPoint, 0.0001);
#else
  Msg::Error("Cannot evaluate closest point on discrete surface without HXT");
  return GPoint();
#endif
}

SPoint2 discreteFace::parFromPoint(const SPoint3 &p, bool onSurface) const
{
#if defined(HAVE_HXT)
  GPoint gp = closestPoint(p, 0.0001);
  return SPoint2(gp.u(), gp.v());
#else
  Msg::Error("Cannot evaluate par from point on discrete surface without HXT");
  return SPoint2();
#endif
}

SVector3 discreteFace::normal(const SPoint2 &param) const
{
#if defined(HAVE_HXT)
  if(_parametrizations.empty()) return SVector3();
  if(_current_parametrization == -1) return SVector3();

  MElement *e = _parametrizations[_current_parametrization].oct->find(
    param.x(), param.y(), 0.0);
  if(!e) {
    Msg::Warning("Triangle not found at uv=(%g,%g) on discrete surface %d",
                 param.x(), param.y(), tag());
    return SVector3(0, 0, 1);
  }
  int position =
    (int)((MTriangle *)e - &_parametrizations[_current_parametrization].t2d[0]);
  const MTriangle &t3d =
    _parametrizations[_current_parametrization].t3d[position];
  return _NORMAL_(t3d);
#else
  Msg::Error("Cannot evaluate normal on discrete surface without HXT");
  return SVector3();
#endif
}

double discreteFace::curvatureMax(const SPoint2 &param) const {
  SVector3 dirMax, dirMin;
  double c, C;
  if(_current_parametrization==-1)
    return 0.0;
  curvatures(param, dirMax, dirMin, C, c);
  return std::max(c,C);
}

double discreteFace::curvatures(const SPoint2 &param, SVector3 &dirMax,
                                SVector3 &dirMin, double &curvMax,
                                double &curvMin) const
{
  if(_parametrizations.empty())
    return 0.0;
  if(_current_parametrization==-1)
    return 0.0;

  MElement *e = _parametrizations[_current_parametrization].oct->find(
    param.x(), param.y(), 0.0);
  if(!e) {
    Msg::Warning("Triangle not found for curvatures at uv=(%g,%g) on "
                 "discrete surface %d", param.x(), param.y(), tag());
    return 0.0;
  }
  
  int position =
    (int)((MTriangle *)e - &_parametrizations[_current_parametrization].t2d[0]);

  //  if (6*position+5 >= _parametrizations[_current_parametrization].CURV.size())printf("error\n");

  //  printf("coucou1\n");
  SVector3 c0max = _parametrizations[_current_parametrization].CURV[6*position+0];
  SVector3 c1max = _parametrizations[_current_parametrization].CURV[6*position+1];
  SVector3 c2max = _parametrizations[_current_parametrization].CURV[6*position+2];
  SVector3 c0min = _parametrizations[_current_parametrization].CURV[6*position+3];
  SVector3 c1min = _parametrizations[_current_parametrization].CURV[6*position+4];
  SVector3 c2min = _parametrizations[_current_parametrization].CURV[6*position+5];

  curvMax = c0max.norm();
  curvMin = c0min.norm();
    
  dirMax = c0max.normalize();
  dirMin = c0min.normalize();
  
  return false;
}

Pair<SVector3, SVector3> discreteFace::firstDer(const SPoint2 &param) const
{
#if defined(HAVE_HXT)
  if(_current_parametrization == -1) return Pair<SVector3, SVector3>(SVector3(), SVector3());
  if(_parametrizations.empty())
    return Pair<SVector3, SVector3>(SVector3(), SVector3());

  MElement *e = _parametrizations[_current_parametrization].oct->find(
    param.x(), param.y(), 0.0);
  if(!e) {
    Msg::Warning("Triangle not found for first derivative at uv=(%g,%g) on "
                 "discrete surface %d", param.x(), param.y(), tag());
    return Pair<SVector3, SVector3>(SVector3(1, 0, 0), SVector3(0, 1, 0));
  }

  int position =
    (int)((MTriangle *)e - &_parametrizations[_current_parametrization].t2d[0]);

  const MTriangle &t3d =
    _parametrizations[_current_parametrization].t3d[position];
  const MVertex *v1 = t3d.getVertex(0);
  const MVertex *v2 = t3d.getVertex(1);
  const MVertex *v3 = t3d.getVertex(2);

  double M3D[3][2] = {{v2->x() - v1->x(), v3->x() - v1->x()},
                      {v2->y() - v1->y(), v3->y() - v1->y()},
                      {v2->z() - v1->z(), v3->z() - v1->z()}};
  v1 = e->getVertex(0);
  v2 = e->getVertex(1);
  v3 = e->getVertex(2);

  double M2D[2][2] = {{(v3->y() - v1->y()), -(v3->x() - v1->x())},
                      {-(v2->y() - v1->y()), (v2->x() - v1->x())}};

  double det = 1. / (M2D[0][0] * M2D[1][1] - M2D[1][0] * M2D[0][1]);

  //  printf("det = %12.5E\n",det);

  double dxdu[3][2];

  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 2; j++) {
      dxdu[i][j] = 0.;
      for(int k = 0; k < 2; k++) {
        dxdu[i][j] += det * M3D[i][k] * M2D[k][j];
      }
    }
  }

  return Pair<SVector3, SVector3>(SVector3(dxdu[0][0], dxdu[1][0], dxdu[2][0]),
                                  SVector3(dxdu[0][1], dxdu[1][1], dxdu[2][1]));
#endif
  Msg::Error("Cannot evaluate first derivative on discrete surface without HXT");
  return Pair<SVector3, SVector3>(SVector3(1, 0, 0), SVector3(0, 0, 0));
}

void discreteFace::secondDer(const SPoint2 &param, SVector3 &dudu,
			       SVector3 &dvdv, SVector3 &dudv) const
{
  return;
}

void discreteFace::createGeometry()
{
#if defined(HAVE_HXT)
  if(_parametrizations.size()) return;
  checkAndFixOrientation();
  HXTStatus s = reparametrize_through_hxt();
  if(s != HXT_STATUS_OK)
    Msg::Error("Could not create geometry of discrete surface %d", tag());
#endif
}

void discreteFace::checkAndFixOrientation()
{
  // first of all, all the triangles have to be oriented in the same way
  std::map<MEdge, std::vector<MElement *>, Less_Edge>
    ed2tri; // edge to 1 or 2 triangle(s)

  for(unsigned int i = 0; i < triangles.size(); ++i) {
    MElement *e = triangles[i];
    for(int j = 0; j < e->getNumEdges(); j++) {
      MEdge ed = e->getEdge(j);
      ed2tri[ed].push_back(e);
    }
  }

  // element to its neighbors
  std::map<MElement *, std::vector<MElement *> > neighbors;
  for(unsigned int i = 0; i < triangles.size(); ++i) {
    MElement *e = triangles[i];
    for(int j = 0; j < e->getNumEdges(); j++) {
      // #improveme: efficiency could be improved by setting neighbors mutually
      std::vector<MElement *> my_mt = ed2tri[e->getEdge(j)];
      if(my_mt.size() > 1) { // my_mt.size() = {1;2}
        MElement *neighTri = my_mt[0] == e ? my_mt[1] : my_mt[0];
        neighbors[e].push_back(neighTri);
      }
    }
  }

  // element for reference orientation
  std::queue<MElement *> checkList;
  // corresponding neighbor element to be checked for its orientation
  std::queue<std::vector<MElement *> > checkLists;
  // todo list
  std::queue<MElement *> my_todo;
  // help to complete todo list
  std::map<MElement *, bool> check_todo;

  my_todo.push(triangles[0]);

  check_todo[triangles[0]] = true;
  while(!my_todo.empty()) {
    MElement *myMT = my_todo.front();
    my_todo.pop();

    std::vector<MElement *> myV = neighbors[myMT];
    std::vector<MElement *> myInsertion;

    checkList.push(myMT);

    for(unsigned int i = 0; i < myV.size(); ++i) {
      if(check_todo.find(myV[i]) == check_todo.end()) {
        myInsertion.push_back(myV[i]);
        check_todo[myV[i]] = true;
        my_todo.push(myV[i]);
      }
    }
    checkLists.push(myInsertion);
  }

  while(!checkList.empty() && !checkLists.empty()) {
    MElement *current = checkList.front();
    checkList.pop();
    std::vector<MElement *> neigs = checkLists.front();
    checkLists.pop();
    for(unsigned int i = 0; i < neigs.size(); i++) {
      bool myCond = false;
      for(unsigned int k = 0; k < 3; k++) {
        for(unsigned int j = 0; j < 3; j++) {
          if(current->getVertex(k) == neigs[i]->getVertex(j)) {
            myCond = true;
            if(!(current->getVertex(k != 2 ? k + 1 : 0) ==
                   neigs[i]->getVertex(j != 0 ? j - 1 : 2) ||
                 current->getVertex(k != 0 ? k - 1 : 2) ==
                   neigs[i]->getVertex(j != 2 ? j + 1 : 0))) {
              neigs[i]->reverse();
            }
            break;
          }
        }
        if(myCond) break;
      }
    }
  }
}

void discreteFace::mesh(bool verbose)
{
#if defined(HAVE_HXT)
  if(_parametrizations.empty()) return;

  std::vector<MTriangle *> _t;
  std::vector<MVertex *> _v;
  std::vector<GEdge *> const tmp = l_edges;
  std::vector<GVertex *> _gv;

  Msg::Info ("Meshing discrete surface %d: the atlas contains %d maps",tag(),_parametrizations.size());

  for(size_t i = 0; i < _parametrizations.size(); i++) {
    l_edges.clear();
    for (size_t j=0;j<_parametrizations[i].bnd.size(); j++){
      //      printf("%d ",_parametrizations[i].bnd[j]->tag());
      if (_parametrizations[i].bnd[j]->geomType() == DiscreteCurve)
	((discreteEdge*)_parametrizations[i].bnd[j])->getSplit(l_edges,_gv);
      else
	l_edges.push_back(_parametrizations[i].bnd[j]);
    }
    //    printf("\n");
    embedded_edges.clear();
    embedded_edges.insert(embedded_edges.begin(),
                          _parametrizations[i].emb.begin(),
                          _parametrizations[i].emb.end());
    _current_parametrization = i;
    triangles.clear();
    mesh_vertices.clear();
    GFace::mesh(verbose);
    _t.insert(_t.begin(), triangles.begin(), triangles.end());
    _v.insert(_v.begin(), mesh_vertices.begin(), mesh_vertices.end());
  }
  triangles = _t;
  mesh_vertices = _v;
  l_edges = tmp;
  embedded_edges.clear();
  meshStatistics.status = GFace::DONE;
#else
  Msg::Error("Cannot mesh discrete surface without HXT");
#endif
}

#if defined(HAVE_HXT)

HXTStatus gmsh2hxt(GFace *gf, HXTMesh **pm, std::map<MVertex *, int> &v2c,
                   std::vector<MVertex *> &c2v)
{
  HXTContext *context;
  hxtContextCreate(&context);
  HXTMesh *m;
  HXT_CHECK(hxtMeshCreate(context, &m));
  std::set<MVertex *> all;
  for(size_t i = 0; i < gf->triangles.size(); i++) {
    all.insert(gf->triangles[i]->getVertex(0));
    all.insert(gf->triangles[i]->getVertex(1));
    all.insert(gf->triangles[i]->getVertex(2));
  }
  m->vertices.num = m->vertices.size = all.size();
  HXT_CHECK(
    hxtAlignedMalloc(&m->vertices.coord, 4 * m->vertices.num * sizeof(double)));

  size_t count = 0;
  c2v.resize(all.size());
  for(std::set<MVertex *>::iterator it = all.begin(); it != all.end(); it++) {
    m->vertices.coord[4 * count + 0] = (*it)->x();
    m->vertices.coord[4 * count + 1] = (*it)->y();
    m->vertices.coord[4 * count + 2] = (*it)->z();
    m->vertices.coord[4 * count + 3] = 0.0;
    v2c[*it] = count;
    c2v[count++] = *it;
  }
  all.clear();

  m->triangles.num = m->triangles.size = gf->triangles.size();
  HXT_CHECK(hxtAlignedMalloc(&m->triangles.node,
                             (m->triangles.num) * 3 * sizeof(uint32_t)));
  HXT_CHECK(hxtAlignedMalloc(&m->triangles.colors,
                             (m->triangles.num) * sizeof(uint16_t)));
  for(size_t i = 0; i < gf->triangles.size(); i++) {
    m->triangles.node[3 * i + 0] = v2c[gf->triangles[i]->getVertex(0)];
    m->triangles.node[3 * i + 1] = v2c[gf->triangles[i]->getVertex(1)];
    m->triangles.node[3 * i + 2] = v2c[gf->triangles[i]->getVertex(2)];
    m->triangles.colors[i] = gf->tag();
  }

  m->lines.num = m->lines.size = 0;

  *pm = m;
  return HXT_STATUS_OK;
}

#endif

GPoint discreteFace::intersectionWithCircle(const SVector3 &n1,
                                            const SVector3 &n2,
                                            const SVector3 &p, const double &R,
                                            double uv[2])
{
#if defined(HAVE_HXT)
  if(_parametrizations.empty()) return GPoint();

  MTriangle *t2d =
    (MTriangle *)_parametrizations[_current_parametrization].oct->find(
      uv[0], uv[1], 0.0);
  MTriangle *t3d = NULL;
  if(t2d) {
    int position =
      (int)(t2d - &_parametrizations[_current_parametrization].t2d[0]);
    t3d = &_parametrizations[_current_parametrization].t3d[position];
  }

  SVector3 n = crossprod(n1, n2);
  n.normalize();

  int N = _parametrizations[_current_parametrization].t3d.size();
  int start = 0;
  if(t2d) start = -1;
  for(int i = start; i < N; i++) {
    if(i >= 0) {
      t2d = &_parametrizations[_current_parametrization].t2d[i];
      t3d = &_parametrizations[_current_parametrization].t3d[i];
    }
    SVector3 v0(t3d->getVertex(0)->x(), t3d->getVertex(0)->y(),
                t3d->getVertex(0)->z());
    SVector3 v1(t3d->getVertex(1)->x(), t3d->getVertex(1)->y(),
                t3d->getVertex(1)->z());
    SVector3 v2(t3d->getVertex(2)->x(), t3d->getVertex(2)->y(),
                t3d->getVertex(2)->z());
    SVector3 v0_2d(t2d->getVertex(0)->x(), t2d->getVertex(0)->y(),
                   t2d->getVertex(0)->z());
    SVector3 v1_2d(t2d->getVertex(1)->x(), t2d->getVertex(1)->y(),
                   t2d->getVertex(1)->z());
    SVector3 v2_2d(t2d->getVertex(2)->x(), t2d->getVertex(2)->y(),
                   t2d->getVertex(2)->z());
    SVector3 t1 = v1 - v0;
    SVector3 t2 = v2 - v0;
    SVector3 t = crossprod(t1, t2);
    t.normalize();
    SVector3 d = crossprod(n, t);
    if(d.norm() < 1.e-12) continue;
    d.normalize();
    double rhs[2] = {dot(n, p), dot(v0, t)};
    double r[2];
    double m[2][2];
    SVector3 x0(0, 0, 0);
    m[0][0] = n.y();
    m[0][1] = n.z();
    m[1][0] = t.y();
    m[1][1] = t.z();
    if(fabs(det2x2(m)) > 1.e-12) {
      sys2x2(m, rhs, r);
      x0 = SVector3(0, r[0], r[1]);
    }
    else {
      m[0][0] = n.x();
      m[0][1] = n.z();
      m[1][0] = t.x();
      m[1][1] = t.z();
      if(fabs(det2x2(m)) > 1.e-12) {
        sys2x2(m, rhs, r);
        x0 = SVector3(r[0], 0, r[1]);
      }
      else {
        m[0][0] = n.x();
        m[0][1] = n.y();
        m[1][0] = t.x();
        m[1][1] = t.y();
        if(sys2x2(m, rhs, r)) {
          x0 = SVector3(r[0], r[1], 0);
        }
        else {
          // printf("mauvaise pioche\n");
          continue;
        }
      }
    }

    const double a = 1.0;
    const double b = -2 * dot(d, p - x0);
    const double c = dot(p - x0, p - x0) - R * R;
    const double delta = b * b - 4 * a * c;
    if(delta >= 0) {
      double sign = (dot(n2, d) > 0) ? 1.0 : -1.0;
      const double ta = (-b + sign * sqrt(delta)) / (2. * a);
      const double tb = (-b - sign * sqrt(delta)) / (2. * a);
      SVector3 s[2] = {x0 + d * ta, x0 + d * tb};
      for(int IT = 0; IT < 2; IT++) {
        double mat[2][2], b[2], uv[2];
        mat[0][0] = dot(t1, t1);
        mat[1][1] = dot(t2, t2);
        mat[0][1] = mat[1][0] = dot(t1, t2);
        b[0] = dot(s[IT] - v0, t1);
        b[1] = dot(s[IT] - v0, t2);
        sys2x2(mat, b, uv);
        // check now if the point is inside the triangle
        if(uv[0] >= -1.e-6 && uv[1] >= -1.e-6 && 1. - uv[0] - uv[1] >= -1.e-6) {
          SVector3 pp =
            v0_2d * (1. - uv[0] - uv[1]) + v1_2d * uv[0] + v2_2d * uv[1];
          uv[0] = pp.x();
          uv[1] = pp.y();
          return GPoint(s[IT].x(), s[IT].y(), s[IT].z(), this, uv);
        }
      }
    }
  }
#endif
  GPoint pp(0);
  pp.setNoSuccess();
  //  Msg::Warning("Could not intersect with circle");
  return pp;
}

#if defined(HAVE_HXT)

/*
static void existingEdges(GFace *gf, std::map<MEdge, GEdge *, Less_Edge> &edges)
{
  std::vector<GEdge *> const &e = gf->edges();
  for(std::vector<GEdge *>::const_iterator it = e.begin(); it != e.end();
      ++it) {
    for(unsigned int i = 0; i < (*it)->lines.size(); i++) {
      MLine *ml = (*it)->lines[i];
      edges.insert(
        std::make_pair(MEdge(ml->getVertex(0), ml->getVertex(1)), *it));
    }
  }
}
*/

void discreteFace::computeSplitEdges(int nbColors, int *colors, std::vector<MEdge> &splitEdges){

  GModel *gm = model();

  // compute all existing edges ...
  std::set<MEdge,Less_Edge> existing_edges;
  for(GModel::eiter it = model()->firstEdge(); it != model()->lastEdge();  it++) {
    for(size_t k = 0; k < (*it)->lines.size(); k++) {
      MEdge e((*it)->lines[k]->getVertex(0), (*it)->lines[k]->getVertex(1));
      existing_edges.insert(e);
    }
  }

  printf("%d existing edges\n",existing_edges.size());
  
  // compute edges with 2 colors that are not existing edges
  std::map<MEdge,  std::pair<int,int>, Less_Edge > allEdges_pairs;  
  for(size_t i = 0; i < triangles.size(); i++) {
    MTriangle *t = triangles[i];
    int c = colors[i];
    for(size_t j = 0; j < 3 ; j++) {
      std::map<MEdge,  std::pair<int,int> , Less_Edge >::iterator it = allEdges_pairs.find(t->getEdge(j));
      if (it == allEdges_pairs.end()){
	allEdges_pairs[t->getEdge(j)] = std::make_pair(c,-1);	
      }
      else{
	it->second.second = c;	
      }	
    }    
  }
  std::map<MEdge,  std::pair<int,int> , Less_Edge >::iterator it = allEdges_pairs.begin();
  for( ; it != allEdges_pairs.end(); ++it)
    if (existing_edges.find(it->first) == existing_edges.end())
      if (it->second.first != it->second.second)
	splitEdges.push_back(MEdge(it->first.getVertex(0),it->first.getVertex(1)));  
}



bool discreteFace::compute_topology_of_partition2(
  int nbColors, int *colors, int *nNodes, int *nodes, double *uv,
  double *nodalCurvatures,
  std::vector<MVertex *> &c2v)
{
  GModel *gm = model();

  // existing edges
  std::set<MEdge,Less_Edge> existing_edges;
  for(GModel::eiter it = model()->firstEdge(); it != model()->lastEdge();  it++) {
    //    printf("GEDGE %d (%d lines)\n",(*it)->tag(),(*it)->lines.size());
    for(size_t k = 0; k < (*it)->lines.size(); k++) {
      MEdge e((*it)->lines[k]->getVertex(0), (*it)->lines[k]->getVertex(1));
      existing_edges.insert(e);
    }
  }
  
  Msg::Debug("computing topology of the %d partitions for surface %d",nbColors,tag());
  
  int TAG = gm->getMaxElementNumber() + 1;

  // Assign parameters for each vertex of each partition
  std::vector<int> cpt(_parametrizations.size());
  std::vector<MTriangle *> &ts = triangles;

  // make a copy of the geometry and of the parametrization (could be smaller)
  std::map<std::pair<MVertex *, int>, SPoint2> params;
  std::map<std::pair<MVertex *, int>, SVector3 > _CMAX;
  std::map<std::pair<MVertex *, int>, SVector3 > _CMIN;
  for(size_t i = 0; i < _parametrizations.size(); i++) {
    cpt[colors[i]] = 0;
    for(int j = nNodes[i]; j < nNodes[i + 1]; j++) {
      MVertex *vert = c2v[nodes[j]];
      double U = uv[2 * j + 0];
      double V = uv[2 * j + 1];
      double *C = &nodalCurvatures[6 * nodes[j]];
      SVector3 CMAX (C[0],C[1],C[2]);
      SVector3 CMIN (C[3],C[4],C[5]);
      std::pair<MVertex *, int> pp = std::make_pair(vert, i);
      params[pp] = SPoint2(U, V);
      _CMAX[pp] = CMAX;
      _CMIN[pp] = CMIN;
    }
  }

  // count how much triangles per partition
#define debug
#ifdef debug
  // save the atlas in pos files for checking - debugging
  char zz[256];
  sprintf(zz,"parametrization_P%d.pos",tag());
  FILE *f = fopen(zz, "w");
  fprintf(f, "View \"\"{\n");
  sprintf(zz,"parametrization_R%d.pos",tag());
  FILE *f2 = fopen(zz, "w");
  fprintf(f2, "View \"\"{\n");
#endif

  for(size_t i = 0; i < ts.size(); i++) {
    cpt[colors[i]]++;
  }
  for(size_t i = 0; i < _parametrizations.size(); i++) {
    _parametrizations[colors[i]].t3d.reserve(cpt[colors[i]]);
    _parametrizations[colors[i]].t2d.reserve(cpt[colors[i]]);
  }

  std::map<MVertex *, std::vector<int>, MVertexLessThanNum > v2c;
  std::set<MEdge,  Less_Edge > allEdges;
  std::map<MEdge,  std::pair<int,int>, Less_Edge > allEdges_pairs;
  
  for(size_t i = 0; i < ts.size(); i++) {
    MTriangle *t = ts[i];
    int c = colors[i];
    for(size_t j = 0; j < 3 ; j++) {
      v2c[t->getVertex(j)].push_back(c);
      allEdges.insert(t->getEdge(j));
      std::map<MEdge,  std::pair<int,int> , Less_Edge >::iterator it = allEdges_pairs.find(t->getEdge(j));
      if (it == allEdges_pairs.end()){
	allEdges_pairs[t->getEdge(j)] = std::make_pair(c,-1);	
      }
      else{
	it->second.second = c;	
      }	
    }    

    MVertex *v0 = new MVertex(t->getVertex(0)->x(), t->getVertex(0)->y(),
                              t->getVertex(0)->z());
    MVertex *v1 = new MVertex(t->getVertex(1)->x(), t->getVertex(1)->y(),
                              t->getVertex(1)->z());
    MVertex *v2 = new MVertex(t->getVertex(2)->x(), t->getVertex(2)->y(),
                              t->getVertex(2)->z());
    SPoint2 p0 = params[std::make_pair(t->getVertex(0), c)];
    SPoint2 p1 = params[std::make_pair(t->getVertex(1), c)];
    SPoint2 p2 = params[std::make_pair(t->getVertex(2), c)];
    MVertex *vv0 = new MVertex(p0.x(), p0.y(), 0.0);
    MVertex *vv1 = new MVertex(p1.x(), p1.y(), 0.0);
    MVertex *vv2 = new MVertex(p2.x(), p2.y(), 0.0);
    MTriangle t3d(v0, v1, v2);
    MTriangle t2d(vv0, vv1, vv2);
    _parametrizations[c].t3d.push_back(t3d);
    _parametrizations[c].t2d.push_back(t2d);
    _parametrizations[c].CURV.push_back(_CMAX[std::make_pair(t->getVertex(0), c)]);
    _parametrizations[c].CURV.push_back(_CMAX[std::make_pair(t->getVertex(1), c)]);
    _parametrizations[c].CURV.push_back(_CMAX[std::make_pair(t->getVertex(2), c)]);
    _parametrizations[c].CURV.push_back(_CMIN[std::make_pair(t->getVertex(0), c)]);
    _parametrizations[c].CURV.push_back(_CMIN[std::make_pair(t->getVertex(1), c)]);
    _parametrizations[c].CURV.push_back(_CMIN[std::make_pair(t->getVertex(2), c)]);
#ifdef debug
    fprintf(f, "ST(%g,%g,%g,%g,%g,%g,%g,%g,%g){%d,%d,%d};\n",
            vv0->x() + 2.2 * c, vv0->y(), vv0->z(), vv1->x() + 2.2 * c,
            vv1->y(), vv1->z(), vv2->x() + 2.2 * c, vv2->y(), vv2->z(), c, c,
            c);
    fprintf(f2, "ST(%g,%g,%g,%g,%g,%g,%g,%g,%g){%d,%d,%d};\n", v0->x(), v0->y(),
            v0->z(), v1->x(), v1->y(), v1->z(), v2->x(), v2->y(), v2->z(), c, c,
            c);
#endif
  }
#ifdef debug
  fprintf(f, "};\n");
  fclose(f);
  fprintf(f2, "};\n");
  fclose(f2);
#endif

  Msg::Debug("Triangles have been colored",nbColors,tag());

  
  // add every model vertex that is necessary
  // sometimes split existing edges
  std::map<MVertex *, std::vector<int> , MVertexLessThanNum>::iterator it = v2c.begin();
  //  int count = 0;
  //  for (;it != v2c.end();++it){
  //    printf("%d %d\n",++count,v2c.size());
  //  }
  //  it = v2c.begin();
  //  count = 0;
  for (;it != v2c.end();++it){
    //    printf("%d %d\n",++count,v2c.size());
    MVertex *v = it->first;
    std::vector<int> & colors = it->second;
    std::sort(colors.begin(),colors.end());
    std::vector<int>::iterator itTmp = std::unique(colors.begin(), colors.end());
    colors.erase(itTmp, colors.end()); 
    //    printf("%d %d %d\n",v->getNum(),colors.size(),ge->dim());
    // vertex was classified on an edge
    GEntity *ge = v->onWhat();
    if (ge->dim() == 1 && colors.size() >= 2){
      Msg::Info("Vertex %d classified on %d %d has %d colors : creating a model vertex there\n",v->getNum(),ge->dim(), ge->tag(),colors.size());
      discreteVertex *dv = new discreteVertex(gm, ++TAG+1, v->x(), v->y(), v->z());
      gm->add(dv);
      v->onWhat()->mesh_vertices.erase( std::remove( v->onWhat()->mesh_vertices.begin(), v->onWhat()->mesh_vertices.end(), v ),
					v->onWhat()->mesh_vertices.end() ); 
      v->setEntity(dv);
      dv->mesh_vertices.push_back(v);
      discreteEdge *de = dynamic_cast<discreteEdge *>(ge);
      if(!de) Msg::Error("Can only split discrete curves at that point ... gmsh will crash");
      Msg::Info("Splitting existing discrete curve %d with %d vertices and %d lines", de->tag(),de->mesh_vertices.size(),de->lines.size());
      de->split(v, dv, TAG);
    }    
  }
  Msg::Info("Existing Model Edges have been split");
  
  // create internal model edges and faces
  std::map<std::pair<int, int>, std::vector<MEdge> > internal_edges;
  std::map<MEdge, std::pair<int, int>, Less_Edge > internal_edges_inv;
  {
    std::set<MEdge,  Less_Edge >::iterator it = allEdges.begin();
    for (; it != allEdges.end(); ++it){
      MVertex *v1 = it->getVertex(0);
      MVertex *v2 = it->getVertex(1);
      MEdge e(v1,v2);
      std::vector<int> &c1 = v2c[v1];
      std::vector<int> &c2 = v2c[v2];
      std::vector<int> c3;
      std::set_intersection(c1.begin(),c1.end(),c2.begin(),c2.end(),std::back_inserter(c3));
      if (c3.size() == 2){
	std::pair<int,int> p = std::make_pair(c3[0],c3[1]);
	if (existing_edges.find (e) == existing_edges.end())
	  internal_edges[p].push_back(e);
	internal_edges_inv[e]=p;
      }    
      else if (c3.size() == 1){ // distinguish internals and boundaries
	std::map<MEdge,  std::pair<int,int> , Less_Edge >::iterator it2 = allEdges_pairs.find(e);
	std::pair<int,int> p = std::make_pair(c3[0],it2->second.second);
	internal_edges_inv[e]=p;
      }    
    }
  }

  {
    std::map<std::pair<int, int>, std::vector<MEdge> >::iterator it =  internal_edges.begin();
    Msg::Info ("%d internal edges have been created",internal_edges.size());
    for (; it != internal_edges.end(); ++it){
      std::vector<std::vector<MVertex *> > vs;
      SortEdgeConsecutive(it->second, vs);
      Msg::Info("Model Edge between colors %d and %d contains %d mesh edges and is separated in %d connected parts",
		it->first.first,it->first.second,it->second.size(), vs.size());
      for (size_t i=0;i<vs.size();i++){
	std::vector<MVertex*> vi = vs[i];
	int countModelVertices = 0;
	for (size_t j=0;j<vi.size();j++)if (vi[j]->onWhat()->dim() == 0)countModelVertices ++;
	// periodic
	if (vi[0]->onWhat()->dim() == 0 && vi[vi.size()-1]->onWhat()->dim() == 0){
	  Msg::Info ("  Part %d of internal edge connects model vertices %d and %d",i,vi[vi.size()-1]->onWhat()->tag(), vi[0]->onWhat()->tag());
	  int counter = 0;
	  while (counter < vi.size()-1){	  
	    discreteEdge *de = new discreteEdge(gm, ++TAG+1, (GVertex*)vi[counter]->onWhat(), NULL);
	    gm->add(de);
	    while(counter < vi.size()-1){
	      de->lines.push_back(new MLine(vi[counter],vi[counter+1]));
	      if (vi[counter+1]->onWhat()->dim() == 0){
		de->setEndVertex((GVertex*)vi[counter+1]->onWhat());
		counter++;
		break;
	      }
	      else vi[counter+1]->setEntity(de);
	      counter++;
	    }
	    Msg::Info ("  creating model edge %d (%d %d) %d lines / %d",  de->tag(),de->getBeginVertex()->tag(),de->getEndVertex()->tag(),
		       de->lines.size(),vi.size()-1);
	  }
	}
	else if (vi[0] == vi[vi.size()-1]){
	  Msg::Info ("  Part %d of internal edge is periodic",i);
	  if (countModelVertices == 0){
	    discreteVertex *dv = new discreteVertex(gm, ++TAG+1, vi[0]->x(), vi[0]->y(), vi[0]->z());
	    gm->add(dv);
	    vi[0]->onWhat()->mesh_vertices.erase( std::remove( vi[0]->onWhat()->mesh_vertices.begin(), vi[0]->onWhat()->mesh_vertices.end(), vi[0] ),
						  vi[0]->onWhat()->mesh_vertices.end() );
	    vi[0]->setEntity(dv);
	    dv->mesh_vertices.push_back(vi[0]);
	    discreteEdge *de = new discreteEdge(gm, ++TAG+1, dv, dv);
	    gm->add(de);
	    for (size_t j=1;j<vi.size();j++)de->lines.push_back(new MLine(vi[j-1],vi[j]));	    
	    for (size_t j=1;j<vi.size()-1;j++)vi[j]->setEntity(de);
	    Msg::Info ("  creating periodic edge %d beween colors %d and %d (new model vertex %d added for closure)",
		       de->tag(),it->first.first,it->first.second,dv->tag());
	  }
	  else Msg::Error("Should dig more ...");
	}       	
      }
    }
  }

  // fill up parametrization boundaries

  for(size_t i = 0; i < _parametrizations.size(); i++) {
    //    printf("PARAMETRIZATION OF PART %d\n\n",i);
    std::set<GEdge *> des;
    std::set<GEdge *> internals;
    for(GModel::eiter it = model()->firstEdge(); it != model()->lastEdge();  it++) {
      //      printf("GEDGE %d (%d lines)\n",(*it)->tag(),(*it)->lines.size());
      for(size_t k = 0; k < (*it)->lines.size(); k++) {
        MEdge e((*it)->lines[k]->getVertex(0), (*it)->lines[k]->getVertex(1));
	std::map<MEdge, std::pair<int, int>, Less_Edge >::iterator it2 =  internal_edges_inv.find(e);	
	//	printf("(%d %d)",it2->second.first,it2->second.second);
	if (it2 != internal_edges_inv.end() && (it2->second.first == i || it2->second.second == i)){
          GEdge *de = *it;
	  if (it2->second.first == it2->second.second){
	    if(internals.find(de) == internals.end() ) {
	      if(de->_compound.size()) {
		if(de->compound_edge)
		  internals.insert((discreteEdge *)de->compound_edge);
	      }
	      else
		internals.insert(de);
	    }
	  }
	  else{
	    if(des.find(de) == des.end() ) {
	      if(de->_compound.size()) {
		if(de->compound_edge)
		  des.insert((discreteEdge *)de->compound_edge);
	      }
	      else
		des.insert(de);
	    }
	  }
	}
      }
      //      printf("\n");
    }
    _parametrizations[i].bnd.insert(_parametrizations[i].bnd.begin(), des.begin(), des.end());
    _parametrizations[i].emb.insert(_parametrizations[i].emb.begin(), internals.begin(), internals.end());
    printf("  boundary of part %d : ",i);
    for (size_t j=0;j<_parametrizations[i].bnd.size();j++)printf("%d (%d %d)",_parametrizations[i].bnd[j]->tag(),_parametrizations[i].bnd[j]->getBeginVertex()->tag(),
    								 _parametrizations[i].bnd[j]->getEndVertex()->tag());
    printf("  internals : ");
    for (size_t j=0;j<_parametrizations[i].emb.size();j++)printf("%d ",_parametrizations[i].emb[j]->tag());
    printf("\n");
  }

  
  //----------------------------------
  
  mesh_vertices.clear();
  triangles.clear();
  gm->setMaxElementNumber(TAG);

  return true;

}


bool discreteFace::compute_topology_of_partition(
  int nbColors, int *colors, int *nNodes, int *nodes, double *uv,
  double *nodalCurvatures,
  std::vector<MVertex *> &c2v)
{
  std::vector<std::vector<MEdge> > boundaries(_parametrizations.size());
  GModel *gm = model();

  int TAG = gm->getMaxElementNumber() + 1;

  // Assign parameters for each vertex of each partition
  std::vector<int> cpt(_parametrizations.size());
  std::vector<MTriangle *> &ts = triangles;
  // make a copy of the geometry and of the parametrization (could be smaller)
  std::map<std::pair<MVertex *, int>, SPoint2> params;
  std::map<std::pair<MVertex *, int>, SVector3 > _CMAX;
  std::map<std::pair<MVertex *, int>, SVector3 > _CMIN;
  for(size_t i = 0; i < _parametrizations.size(); i++) {
    cpt[colors[i]] = 0;
    for(int j = nNodes[i]; j < nNodes[i + 1]; j++) {
      MVertex *vert = c2v[nodes[j]];
      double U = uv[2 * j + 0];
      double V = uv[2 * j + 1];
      double *C = &nodalCurvatures[6 * nodes[j]];
      SVector3 CMAX (C[0],C[1],C[2]);
      SVector3 CMIN (C[3],C[4],C[5]);
      std::pair<MVertex *, int> pp = std::make_pair(vert, i);
      params[pp] = SPoint2(U, V);
      _CMAX[pp] = CMAX;
      _CMIN[pp] = CMIN;
    }
  }

  // count how much triangles per partition
  for(size_t i = 0; i < ts.size(); i++) {
    cpt[colors[i]]++;
  }
  for(size_t i = 0; i < _parametrizations.size(); i++) {
    _parametrizations[colors[i]].t3d.reserve(cpt[colors[i]]);
    _parametrizations[colors[i]].t2d.reserve(cpt[colors[i]]);
  }

  //#define debug
#ifdef debug
  // save the atlas in pos files for checking - debugging
  char zz[256];
  sprintf(zz,"parametrization_P%d.pos",tag());
  FILE *f = fopen(zz, "w");
  fprintf(f, "View \"\"{\n");
  sprintf(zz,"parametrization_R%d.pos",tag());
  FILE *f2 = fopen(zz, "w");
  fprintf(f2, "View \"\"{\n");
#endif
  // created copies of triangles, both in 2D and in 3D
  for(size_t i = 0; i < ts.size(); i++) {
    MTriangle *t = ts[i];
    int c = colors[i];
    MVertex *v0 = new MVertex(t->getVertex(0)->x(), t->getVertex(0)->y(),
                              t->getVertex(0)->z());
    MVertex *v1 = new MVertex(t->getVertex(1)->x(), t->getVertex(1)->y(),
                              t->getVertex(1)->z());
    MVertex *v2 = new MVertex(t->getVertex(2)->x(), t->getVertex(2)->y(),
                              t->getVertex(2)->z());
    SPoint2 p0 = params[std::make_pair(t->getVertex(0), c)];
    SPoint2 p1 = params[std::make_pair(t->getVertex(1), c)];
    SPoint2 p2 = params[std::make_pair(t->getVertex(2), c)];
    MVertex *vv0 = new MVertex(p0.x(), p0.y(), 0.0);
    MVertex *vv1 = new MVertex(p1.x(), p1.y(), 0.0);
    MVertex *vv2 = new MVertex(p2.x(), p2.y(), 0.0);
    MTriangle t3d(v0, v1, v2);
    MTriangle t2d(vv0, vv1, vv2);
    _parametrizations[c].t3d.push_back(t3d);
    _parametrizations[c].t2d.push_back(t2d);
    _parametrizations[c].CURV.push_back(_CMAX[std::make_pair(t->getVertex(0), c)]);
    _parametrizations[c].CURV.push_back(_CMAX[std::make_pair(t->getVertex(1), c)]);
    _parametrizations[c].CURV.push_back(_CMAX[std::make_pair(t->getVertex(2), c)]);
    _parametrizations[c].CURV.push_back(_CMIN[std::make_pair(t->getVertex(0), c)]);
    _parametrizations[c].CURV.push_back(_CMIN[std::make_pair(t->getVertex(1), c)]);
    _parametrizations[c].CURV.push_back(_CMIN[std::make_pair(t->getVertex(2), c)]);

#ifdef debug
    fprintf(f, "ST(%g,%g,%g,%g,%g,%g,%g,%g,%g){%d,%d,%d};\n",
            vv0->x() + 2.2 * c, vv0->y(), vv0->z(), vv1->x() + 2.2 * c,
            vv1->y(), vv1->z(), vv2->x() + 2.2 * c, vv2->y(), vv2->z(), c, c,
            c);
    fprintf(f2, "ST(%g,%g,%g,%g,%g,%g,%g,%g,%g){%d,%d,%d};\n", v0->x(), v0->y(),
            v0->z(), v1->x(), v1->y(), v1->z(), v2->x(), v2->y(), v2->z(), c, c,
            c);
#endif
  }
#ifdef debug
  fprintf(f, "};\n");
  fclose(f);
  fprintf(f2, "};\n");
  fclose(f2);
#endif

  // compute 1D topology

  // edges between two colors
  std::map<MEdge, std::pair<int, int>, Less_Edge> l;
  for(size_t i = 0; i < ts.size(); i++) {
    MTriangle *t = ts[i];
    int c = colors[i];
    for(int j = 0; j < 3; j++) {
      MEdge e = t->getEdge(j);
      std::map<MEdge, std::pair<int, int>, Less_Edge>::iterator it = l.find(e);
      if(it == l.end()) {
        std::pair<int, int> p = std::make_pair(c, (int)-1);
        l[e] = p;
      }
      else {
	if(it->second.first == c) {
	  l.erase(it);
	}
	else
          it->second.second = c;
      }
    }
  }

  // compute model edges that are internal to a partition or on its boundary
  // the form of an internal edge is a vector of MEdge

  std::map<std::pair<int, int>, std::vector<MEdge> > edges;
  {
    std::map<MEdge, std::pair<int, int>, Less_Edge>::iterator it = l.begin();
    for(; it != l.end(); ++it) {
      std::pair<int, int> x = it->second;
      std::pair<int, int> y = std::make_pair(std::min(x.first, x.second),
                                             std::max(x.first, x.second));
      std::map<std::pair<int, int>, std::vector<MEdge> >::iterator itf =
        edges.find(y);
      if(itf == edges.end()) {
        std::vector<MEdge> v;
        v.push_back(it->first);
        edges[y] = v;
      }
      else {
        itf->second.push_back(it->first);
      }
    }
  }

  // each color has MEdges as boundaries
  {
    std::map<std::pair<int, int>, std::vector<MEdge> >::iterator it =
      edges.begin();
    for(; it != edges.end(); ++it) {
      if(it->first.first != -1)
        boundaries[it->first.first].insert(boundaries[it->first.first].begin(),
                                           it->second.begin(),
                                           it->second.end());
      if(it->first.second != -1)
        boundaries[it->first.second].insert(
          boundaries[it->first.second].begin(), it->second.begin(),
          it->second.end());
    }
  }

  // split external edges i.e. edges that were already there but that are split
  // by partitions
  {
    std::map<std::pair<int, int>, std::vector<MEdge> >::iterator it =
      edges.begin();
    for(; it != edges.end(); ++it) {
      std::vector<std::vector<MVertex *> > vs;
      // this is the tricky beast !!!
      SortEdgeConsecutive(it->second, vs);
      for(size_t k = 0; k < vs.size(); k++) {
        std::vector<MVertex *> &v = vs[k];
        MVertex *vs[2] = {v[0], v[v.size() - 1]};
        for(int i = 0; i < 2; i++) {
          if(vs[i]->onWhat()->dim() == 1 && it->first.second != -1 &&
             it->first.first != -1) {
            // vs[i]->onWhat()->mesh_vertices.erase
            //     (std::remove(vs[i]->onWhat()->mesh_vertices.begin(),
            //	   vs[i]->onWhat()->mesh_vertices.end(), vs[i]),
            //	   vs[i]->onWhat()->mesh_vertices.end());
            discreteEdge *de = dynamic_cast<discreteEdge *>(vs[i]->onWhat());
            if(!de) Msg::Error("Can only split discrete curves at that point ... gmsh will crash");
            discreteVertex *gstart = new discreteVertex
              (gm, ++TAG + 1, vs[i]->x(), vs[i]->y(), vs[i]->z());
            gm->add(gstart);
            vs[i]->setEntity(gstart);
            gstart->mesh_vertices.push_back(vs[i]);
            de->split(vs[i], gstart, TAG);
            Msg::Info("Splitting existing discrete curve %d", de->tag());
          }
        }
      }
    }
  }

  // at that point, end vertices of partition lines are either classified on
  // GVertex or on GFace
  {
    std::map<std::pair<int, int>, std::vector<MEdge> >::iterator it =
      edges.begin();
    for(; it != edges.end(); ++it) {
      std::vector<std::vector<MVertex *> > vs;

      SortEdgeConsecutive(it->second, vs);

      for(size_t k = 0; k < vs.size(); k++) {
        std::vector<MVertex *> &v = vs[k];
        MVertex *ends[2] = {v[0], v[v.size() - 1]};
        if(ends[0]->onWhat() == this || ends[1]->onWhat() == this) {
          for(int i = 0; i < 2; i++) {
            if(ends[i]->onWhat() == this) {
              discreteVertex *gstart = new discreteVertex
                (gm, ++TAG, ends[i]->x(), ends[i]->y(), ends[i]->z());
              v_internals.push_back(gstart);
              gm->add(gstart);
              ends[i]->setEntity(gstart);
              gstart->mesh_vertices.push_back(ends[i]);
            }
          }
        }
        if(it->first.first != -1) {
          discreteEdge *de =
            new discreteEdge(gm, ++TAG, (GVertex *)ends[0]->onWhat(),
                             (GVertex *)ends[1]->onWhat());
          e_internals.push_back(de);
          Msg::Info("Creation of one internal discrete curve %d (%d %d) in "
                    "discrete surface %d", de->tag(), ends[0]->onWhat()->tag(),
                    ends[1]->onWhat()->tag(), tag());
          gm->add(de);
          for(size_t i = 1; i < v.size() - 1; i++) {
            v[i]->setEntity(de);
            de->mesh_vertices.push_back(v[i]);
          }
          for(size_t i = 1; i < v.size(); i++)
            de->lines.push_back(new MLine(v[i - 1], v[i]));
        }
      }
    }
  }
  mesh_vertices.clear();
  triangles.clear();
  gm->setMaxElementNumber(TAG);

  for(size_t i = 0; i < _parametrizations.size(); i++) {
    Less_Edge le;
    std::sort(boundaries[i].begin(), boundaries[i].end(), le);
    std::set<GEdge *> des;
    for(GModel::eiter it = model()->firstEdge(); it != model()->lastEdge();
        it++) {
      //      printf("checking if edge %d is in the boundary of %d\n",(*it)->tag(),tag());
      for(size_t k = 0; k < (*it)->lines.size(); k++) {
        MEdge e((*it)->lines[k]->getVertex(0), (*it)->lines[k]->getVertex(1));
        if(std::binary_search(boundaries[i].begin(), boundaries[i].end(), e,
                              le)) {
          GEdge *de = *it;
          if(des.find(de) == des.end()) {
            if(de->_compound.size()) {
              if(de->compound_edge)
                des.insert((discreteEdge *)de->compound_edge);
            }
            else {
              des.insert(de);
            }
          }
        }
      }
    }
    _parametrizations[i].bnd.insert(_parametrizations[i].bnd.begin(),
                                    des.begin(), des.end());
  }
  
  return true;
}

HXTStatus discreteFace::computsSplitEdgesForPartitionIntoGenusOneSurfaces(std::vector<MEdge> &splitEdges)
{
  int n = 1;
  HXT_CHECK(hxtInitializeLinearSystems(&n, NULL));

  HXTMesh *m;
  std::map<MVertex *, int> v2c;
  std::vector<MVertex *> c2v;
  gmsh2hxt(this, &m, v2c, c2v);

  HXTParametrization *parametrization;
  int *colors, *nNodes, *nodes, nc;
  double *uv;
  HXT_CHECK(hxtParametrizationCreate(m, 0, &parametrization));
  HXT_CHECK(hxtParametrizationCompute(parametrization, &colors, &nNodes, &nodes,
                                      &uv, &nc, &m));

  computeSplitEdges(nc, colors, splitEdges);
  HXT_CHECK(hxtParametrizationDelete(&parametrization));
  HXT_CHECK(hxtMeshDelete(&m));
  return HXT_STATUS_OK;

}


HXTStatus discreteFace::reparametrize_through_hxt()
{
  int n = 1;
  HXT_CHECK(hxtInitializeLinearSystems(&n, NULL));

  HXTMesh *m;
  std::map<MVertex *, int> v2c;
  std::vector<MVertex *> c2v;
  gmsh2hxt(this, &m, v2c, c2v);

  HXTParametrization *parametrization;
  int *colors, *nNodes, *nodes, nc;
  double *uv;
  HXT_CHECK(hxtParametrizationCreate(m, 0, &parametrization));
  HXT_CHECK(hxtParametrizationCompute(parametrization, &colors, &nNodes, &nodes,
                                      &uv, &nc, &m));

  // compute curvatures
  HXTEdges *edges;
  double *nodalCurvatures;
  double *crossField;
  HXT_CHECK(hxtEdgesCreate(m, &edges));
  HXT_CHECK(
    hxtCurvatureRusinkiewicz(m, &nodalCurvatures, &crossField, edges, false));
  HXT_CHECK(hxtEdgesDelete(&edges));
  _parametrizations.resize(nc);
  if(!compute_topology_of_partition2(nc, colors, nNodes, nodes, uv, nodalCurvatures,
				    c2v))
    Msg::Warning("Impossible to compute the topology of the %d partitions", nc);

  HXT_CHECK(hxtFree(&nodalCurvatures));
  HXT_CHECK(hxtFree(&crossField));

  Msg::Info("Surface %d split in %d parts", tag(), _parametrizations.size());

  //  return HXT_STATUS_OK;
  //  std::map<MEdge, GEdge *, Less_Edge> cad_edges;
  //  existingEdges(this, cad_edges);

  for(size_t i = 0; i < _parametrizations.size(); i++) {
    std::vector<MElement *> temp;
    for(size_t j = 0; j < _parametrizations[i].t2d.size(); j++) {
      temp.push_back(&_parametrizations[i].t2d[j]);
      double MIN[3] = {_parametrizations[i].t3d[j].getVertex(0)->x(),
                       _parametrizations[i].t3d[j].getVertex(0)->y(),
                       _parametrizations[i].t3d[j].getVertex(0)->z()};
      double MAX[3] = {_parametrizations[i].t3d[j].getVertex(0)->x(),
                       _parametrizations[i].t3d[j].getVertex(0)->y(),
                       _parametrizations[i].t3d[j].getVertex(0)->z()};
      for(int k = 1; k < 3; k++) {
        MAX[0] =
          std::max(MAX[0], _parametrizations[i].t3d[j].getVertex(k)->x());
        MIN[0] =
          std::min(MIN[0], _parametrizations[i].t3d[j].getVertex(k)->x());
        MAX[1] =
          std::max(MAX[1], _parametrizations[i].t3d[j].getVertex(k)->y());
        MIN[1] =
          std::min(MIN[1], _parametrizations[i].t3d[j].getVertex(k)->y());
        MAX[2] =
          std::max(MAX[2], _parametrizations[i].t3d[j].getVertex(k)->z());
        MIN[2] =
          std::min(MIN[2], _parametrizations[i].t3d[j].getVertex(k)->z());
      }
      std::pair<MTriangle *, MTriangle *> *tt =
        new std::pair<MTriangle *, MTriangle *>(&_parametrizations[i].t3d[j],
                                                &_parametrizations[i].t2d[j]);
      _parametrizations[i].rtree3d.Insert(MIN, MAX, tt);
    }
    _parametrizations[i].oct = new MElementOctree(temp);
  }

  for(size_t i = 0; i < _parametrizations.size(); i++) {
    for(size_t j = 0; j < _parametrizations[i].emb.size(); j++) {
      model()->add(_parametrizations[i].emb[j]);
    }
  }

  HXT_CHECK(hxtParametrizationDelete(&parametrization));
  HXT_CHECK(hxtMeshDelete(&m));
  return HXT_STATUS_OK;
}

hxt_reparam_surf::~hxt_reparam_surf()
{
  if(oct) delete oct;
}

#endif
