#ifndef HXT_POINT_GEN_OPTIM_H
#define HXT_POINT_GEN_OPTIM_H

#include "hxt_mesh.h"

HXTStatus hxtSurfaceMeshExportTriangleQuality(HXTMesh *nmesh, const double *sizemap);
HXTStatus hxtSurfaceMeshExportAlignedEdges(HXTMesh *mesh, 
                                           HXTMesh *nmesh, 
                                           uint64_t *p2t,
                                           const double *directions, 
                                           const double *sizemap);

HXTStatus hxtPointGenSmoothing(HXTMesh *mesh);

#endif