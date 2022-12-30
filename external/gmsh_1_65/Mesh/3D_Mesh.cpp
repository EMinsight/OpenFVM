// $Id: 3D_Mesh.cpp,v 1.73 2006-01-31 00:22:33 geuzaine Exp $
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

// Isotropic Delaunay 3D:
// while the tree of bad quality tets is not empty {
//   take the worst tet
//   create a new point
//   remove the test whose circumscribed sphere contains the point
//   reconstruct the convex volume
// } 

#include <vector>
#include <algorithm>
#include <time.h>
#include "Gmsh.h"
#include "Numeric.h"
#include "Geo.h"
#include "Mesh.h"
#include "3D_Mesh.h"
#include "Create.h"
#include "Context.h"

extern Mesh *THEM;
extern Context_T CTX;
extern int FACE_DIMENSION;

static Tree_T *Tsd, *Sim_Sur_Le_Bord, *POINTS_TREE;
static List_T *Simplexes_Destroyed, *Simplexes_New, *Suppress;
static List_T *POINTS_LIST;
static Simplex *THES;
static Vertex *THEV;
static Tree_T *SimXFac;
static double volume, LC3D;
static int ZONEELIMINEE, Methode = 0;

Simplex MyNewBoundary;
int Alerte_Point_Scabreux;

inline void cgsmpl(Simplex * s, double &x, double &y, double &z)
{
  // this inlining crashes with gcc -O2...

  x = 0.25 * (s->V[0]->Pos.X +
              s->V[1]->Pos.X + s->V[2]->Pos.X + s->V[3]->Pos.X);
  y = 0.25 * (s->V[0]->Pos.Y +
              s->V[1]->Pos.Y + s->V[2]->Pos.Y + s->V[3]->Pos.Y);
  z = 0.25 * (s->V[0]->Pos.Z +
              s->V[1]->Pos.Z + s->V[2]->Pos.Z + s->V[3]->Pos.Z);
}

struct closestSimplex {
  double X, Y, Z;
  closestSimplex(double x, double y, double z) : X(x), Y(y), Z(z)
  {
    ;
  }
  bool operator () (Simplex * a, Simplex * b)
  {
    double x1, y1, z1, x2, y2, z2;
    cgsmpl(a, x1, y1, z1);
    cgsmpl(b, x2, y2, z2);
    double d1 =
      (x1 - X) * (x1 - X) + (y1 - Y) * (y1 - Y) + (z1 - Z) * (z1 - Z);
    double d2 =
      (x2 - X) * (x2 - X) + (y2 - Y) * (y2 - Y) + (z2 - Z) * (z2 - Z);
    return d1 < d2;
  }
};

void DebugSimplexe(Simplex * s)
{
  int i;

  fprintf(stderr, "Simplexe %p = %d %d %d %d \n",
          s, s->V[0]->Num, s->V[1]->Num, s->V[2]->Num, s->V[3]->Num);

  for(i = 0; i < 4; i++) {
    if(s->S[i] != &MyNewBoundary)
      printf(" face : %d %d %d -> Simplexe %p\n",
             s->F[i].V[0]->Num, s->F[i].V[1]->Num, s->F[i].V[2]->Num,
             s->S[i]);
    else
      printf(" face : %d %d %d -> Simplexe Boundary\n",
             s->F[i].V[0]->Num, s->F[i].V[1]->Num, s->F[i].V[2]->Num);
  }
}

void VSIM(void *a, void *b)
{
  Simplex *S;

  S = *(Simplex **) a;
  if(S->V[3])
    volume += fabs(S->Volume_Simplexe());
}

void add_points(void *a, void *b)
{
  Tree_Insert(POINTS_TREE, a);
}

void add_points_2(void *a, void *b)
{
  List_Add(POINTS_LIST, a);
}


