// $Id: ExtrudeParams.cpp,v 1.18 2006-01-06 00:34:24 geuzaine Exp $
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
#include "Geo.h"
#include "CAD.h"
#include "ExtrudeParams.h"

void Projette(double p[3], double mat[3][3])
{
  double X, Y, Z;

  X = p[0] * mat[0][0] + p[1] * mat[0][1] + p[2] * mat[0][2];
  Y = p[0] * mat[1][0] + p[1] * mat[1][1] + p[2] * mat[1][2];
  Z = p[0] * mat[2][0] + p[1] * mat[2][1] + p[2] * mat[2][2];
  p[0] = X;
  p[1] = Y;
  p[2] = Z;
}

ExtrudeParams::ExtrudeParams(int ModeEx)
{
  _useZonLayer = true;
  geo.Mode = ModeEx;
  geo.Source = -1;
  mesh.ExtrudeMesh = false;
  mesh.Recombine = false;
}


void ExtrudeParams::fill(int type,
                         double T0, double T1, double T2,
                         double A0, double A1, double A2,
                         double X0, double X1, double X2, double angle)
{
  geo.trans[0] = T0;
  geo.trans[1] = T1;
  geo.trans[2] = T2;
  geo.axe[0] = A0;
  geo.axe[1] = A1;
  geo.axe[2] = A2;
  geo.pt[0] = X0;
  geo.pt[1] = X1;
  geo.pt[2] = X2;
  geo.angle = angle;
  geo.Type = type;
}

void ExtrudeParams::Extrude(int iLayer, int iElemLayer,
                            double &x, double &y, double &z)
{
  double t = u(iLayer, iElemLayer);
  Extrude(t, x, y, z);
}

void ExtrudeParams::Rotate(double matr[3][3])
{
  Projette(geo.trans, matr);
  Projette(geo.axe, matr);
  Projette(geo.pt, matr);
  geo.angle = -geo.angle;
}

void ExtrudeParams::Extrude(double t, double &x, double &y, double &z)
{
  double dx, dy, dz, angle;

  switch (geo.Type) {
  case TRANSLATE:
    dx = geo.trans[0] * t;
    dy = geo.trans[1] * t;
    dz = geo.trans[2] * t;
    x += dx;
    y += dy;
    z += dz;
    break;
  case ROTATE:
    angle = geo.angle;
    geo.angle = geo.angle * t;
    ProtudeXYZ(x, y, z, this);
    geo.angle = angle;
    break;
  case TRANSLATE_ROTATE:
    angle = geo.angle;
    geo.angle = geo.angle * t;
    ProtudeXYZ(x, y, z, this);
    geo.angle = angle;
    dx = geo.trans[0] * t;
    dy = geo.trans[1] * t;
    dz = geo.trans[2] * t;
    x += dx;
    y += dy;
    z += dz;
    break;
  default:
    Msg(GERROR, "Unknown extrusion type");
    break;
  }
}

double ExtrudeParams::u(int iLayer, int iElemLayer)
{
  double t0, t1;

  if(!iLayer) {
    t0 = 0.0;
    t1 = mesh.hLayer[0];
  }
  else {
    t0 = mesh.hLayer[iLayer - 1];
    t1 = mesh.hLayer[iLayer];
  }
  double t = (double)iElemLayer / (double)mesh.NbElmLayer[iLayer];
  return t0 + t * (t1 - t0);
}
