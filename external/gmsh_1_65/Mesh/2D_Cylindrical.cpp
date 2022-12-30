// $Id: 2D_Cylindrical.cpp,v 1.20 2006-01-06 00:34:25 geuzaine Exp $
//
// Copyright (C) 1997-2006 C. Geuzaine, J.-F. Remacle
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
// 
// Please report all bugs and problems to <gmsh@geuz.org>.

#include "Gmsh.h"
#include "Numeric.h"
#include "Mesh.h"
#include "Context.h"

extern Mesh *THEM;
extern Context_T CTX;

static Surface *SURF;
static double THETAMIN, THETAMAX;

void ChangePi(void *a, void *dum)
{
  Vertex *v;
  v = *(Vertex **) a;

  if((v->Pos.X / SURF->Cyl.radius1) >= THETAMIN + .99999 * Pi) {
    Msg(INFO, "%g -> ", v->Pos.X / SURF->Cyl.radius1);
    v->Pos.X -= (2. * Pi) * SURF->Cyl.radius1;
    Msg(INFO, "%g -> ", v->Pos.X / SURF->Cyl.radius1);
  }
}

void THETAMINMAX(void *a, void *dum)
{
  Vertex *v;
  double ZRepere, S, C, y[3], teta;
  double p[3], z[3], x[3], o[3];

  v = *(Vertex **) a;

  p[0] = v->Pos.X - SURF->Cyl.center[0];
  p[1] = v->Pos.Y - SURF->Cyl.center[1];
  p[2] = v->Pos.Z - SURF->Cyl.center[2];
  z[0] = SURF->Cyl.zaxis[0];
  z[1] = SURF->Cyl.zaxis[1];
  z[2] = SURF->Cyl.zaxis[2];
  norme(z);
  prosca(p, z, &ZRepere);

  //ZRepere = fabs(ZRepere);

  o[0] = p[0] - ZRepere * z[0];
  o[1] = p[1] - ZRepere * z[1];
  o[2] = p[2] - ZRepere * z[2];
  x[0] = SURF->Cyl.xaxis[0];
  x[1] = SURF->Cyl.xaxis[1];
  x[2] = SURF->Cyl.xaxis[2];

  norme(o);
  norme(x);
  prodve(z, x, y);
  norme(y);
  prosca(o, x, &C);
  prosca(o, y, &S);
  teta = atan2(S, C);
  THETAMIN = DMIN(teta, THETAMIN);
  THETAMAX = DMAX(teta, THETAMAX);
}

// Cylindrical surfaces

void XYZtoTZ(void *a, void *dum)
{
  Vertex *v;
  double ZRepere, S, C, y[3], teta;
  double p[3], z[3], x[3], o[3];

  v = *(Vertex **) a;

  p[0] = v->Pos.X - SURF->Cyl.center[0];
  p[1] = v->Pos.Y - SURF->Cyl.center[1];
  p[2] = v->Pos.Z - SURF->Cyl.center[2];
  z[0] = SURF->Cyl.zaxis[0];
  z[1] = SURF->Cyl.zaxis[1];
  z[2] = SURF->Cyl.zaxis[2];
  norme(z);
  prosca(p, z, &ZRepere);

  //ZRepere = fabs(ZRepere);

  o[0] = p[0] - ZRepere * z[0];
  o[1] = p[1] - ZRepere * z[1];
  o[2] = p[2] - ZRepere * z[2];
  x[0] = SURF->Cyl.xaxis[0];
  x[1] = SURF->Cyl.xaxis[1];
  x[2] = SURF->Cyl.xaxis[2];

  norme(o);
  norme(x);
  prodve(z, x, y);
  norme(y);
  prosca(o, x, &C);
  prosca(o, y, &S);
  teta = atan2(S, C);
  Msg(DEBUG, "pt %d %g %g", v->Num, ZRepere, teta);

  v->Pos.X = teta * SURF->Cyl.radius1;
  v->Pos.Y = ZRepere;
  v->Pos.Z = 0.0;
}