double Interpole_lcTetraedre(Simplex * s, Vertex * v)
{
  double mat[3][3], rhs[3], sol[3], det;

  s->matsimpl(mat);
  rhs[0] = v->Pos.X - s->V[0]->Pos.X;
  rhs[1] = v->Pos.Y - s->V[0]->Pos.Y;
  rhs[2] = v->Pos.Z - s->V[0]->Pos.Z;

  sys3x3(mat, rhs, sol, &det);
  if(det == 0.0 ||
     (1. - sol[0] - sol[1] - sol[2]) > 1. ||
     (1. - sol[0] - sol[1] - sol[2]) < 0. ||
     sol[0] > 1. ||
     sol[1] > 1. ||
     sol[2] > 1. || sol[0] < 0. || sol[1] < 0. || sol[2] < 0.) {
    return DMAX(s->V[0]->lc,
                DMAX(s->V[1]->lc, DMAX(s->V[2]->lc, s->V[3]->lc)));
    //sol[0] = sol[1] = sol[2] = 0.25;
  }

  return (s->V[0]->lc * (1. - sol[0] - sol[1] - sol[2]) +
          sol[0] * s->V[1]->lc + sol[1] * s->V[2]->lc + sol[2] * s->V[3]->lc);
}

Vertex *NewVertex(Simplex * s)
{
  Vertex *v;

  v =
    Create_Vertex(++THEM->MaxPointNum, s->Center.X, s->Center.Y, s->Center.Z,
                  1., 0.0);
  v->lc = Interpole_lcTetraedre(s, v);

  return (v);
}

inline int Pt_In_Circum(Simplex * s, Vertex * v)
{
  // Determines of a point is inside the simplexe's circumscribed sphere
  double d1 = s->Radius;
  double d2 = sqrt(DSQR(v->Pos.X - s->Center.X) +
		   DSQR(v->Pos.Y - s->Center.Y) + 
		   DSQR(v->Pos.Z - s->Center.Z));
  double eps = fabs(d1 - d2) / (d1 + d2);

  if(eps < 1.e-12) return 0; // return 1? 0?
  if(d2 < d1) return 1;
  return 0;
}

struct SimplexInteriorCheck {
  bool operator() (Simplex * s, double x[3], double u[3])
  {
    Vertex v(x[0], x[1], x[2]);
      return Pt_In_Circum(s, &v);
  }
};

struct SimplexInBox {
  double sizeBox;
  SimplexInBox(double sb) : sizeBox(sb)
  {
    ;
  }
  void operator() (Simplex * s, double min[3], double max[3])
  {
    double ss;
    if(sizeBox > s->Radius)
      ss = s->Radius;
    else
      ss = sizeBox;
    min[0] = s->Center.X - ss;
    max[0] = s->Center.X + ss;
    min[1] = s->Center.Y - ss;
    max[1] = s->Center.Y + ss;
    min[2] = s->Center.Z - ss;
    max[2] = s->Center.Z + ss;
  }
};

// Recursive search

Simplex *SearchPointByNeighbor(Vertex * v, Simplex * s, Tree_T * visited,
                               int depth)
{
  if(depth > 150)
    return 0;
  if(Tree_Search(visited, &s))
    return 0;
  Tree_Add(visited, &s);
  if(Pt_In_Circum(s, v))
    return s;
  Simplex *S[4];
  int k = 0;
  for(int i = 0; i < 4; i++) {
    if(s->S[i] != &MyNewBoundary) {
      S[k++] = s->S[i];
    }
  }
  std::sort(S, S + k, closestSimplex(v->Pos.X, v->Pos.Y, v->Pos.Z));
  for(int i = 0; i < k; i++) {
    Simplex *q = SearchPointByNeighbor(v, S[i], visited, depth + 1);
    if(q)
      return q;
  }
  return 0;
}

void Action_First_Simplexes(void *a, void *b)
{
  Simplex **q;

  if(!THES) {
    q = (Simplex **) a;
    if(Pt_In_Circum(*q, THEV)) {
      THES = *q;
    }
  }
}

