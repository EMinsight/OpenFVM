#ifndef _SMOOTH_NORMALS_H_
#define _SMOOTH_NORMALS_H_

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

#include <set>
#include <vector>
#include "Numeric.h"

using namespace std;

struct nnb
{
  float nx, ny, nz;
  int nb;
};

struct xyzn
{
  double x, y, z;
  vector<nnb> n;
  static double eps;
  xyzn(double xx, double yy, double zz) : x(xx), y(yy), z(zz){}
  ~xyzn(){}
  float angle(int i, float n0, float n1, float n2);
  void update(float n0, float n1, float n2, double tol);
};

struct lessthanxyzn
{
  bool operator () (const xyzn & p2, const xyzn & p1)const
  {
    if(p1.x - p2.x > xyzn::eps)
      return true;
    if(p1.x - p2.x < -xyzn::eps)
      return false;
    if(p1.y - p2.y > xyzn::eps)
      return true;
    if(p1.y - p2.y < -xyzn::eps)
      return false;
    if(p1.z - p2.z > xyzn::eps)
      return true;
    return false;
  }
};

typedef set < xyzn, lessthanxyzn > xyzn_cont;
typedef xyzn_cont::const_iterator xyzn_iter;

class smooth_normals{
 private:
  double tol;
  xyzn_cont c;  
 public:
  smooth_normals(double angle) : tol(angle) {}
  void add(double x, double y, double z, double nx, double ny, double nz);
  bool get(double x, double y, double z, double &nx, double &ny, double &nz);
};

#endif
