// $Id: MinMax.cpp,v 1.21 2006-01-06 00:34:24 geuzaine Exp $
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
#include "Vertex.h"
#include "Context.h"

extern Context_T CTX;

static void minmax(void *a, void *b)
{
  Vertex *v = *(Vertex **) a;
  CTX.min[0] = (CTX.min[0] < v->Pos.X) ? CTX.min[0] : v->Pos.X;
  CTX.max[0] = (CTX.max[0] > v->Pos.X) ? CTX.max[0] : v->Pos.X;
  CTX.min[1] = (CTX.min[1] < v->Pos.Y) ? CTX.min[1] : v->Pos.Y;
  CTX.max[1] = (CTX.max[1] > v->Pos.Y) ? CTX.max[1] : v->Pos.Y;
  CTX.min[2] = (CTX.min[2] < v->Pos.Z) ? CTX.min[2] : v->Pos.Z;
  CTX.max[2] = (CTX.max[2] > v->Pos.Z) ? CTX.max[2] : v->Pos.Z;

  CTX.cg[0] += v->Pos.X;
  CTX.cg[1] += v->Pos.Y;
  CTX.cg[2] += v->Pos.Z;
}

void CalculateMinMax(Tree_T * t, double *bbox)
{
  CTX.cg[0] = 0.0;
  CTX.cg[1] = 0.0;
  CTX.cg[2] = 0.0;

  if(!Tree_Nbr(t)) {
    if(!bbox || (bbox[0] > bbox[1])) {  //the bbox is wrong
      CTX.min[0] = CTX.min[1] = CTX.min[2] = -1.;
      CTX.max[0] = CTX.max[1] = CTX.max[2] = 1.;
      CTX.range[0] = CTX.range[1] = CTX.range[2] = 0.;
      CTX.lc = 1.;
      return;
    }
    else {
      CTX.min[0] = bbox[0];
      CTX.max[0] = bbox[1];
      CTX.min[1] = bbox[2];
      CTX.max[1] = bbox[3];
      CTX.min[2] = bbox[4];
      CTX.max[2] = bbox[5];

      CTX.cg[0] = (CTX.min[0] + CTX.max[0])/2.;
      CTX.cg[1] = (CTX.min[1] + CTX.max[1])/2.;
      CTX.cg[2] = (CTX.min[2] + CTX.max[2])/2.;
    }
  }
  else {
    Vertex *v;
    Tree_Right(t, &v);
    CTX.min[0] = CTX.max[0] = v->Pos.X;
    CTX.min[1] = CTX.max[1] = v->Pos.Y;
    CTX.min[2] = CTX.max[2] = v->Pos.Z;
    Tree_Action(t, minmax);
    int nb = Tree_Nbr(t);
    CTX.cg[0] /= nb;
    CTX.cg[1] /= nb;
    CTX.cg[2] /= nb;
  }

  CTX.range[0] = CTX.max[0] - CTX.min[0];
  CTX.range[1] = CTX.max[1] - CTX.min[1];
  CTX.range[2] = CTX.max[2] - CTX.min[2];

  if(CTX.range[0] == 0. && CTX.range[1] == 0. && CTX.range[2] == 0.) {
    CTX.min[0] -= 1.;
    CTX.min[1] -= 1.;
    CTX.max[0] += 1.;
    CTX.max[1] += 1.;
    CTX.lc = 1.;
  }
  else if(CTX.range[0] == 0. && CTX.range[1] == 0.) {
    CTX.lc = CTX.range[2];
    CTX.min[0] -= CTX.lc;
    CTX.min[1] -= CTX.lc;
    CTX.max[0] += CTX.lc;
    CTX.max[1] += CTX.lc;
  }
  else if(CTX.range[0] == 0. && CTX.range[2] == 0.) {
    CTX.lc = CTX.range[1];
    CTX.min[0] -= CTX.lc;
    CTX.max[0] += CTX.lc;
  }
  else if(CTX.range[1] == 0. && CTX.range[2] == 0.) {
    CTX.lc = CTX.range[0];
    CTX.min[1] -= CTX.lc;
    CTX.max[1] += CTX.lc;
  }
  else if(CTX.range[0] == 0.) {
    CTX.lc = sqrt(DSQR(CTX.range[1]) + DSQR(CTX.range[2]));
    CTX.min[0] -= CTX.lc;
    CTX.max[0] += CTX.lc;
  }
  else if(CTX.range[1] == 0.) {
    CTX.lc = sqrt(DSQR(CTX.range[0]) + DSQR(CTX.range[2]));
    CTX.min[1] -= CTX.lc;
    CTX.max[1] += CTX.lc;
  }
  else if(CTX.range[2] == 0.) {
    CTX.lc = sqrt(DSQR(CTX.range[0]) + DSQR(CTX.range[1]));
  }
  else {
    CTX.lc = sqrt(DSQR(CTX.range[0]) + DSQR(CTX.range[1]) + DSQR(CTX.range[2]));
  }

}