void LiS(void *a, void *b)
{
  int j, N;
  SxF SXF, *pSXF;
  Simplex **pS, *S;

  pS = (Simplex **) a;
  S = *pS;
  N = (S->V[3]) ? 4 : 3;

  for(j = 0; j < N; j++) {
    SXF.F = S->F[j];
    if((pSXF = (SxF *) Tree_PQuery(SimXFac, &SXF))) {
      // Create link
      S->S[j] = pSXF->S;
      pSXF->S->S[pSXF->NumFaceSimpl] = S;
    }
    else {
      SXF.S = S;
      SXF.NumFaceSimpl = j;
      Tree_Add(SimXFac, &SXF);
    }
  }
}

void RzS(void *a, void *b)
{
  int j, N;
  Simplex **pS, *S;
  pS = (Simplex **) a;
  S = *pS;

  N = (S->V[3]) ? 4 : 3;

  for(j = 0; j < N; j++) {
    if((S->S[j]) == NULL) {
      S->S[j] = &MyNewBoundary;
    }
  }
}

// create the links between the simplices, i.e., search for neighbors

void Link_Simplexes(List_T * Sim, Tree_T * Tim)
{
  Simplex *S;
  int i;

  SimXFac = Tree_Create(sizeof(SxF), compareSxF);
  if(Sim) {
    for(i = 0; i < List_Nbr(Sim); i++) {
      List_Read(Sim, i, &S);
      LiS(&S, NULL);
    }
    for(i = 0; i < List_Nbr(Sim); i++) {
      List_Read(Sim, i, &S);
      RzS(&S, NULL);
    }
  }
  else {
    Tree_Action(Tim, LiS);
    Tree_Action(Tim, RzS);
  }
  Tree_Delete(SimXFac);
}

void Box_6_Tetraedron(List_T * P, Mesh * m)
{
#define FACT 1.1
#define LOIN 0.2

  int i, j;
  static int pts[8][3] = { 
    {0, 0, 0},
    {1, 0, 0},
    {1, 1, 0},
    {0, 1, 0},
    {0, 0, 1},
    {1, 0, 1},
    {1, 1, 1},
    {0, 1, 1}
  };
  static int tet[6][4] = { 
    {1, 5, 2, 4},
    {2, 5, 6, 4},
    {4, 5, 6, 8},
    {6, 4, 8, 7},
    {6, 4, 7, 3},
    {2, 3, 4, 6}
  };
  double Xm = 0., Ym = 0., Zm = 0., XM = 0., YM = 0., ZM = 0., Xc, Yc, Zc;
  Simplex *S, *ps;
  Vertex *V, *v, *pv;
  List_T *smp;

  smp = List_Create(8, 1, sizeof(Simplex *));

  V = (Vertex *) Malloc(8 * sizeof(Vertex));

  for(i = 0; i < List_Nbr(P); i++) {
    List_Read(P, i, &v);
    if(!i) {
      Xm = XM = v->Pos.X;
      Ym = YM = v->Pos.Y;
      Zm = ZM = v->Pos.Z;
    }
    else {
      Xm = DMIN(Xm, v->Pos.X);
      XM = DMAX(XM, v->Pos.X);
      Ym = DMIN(Ym, v->Pos.Y);
      YM = DMAX(YM, v->Pos.Y);
      Zm = DMIN(Zm, v->Pos.Z);
      ZM = DMAX(ZM, v->Pos.Z);
    }
  }
  if(Xm == XM)
    XM = Xm + 1.;
  if(Ym == YM)
    YM = Ym + 1.;
  if(Zm == ZM)
    ZM = Zm + 1.;

  Xc = XM - Xm;
  Yc = YM - Ym;
  Zc = ZM - Zm;

  LC3D = sqrt(Xc * Xc + Yc * Yc + Zc * Zc);

  /* 8 box points

     Z    8____________7
     |   /|           /|
     |  / |          / |
     | /  |         /  |
    5|/___|________/6  |
     |   4|________|___|3
     |   /         |   /
     |  / Y        |  /
     | /           | /
     |/____________|/___ X
     1             2

   */

  for(i = 0; i < 8; i++) {
    if(pts[i][0])
      V[i].Pos.X = Xm - LOIN * Xc;
    else
      V[i].Pos.X = XM + LOIN * Xc;

    if(pts[i][1])
      V[i].Pos.Y = Ym - LOIN * Yc;
    else
      V[i].Pos.Y = YM + LOIN * Yc;

    if(pts[i][2])
      V[i].Pos.Z = Zm - LOIN * Zc;
    else
      V[i].Pos.Z = ZM + LOIN * Zc;

    V[i].Num = -(++THEM->MaxPointNum);
    pv = &V[i];
    pv->lc = 1.0;
    pv->Mov = NULL;
    Tree_Replace(m->Vertices, &pv);
  }

  // 6-tet mesh of the box

  for(i = 0; i < 6; i++) {
    S = Create_Simplex(&V[tet[i][0] - 1], &V[tet[i][1] - 1],
                       &V[tet[i][2] - 1], &V[tet[i][3] - 1]);
    List_Add(smp, &S);
  }

  Link_Simplexes(smp, NULL);
  for(i = 0; i < List_Nbr(smp); i++) {
    List_Read(smp, i, &ps);
    for(j = 0; j < 4; j++)
      if(ps->S[j] == NULL)
        ps->S[j] = &MyNewBoundary;
    Tree_Replace(m->Simplexes, &ps);
  }

}


