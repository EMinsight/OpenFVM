#ifndef _VISIBILITY_H_
#define _VISIBILITY_H_

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

#include "List.h"
#include "Mesh.h"

typedef struct{
  int n;
  char *s;
} NxS;

class Entity {
public :
  int type;
  union {
    Vertex *vertex;
    Curve *curve;
    Surface *surface;
    Volume *volume;
    PhysicalGroup *physical;
    MeshPartition *partition;
  } data;
  char *str;

  int   Num();
  char *Type();
  char *Name();
  int   Visible();
  void  Visible(int mode);
  void  RecurVisible();
  char *BrowserLine();
};

void    SetVisibilitySort(int sort);
List_T* GetVisibilityList(int type);
void    ClearVisibilityList(int type);
void    InitVisibilityThroughPhysical();
void    SetVisibilityByNumber(int num, int type, int mode);
void    SetVisibilityByNumber(char *str, int type, int mode);

#endif