void TZtoXYZ(void *a, void *dum)
{
  Vertex *v;
  double d[3], x[3], prv[3];
  double XX, YY, ZZ;

  v = *(Vertex **) a;
  d[0] = SURF->Cyl.zaxis[0];
  d[1] = SURF->Cyl.zaxis[1];
  d[2] = SURF->Cyl.zaxis[2];
  norme(d);
  x[0] = SURF->Cyl.xaxis[0];
  x[1] = SURF->Cyl.xaxis[1];
  x[2] = SURF->Cyl.xaxis[2];
  norme(x);
  prodve(d, x, prv);
  norme(prv);

  XX = SURF->Cyl.center[0] + v->Pos.Y * d[0] +
    SURF->Cyl.radius1 * cos(v->Pos.X / SURF->Cyl.radius1) * x[0] +
    SURF->Cyl.radius1 * (sin(v->Pos.X / SURF->Cyl.radius1)) * prv[0];
  YY = SURF->Cyl.center[1] + v->Pos.Y * d[1] +
    SURF->Cyl.radius1 * cos(v->Pos.X / SURF->Cyl.radius1) * x[1] +
    SURF->Cyl.radius1 * (sin(v->Pos.X / SURF->Cyl.radius1)) * prv[1];
  ZZ = SURF->Cyl.center[2] + v->Pos.Y * d[2] +
    SURF->Cyl.radius1 * cos(v->Pos.X / SURF->Cyl.radius1) * x[2] +
    SURF->Cyl.radius1 * (sin(v->Pos.X / SURF->Cyl.radius1)) * prv[2];

  v->Pos.X = XX;
  v->Pos.Y = YY;
  v->Pos.Z = ZZ;
}

// Conical surfaces

void XYZtoCone(void *a, void *dum)
{
  Vertex *v;
  double ZRepere, S, C, y[3], teta;
  double p[3], z[3], x[3], o[3];
  double inclinaison, ract;

  v = *(Vertex **) a;

  p[0] = v->Pos.X - SURF->Cyl.center[0];
  p[1] = v->Pos.Y - SURF->Cyl.center[1];
  p[2] = v->Pos.Z - SURF->Cyl.center[2];
  z[0] = SURF->Cyl.zaxis[0];
  z[1] = SURF->Cyl.zaxis[1];
  z[2] = SURF->Cyl.zaxis[2];
  norme(z);
  prosca(p, z, &ZRepere);

  //ZRepere = fabs(ZRepere);

  o[0] = p[0] - ZRepere * z[0];
  o[1] = p[1] - ZRepere * z[1];
  o[2] = p[2] - ZRepere * z[2];
  x[0] = SURF->Cyl.xaxis[0];
  x[1] = SURF->Cyl.xaxis[1];
  x[2] = SURF->Cyl.xaxis[2];

  norme(o);
  norme(x);
  prodve(z, x, y);
  norme(y);
  prosca(o, x, &C);
  prosca(o, y, &S);
  teta = atan2(S, C);
  if(teta >= THETAMIN + .99999 * Pi)
    teta -= (2. * Pi);

  inclinaison = Pi * SURF->Cyl.radius2 / 180.;

  ract = SURF->Cyl.radius1 - ZRepere * tan(inclinaison);

  v->Pos.X = ract * cos(teta);
  v->Pos.Y = ract * sin(teta);
  v->Pos.Z = 0.0;
  Msg(DEBUG, "%g %g", ZRepere, v->Pos.X);
}