void Fill_Sim_Des(void *a, void *b)
{
  Simplex **S;
  S = (Simplex **) a;
  if(Pt_In_Circum(*S, THEV))
    List_Add(Simplexes_Destroyed, a);
}

void TStoLS(void *a, void *b)
{
  List_Add(Simplexes_Destroyed, a);
}

void TAtoLA(void *a, void *b)
{
  List_Add(Simplexes_New, a);
}

void CrSi(void *a, void *b)
{
  SxF *S;
  Simplex *s;
  S = (SxF *) a;
  if(S->NumFaceSimpl == 1) {
    s = Create_Simplex(THEV, S->F.V[0], S->F.V[1], S->F.V[2]);
    s->iEnt = ZONEELIMINEE;
    THEM->Metric->setSimplexQuality(s);
    List_Add(Simplexes_New, &s);
  }
  else if(S->NumFaceSimpl != 2) {
    Msg(WARNING, "Huh! Panic in CrSi");
  }
}


void NewSimplexes(Mesh * m, List_T * Sim, List_T * news)
{
  int i, j;
  Tree_T *SimXFac;
  Simplex *S;
  SxF SXF, *pSXF;

  SimXFac = Tree_Create(sizeof(SxF), compareSxF);

  for(i = 0; i < List_Nbr(Sim); i++) {
    List_Read(Sim, i, &S);
    if(!i)
      ZONEELIMINEE = S->iEnt;
    else {
      if(S->iEnt != ZONEELIMINEE) {
        Msg(WARNING, "Huh! The elimination failed %d %d",
            S->iEnt, ZONEELIMINEE);
      }
    }
    for(j = 0; j < 4; j++) {
      SXF.F = S->F[j];
      if((pSXF = (SxF *) Tree_PQuery(SimXFac, &SXF))) {
        (pSXF->NumFaceSimpl)++;
      }
      else {
        SXF.NumFaceSimpl = 1;
        Tree_Add(SimXFac, &SXF);
      }
    }
  }

  // The non-common faces are on the boundary -> create new simplices
  Tree_Action(SimXFac, CrSi);
  Tree_Delete(SimXFac);
}


/* Methode recursive : Rempli Tsd les simplexes detruits 
   Invariant : Le simplexe est a eliminer
   Le simplexe n'est pas encore considere */

int recur_bowyer(Simplex * s)
{
  int i;

  Tree_Insert(Tsd, &s);
  for(i = 0; i < 4; i++) {
    if(s->S[i] && s->S[i] != &MyNewBoundary && !Tree_Query(Tsd, &s->S[i])) {
      if(Pt_In_Circum(s->S[i], THEV) && (s->iEnt == s->S[i]->iEnt)) {
        recur_bowyer(s->S[i]);
      }
      else {
        if(s->iEnt != s->S[i]->iEnt) {
          //Msg(WARNING, "Point scabreux %d", s->S[i]->Num);
          Alerte_Point_Scabreux = 1;
        }
        Tree_Insert(Sim_Sur_Le_Bord, &s->S[i]);
      }
    }
  }
  return 1;
}



