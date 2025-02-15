/***************************************************************************
 *   Copyright (C) 2004-2008 by OpenFVM team                               *
 *   http://sourceforge.net/projects/openfvm/                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

enum
{
  BEAM,
  TRIANGLE,
  QUADRANGLE,
  TETRAHEDRON,
  HEXAHEDRON,
  PRISM
} msh_type;

typedef struct
{

  double x, y, z;

} msh_vector;

typedef struct
{

  int index;

  int type;

  int nbnodes;
  int *node;

  int element;

  msh_vector cface;
  int pair;

  msh_vector n;

  msh_vector A;
  double Aj;

  msh_vector d;
  double dj;
  double kj;
  msh_vector rpl;
  msh_vector rnl;

  int physreg;
  int elemreg;
  int partition;

  int ghost;

  int bc;

} msh_face;

typedef struct
{

  int index;

  int type;

  msh_vector normal;
  msh_vector celement;

  int nbnodes;
  int *node;

  int nbfaces;
  int *face;

  double dp;
  double Lp;
  double Ap;
  double Vp;

  int physreg;
  int elemreg;
  int partition;

  int process;

  int bc;

  double *b;
  double *c;
  double *d;

} msh_element;

typedef struct
{

  msh_vector normal;
  double D;

} msh_plane;

// Mesh

int nbnodes;
int nbfaces;
int nbelements;
int nbpatches;

int nbtris;
int nbquads;
int nbtetras;
int nbhexas;
int nbprisms;

msh_vector *nodes;
msh_face *faces;
msh_face *patches;
msh_element *elements;

// Ghosts
int nbghosts;
int *ghosts;

int nod_correlation_malloced;
int ele_correlation_malloced;

int *nod_correlation;
int *ele_correlation;

int MshImportMSH (char *file);
int MshExportMSH (char *file);

int MshExportDecomposedMSH (char *file, int region, int nbregions);

void MshFreeMemory ();