void ConetoXYZ(void *a, void *dum)
{
  Vertex *v;
  double d[3], z, x[3], prv[3];
  double XX, YY, ZZ;
  double inclinaison, teta, radiusact;

  v = *(Vertex **) a;
  d[0] = SURF->Cyl.zaxis[0];
  d[1] = SURF->Cyl.zaxis[1];
  d[2] = SURF->Cyl.zaxis[2];
  norme(d);
  x[0] = SURF->Cyl.xaxis[0];
  x[1] = SURF->Cyl.xaxis[1];
  x[2] = SURF->Cyl.xaxis[2];
  norme(x);
  prodve(d, x, prv);
  norme(prv);
  inclinaison = Pi * SURF->Cyl.radius2 / 180.;
  z = (SURF->Cyl.radius1 - myhypot(v->Pos.X, v->Pos.Y)) / tan(inclinaison);
  radiusact = (SURF->Cyl.radius1 + z * tan(inclinaison));
  teta = atan2(v->Pos.Y, v->Pos.X);

  XX = SURF->Cyl.center[0] + z * d[0] +
    radiusact * cos(teta) * x[0] + radiusact * (sin(teta)) * prv[0];
  YY = SURF->Cyl.center[1] + z * d[1] +
    radiusact * cos(teta) * x[1] + radiusact * (sin(teta)) * prv[1];
  ZZ = SURF->Cyl.center[2] + z * d[2] +
    radiusact * cos(teta) * x[2] + radiusact * (sin(teta)) * prv[2];

  v->Pos.X = XX;
  v->Pos.Y = YY;
  v->Pos.Z = ZZ;

}

int MeshCylindricalSurface(Surface * s)
{
  return 0;

#if 0

  int i, j, ori;
  Curve *pC;
  Vertex *v;
  Tree_T *tnxe;
  char text[256];
  extern double MAXIMUM_LC_FOR_SURFACE;

  if(s->Typ != MSH_SURF_CYLNDR && s->Typ != MSH_SURF_CONE
     && s->Typ != MSH_SURF_TORUS)
    return 0;
  if(s->Typ == MSH_SURF_TORUS)
    return 1;

  Msg(DEBUG, "z %d : %12.5E %12.5E %12.5E", s->Num, s->Cyl.zaxis[0],
      s->Cyl.zaxis[1], s->Cyl.zaxis[2]);
  Msg(DEBUG, "x %d : %12.5E %12.5E %12.5E", s->Num, s->Cyl.xaxis[0],
      s->Cyl.xaxis[1], s->Cyl.xaxis[2]);

  SURF = s;

  for(i = 0; i < List_Nbr(s->Generatrices); i++) {
    List_Read(s->Generatrices, i, &pC);
    for(j = 0; j < List_Nbr(pC->Vertices); j++) {
      List_Read(pC->Vertices, j, &v);
      Tree_Insert(s->Vertices, List_Pointer(pC->Vertices, j));
    }
  }
  THETAMIN = 2. * Pi;
  THETAMAX = -2. * Pi;
  Tree_Action(s->Vertices, THETAMINMAX);
  Tree_Action(s->Vertices, Freeze_Vertex);

  if(s->Typ == MSH_SURF_CYLNDR)
    Tree_Action(s->Vertices, XYZtoTZ);
  else if(s->Typ == MSH_SURF_CONE)
    Tree_Action(s->Vertices, XYZtoCone);

  Msg(DEBUG, "%12.5E %12.5E", THETAMAX, THETAMIN);

  if((s->Typ == MSH_SURF_CYLNDR) && (THETAMAX - THETAMIN > Pi * 1.01))
    Tree_Action(s->Vertices, ChangePi);

  ori = Calcule_Contours(s);
  MAXIMUM_LC_FOR_SURFACE = SURF->Cyl.radius1 / 3.;

  if(CTX.mesh.algo2d == DELAUNAY_ISO)
    Maillage_Automatique_VieuxCode(s, THEM, ori);
  else if(CTX.mesh.algo2d == DELAUNAY_ANISO)
    AlgorithmeMaillage2DAnisotropeModeJF(s);
  else
    Mesh_Triangle(s);

  for(i = 0; i < CTX.mesh.nb_smoothing; i++) {
    tnxe = Tree_Create(sizeof(NXE), compareNXE);
    create_NXE(s->Vertices, s->Simplexes, tnxe);
    Tree_Action(tnxe, ActionLiss);
    Tree_Delete(tnxe);
  }

  if(s->Typ == MSH_SURF_CYLNDR)
    Tree_Action(s->Vertices, TZtoXYZ);
  else if(s->Typ == MSH_SURF_CONE)
    Tree_Action(s->Vertices, ConetoXYZ);
  Tree_Action(s->Vertices, deFreeze_Vertex);

  return 1;

#endif
}