bool Bowyer_Watson(Mesh * m, Vertex * v, Simplex * S, int force)
{
  int i;
  Simplex *s;
  double volumeold, volumenew;

  THEV = v;

  double x =
    (S->V[0]->Pos.X + S->V[1]->Pos.X + S->V[2]->Pos.X + S->V[3]->Pos.X) / 4.;
  double y =
    (S->V[0]->Pos.Y + S->V[1]->Pos.Y + S->V[2]->Pos.Y + S->V[3]->Pos.Y) / 4.;
  double z =
    (S->V[0]->Pos.Z + S->V[1]->Pos.Z + S->V[2]->Pos.Z + S->V[3]->Pos.Z) / 4.;

  if(force)
    THEM->Metric->Identity();
  else
    THEM->Metric->setMetric(x, y, z);

  Tsd = Tree_Create(sizeof(Simplex *), compareSimplex);
  Sim_Sur_Le_Bord = Tree_Create(sizeof(Simplex *), compareSimplex);
  List_Reset(Simplexes_Destroyed);
  List_Reset(Simplexes_New);

  if(Methode) {
    Tree_Action(m->Simplexes, Fill_Sim_Des);
  }
  else {
    recur_bowyer(S);
  }

  Tree_Action(Tsd, TStoLS);
  NewSimplexes(m, Simplexes_Destroyed, Simplexes_New);

  // compute volume

  if(Alerte_Point_Scabreux || !CTX.mesh.speed_max) {
    volume = 0.0;
    for(i = 0; i < List_Nbr(Simplexes_Destroyed); i++) {
      VSIM(List_Pointer(Simplexes_Destroyed, i), NULL);
    }
    volumeold = volume;
    volume = 0.0;
    for(i = 0; i < List_Nbr(Simplexes_New); i++) {
      VSIM(List_Pointer(Simplexes_New, i), NULL);
    }
    volumenew = volume;
  }
  else {
    volumeold = 1.0;
    volumenew = 1.0;
  }

  // volume criterion

  if((fabs(volumeold - volumenew) / (volumeold + volumenew)) > 1.e-8) {
    if(Tree_Suppress(m->Simplexes, &S)) {
      S->Quality = 0.0;
      Tree_Add(m->Simplexes, &S);
    }
    if(force) {
      List_Reset(Simplexes_Destroyed);
      List_Reset(Simplexes_New);
      Tree_Delete(Sim_Sur_Le_Bord);
      Tree_Delete(Tsd);
      //printf(" Aie Aie Aie volume changed %g -> %g\n",volumeold,volumenew);
      return false;
    }
  }
  else {
    Tree_Add(m->Vertices, &THEV);
    for(i = 0; i < List_Nbr(Simplexes_New); i++) {
      Simplex *theNewS;
      List_Read(Simplexes_New, i, &theNewS);
#if 0
      if(force) {
	double xc = theNewS->Center.X;
	double yc = theNewS->Center.Y;
	double zc = theNewS->Center.Z;
	double rd = theNewS->Radius;   
	cgsmpl (theNewS,x,y,z);
	THEM->Metric->setMetric (x, y, z);
	THEM->Metric->setSimplexQuality (theNewS);
	theNewS->Center.X = xc;
	theNewS->Center.Y = yc;
	theNewS->Center.Z = zc;
	theNewS->Radius = rd;  
      }
#endif
      Tree_Add(m->Simplexes, &theNewS);
    }

    // remove deleted tets

    for(i = 0; i < List_Nbr(Simplexes_Destroyed); i++) {
      List_Read(Simplexes_Destroyed, i, &s);
      if(!Tree_Suppress(m->Simplexes, &s))
        Msg(GERROR, "Impossible to delete simplex");
      Free_Simplex(&s, 0);
    }

    // create links between new tets

    Tree_Action(Sim_Sur_Le_Bord, TAtoLA);
    Link_Simplexes(Simplexes_New, m->Simplexes);
  }

  Tree_Delete(Sim_Sur_Le_Bord);
  Tree_Delete(Tsd);
  return true;
}

