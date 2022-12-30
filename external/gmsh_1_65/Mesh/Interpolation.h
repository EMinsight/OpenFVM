#ifndef _INTERPOLATION_H_
#define _INTERPOLATION_H_

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

#include "Vertex.h"
#include "Mesh.h"

Vertex InterpolateCurve (Curve * Curve, double u, int derivee);

Vertex InterpolateSurface (Surface * s, double u, double v, 
                           int derivee, int u_v);

Vertex TransfiniteQua (Vertex c1, Vertex c2, Vertex c3, Vertex c4,
                       Vertex s1, Vertex s2, Vertex s3, Vertex s4,
                       double u, double v);

Vertex TransfiniteTri (Vertex c1, Vertex c2, Vertex c3,
                       Vertex s1, Vertex s2, Vertex s3,
                       double u, double v);

Vertex TransfiniteHex 
  (Vertex f1, Vertex f2, Vertex f3, Vertex f4, Vertex f5, Vertex f6,
   Vertex c1, Vertex c2, Vertex c3, Vertex c4, Vertex c5, Vertex c6,
   Vertex c7, Vertex c8, Vertex c9, Vertex c10, Vertex c11, Vertex c12,
   Vertex s1, Vertex s2, Vertex s3, Vertex s4,
   Vertex s5, Vertex s6, Vertex s7, Vertex s8,
   double u, double v, double w);

void TransfiniteSph (Vertex S, Vertex center, Vertex * T);

void Normal2Surface (Surface * s, double u, double v, double n[3]);

#endif