void Convex_Hull_Mesh(List_T * Points, Mesh * m)
{
  int i, j, N, n;

  N = List_Nbr(Points);
  n = IMAX(N / 20, 1);

  //clock_t t1 = clock();

  Box_6_Tetraedron(Points, m);
  List_Sort(Points, comparePosition);

  int Nb1 = 0, Nb2 = 0, Nb3 = 0;

  for(i = 0; i < N; i++) {
    THES = NULL;
    List_Read(Points, i, &THEV);
    if(Simplexes_New)
      for(j = 0; j < List_Nbr(Simplexes_New); j++) {
        Simplex *simp;
        List_Read(Simplexes_New, j, &simp);
        if(Pt_In_Circum(simp, THEV)) {
          THES = simp;
          break;
        }
      }

    if(THES)
      Nb1++;

    if(!THES) {
      if(Simplexes_New && List_Nbr(Simplexes_New)) {
        Tree_T *visited = Tree_Create(sizeof(Simplex *), compareSimplex);
        Simplex *simp;
        List_Read(Simplexes_New, 0, &simp);
        THES = SearchPointByNeighbor(THEV, simp, visited, 0);
        Tree_Delete(visited);
      }
      if(THES)
        Nb2++;
    }


    if(!THES) {
      Tree_Action(m->Simplexes, Action_First_Simplexes);
      if(THES)
        Nb3++;
    }

    if(i % n == n - 1) {
      volume = 0.0;
      Tree_Action(m->Simplexes, VSIM);
      Msg(STATUS3, "Nod=%d/%d Elm=%d", i + 1, N, Tree_Nbr(m->Simplexes));
      Msg(STATUS1, "Vol=%g (%d %d %d)", volume, Nb1, Nb2, Nb3);
    }
    if(!THES) {
      Msg(WARNING, "Vertex (%g,%g,%g) in no simplex", THEV->Pos.X,
          THEV->Pos.Y, THEV->Pos.Z);
      THEV->Pos.X +=
        CTX.mesh.rand_factor * LC3D * (1. -
                                       (double)rand() / (double)RAND_MAX);
      THEV->Pos.Y +=
        CTX.mesh.rand_factor * LC3D * (1. -
                                       (double)rand() / (double)RAND_MAX);
      THEV->Pos.Z +=
        CTX.mesh.rand_factor * LC3D * (1. -
                                       (double)rand() / (double)RAND_MAX);
      Tree_Action(m->Simplexes, Action_First_Simplexes);
    }
    bool ca_marche = Bowyer_Watson(m, THEV, THES, 1);
    int count = 0;
    while(!ca_marche) {
      count++;
      double dx =
        CTX.mesh.rand_factor * LC3D * (1. -
                                       (double)rand() / (double)RAND_MAX);
      double dy =
        CTX.mesh.rand_factor * LC3D * (1. -
                                       (double)rand() / (double)RAND_MAX);
      double dz =
        CTX.mesh.rand_factor * LC3D * (1. -
                                       (double)rand() / (double)RAND_MAX);
      THEV->Pos.X += dx;
      THEV->Pos.Y += dy;
      THEV->Pos.Z += dz;
      THES = NULL;
      Tree_Action(m->Simplexes, Action_First_Simplexes);
      ca_marche = Bowyer_Watson(m, THEV, THES, 1);
      THEV->Pos.X -= dx;
      THEV->Pos.Y -= dy;
      THEV->Pos.Z -= dz;
      if(count > 5) {
        N++;
        List_Add(POINTS_LIST, &THEV);
        Msg(WARNING, "Unable to add point %d (will try it later)", THEV->Num);
        break;
      }
    }
  }
  //clock_t t2 = clock();

  //Msg(STATUS3,"Nb1 = %d Nb2 = %d Nb3 = %d N = %d t = %lf",Nb1,Nb2,Nb3
  //    ,N,(double)(t2-t1)/CLOCKS_PER_SEC);
}

void suppress_vertex(void *data, void *dum)
{
  Vertex **pv;

  pv = (Vertex **) data;
  if((*pv)->Num < 0)
    List_Add(Suppress, pv);
}

void suppress_simplex(void *data, void *dum)
{
  Simplex **pv;

  pv = (Simplex **) data;
  if((*pv)->iEnt == 0)
    List_Add(Suppress, pv);
}

void Maillage_Volume(void *data, void *dum)
{
  Volume *v, **pv;
  Mesh M;
  Surface S, *s;
  Simplex *simp;
  Vertex *newv;
  int n, N;
  double uvw[3];
  int i;

  FACE_DIMENSION = 2;

  pv = (Volume **) data;
  v = *pv;

  /*  if(v->Typ == MSH_VOLUME_DISCRETE) 
    {
      printf("coucou1\n");
      int temp = CTX.mesh.algo3d;
      CTX.mesh.algo3d = FRONTAL_NETGEN;
      CTX.mesh.algo3d =DELAUNAY_TETGEN;
      Mesh_Tetgen(v);
      CTX.mesh.algo3d = temp;
    }
    else*/ if(Extrude_Mesh(v)) {
  }
  else if(MeshTransfiniteVolume(v)) {
  }
  else if((v->Typ != 99999) && Mesh_Netgen(v)) {
  }
  else if((v->Typ != 99999) && Mesh_Tetgen(v)) {
  }
  else if((v->Typ == 99999) && (CTX.mesh.algo3d != FRONTAL_NETGEN)
                            && (CTX.mesh.algo3d != DELAUNAY_TETGEN)) {
    
    Simplexes_New = List_Create(10, 10, sizeof(Simplex *));
    Simplexes_Destroyed = List_Create(10, 10, sizeof(Simplex *));

    Mesh *LOCAL = &M;
    s = &S;

    POINTS_TREE = Tree_Create(sizeof(Vertex *), comparePosition);
    POINTS_LIST = List_Create(100, 100, sizeof(Vertex *));
    LOCAL->Simplexes = v->Simplexes;
    LOCAL->Vertices = v->Vertices;
    

    for(i = 0; i < List_Nbr(v->Surfaces); i++) {
      List_Read(v->Surfaces, i, &s);
      Tree_Action(s->Vertices, add_points);
    }
    Tree_Action(POINTS_TREE, add_points_2);
    Tree_Delete(POINTS_TREE);

    N = List_Nbr(POINTS_LIST);
    n = N / 30 + 1;

    if(!N)
      return;

    // Create initial mesh respecting the boundary

    Msg(STATUS2, "Mesh 3D... (initial)");

    Convex_Hull_Mesh(POINTS_LIST, LOCAL);

    if(!Coherence(v, LOCAL))
      Msg(GERROR,
          "Surface recovery failed (send a bug report with the geo file to <gmsh@geuz.org>!)");

    Link_Simplexes(NULL, LOCAL->Simplexes);

    if(CTX.mesh.initial_only == 3) {
      POINTS_TREE = THEM->Vertices;
      Tree_Action(v->Vertices, add_points);
      POINTS_TREE = THEM->Simplexes;
      Tree_Action(v->Simplexes, add_points);
      return;
    }

    // remove nodes with nums < 0

    Suppress = List_Create(10, 10, sizeof(Vertex *));
    Tree_Action(v->Vertices, suppress_vertex);
    for(i = 0; i < List_Nbr(Suppress); i++) {
      Tree_Suppress(v->Vertices, List_Pointer(Suppress, i));
    }
    List_Delete(Suppress);

    // remove elements whose volume num==0 (i.e., that don't belong to
    // any volume)

    Suppress = List_Create(10, 10, sizeof(Simplex *));
    Tree_Action(v->Simplexes, suppress_simplex);
    for(i = 0; i < List_Nbr(Suppress); i++) {
      Tree_Suppress(v->Simplexes, List_Pointer(Suppress, i));
    }

    List_Delete(Suppress);

    if(Tree_Nbr(LOCAL->Simplexes) == 0)
      return;

    // If there is something left to mesh:

    Msg(STATUS2, "Mesh 3D... (final)");

    v->Simplexes = LOCAL->Simplexes;

    POINTS_TREE = THEM->Simplexes;

    Tree_Right(LOCAL->Simplexes, &simp);
    i = 0;

    while(simp->Quality > CONV_VALUE) {
      newv = NewVertex(simp);
      while(!simp->Pt_In_Simplexe(newv, uvw, 1.e-5) &&
            (simp->S[0] == &MyNewBoundary ||
             !simp->S[0]->Pt_In_Simplexe(newv, uvw, 1.e-5)) &&
            (simp->S[1] == &MyNewBoundary ||
             !simp->S[1]->Pt_In_Simplexe(newv, uvw, 1.e-5)) &&
            (simp->S[2] == &MyNewBoundary ||
             !simp->S[2]->Pt_In_Simplexe(newv, uvw, 1.e-5)) &&
            (simp->S[3] == &MyNewBoundary ||
             !simp->S[3]->Pt_In_Simplexe(newv, uvw, 1.e-5))) {
        Tree_Suppress(LOCAL->Simplexes, &simp);
        simp->Quality = 0.1;
        Tree_Insert(LOCAL->Simplexes, &simp);
        Tree_Right(LOCAL->Simplexes, &simp);
        if(simp->Quality < CONV_VALUE)
          break;
        newv = NewVertex(simp);
      }
      if(simp->Quality < CONV_VALUE)
        break;
      i++;
      if(i % n == n - 1) {
        volume = 0.0;
        Tree_Action(LOCAL->Simplexes, VSIM);
        Msg(STATUS3, "Nod=%d Elm=%d",
            Tree_Nbr(LOCAL->Vertices), Tree_Nbr(LOCAL->Simplexes));
        Msg(STATUS1, "Vol(%g) Conv(%g->%g)", volume, simp->Quality,
            CONV_VALUE);
      }
      Bowyer_Watson(LOCAL, newv, simp, 0);
      Tree_Right(LOCAL->Simplexes, &simp);
    }

    POINTS_TREE = THEM->Vertices;
    Tree_Action(v->Vertices, add_points);
    POINTS_TREE = THEM->Simplexes;
    Tree_Action(v->Simplexes, add_points);

#if 0 // this is full of bugs :-)
    if(CTX.mesh.quality) {
      extern void SwapEdges3D(Mesh * M, Volume * v, double GammaPrescribed,
                              bool order);
      Msg(STATUS3, "Swapping edges (1st pass)");
      SwapEdges3D(THEM, v, CTX.mesh.quality, true);
      Msg(STATUS3, "Swapping edges (2nd pass)");
      SwapEdges3D(THEM, v, CTX.mesh.quality, false);
      Msg(STATUS3, "Swapping edges (last pass)");
      SwapEdges3D(THEM, v, CTX.mesh.quality, true);
    }
#endif

#if 0 // this is full of bugs, too :-)
    if(CTX.mesh.nb_smoothing) {
      Msg(STATUS3, "Smoothing volume %d", v->Num);
      tnxe = Tree_Create (sizeof (NXE), compareNXE);
      create_NXE (v->Vertices, v->Simplexes, tnxe);
      for (int i = 0; i < CTX.mesh.nb_smoothing; i++)
	Tree_Action (tnxe, ActionLiss);
      delete_NXE (tnxe);
      Msg(STATUS3, "Swapping edges (last pass)");
      SwapEdges3D (THEM, v, 0.5, true);
    }
#endif

    List_Delete(Simplexes_New);
    List_Delete(Simplexes_Destroyed);
  }

}
