// $Id: Geom.cpp,v 1.97 2006-01-10 03:58:31 geuzaine Exp $
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

#include "BDS.h"
#include "Gmsh.h"
#include "GmshUI.h"
#include "Numeric.h"
#include "Geo.h"
#include "CAD.h"
#include "Mesh.h"
#include "Utils.h"
#include "Draw.h"
#include "Context.h"
#include "Interpolation.h"
#include "Plugin.h"
#include "PluginManager.h"
#include "gl2ps.h"

extern Context_T CTX;
extern Mesh *THEM;

// Points

void Draw_Geo_Point(void *a, void *b)
{
  char Num[100];

  Vertex *v = *(Vertex **) a;

  if(!(v->Visible & VIS_GEOM))
    return;

  if(CTX.render_mode == GMSH_SELECT) {
    glPushName(0);
    glPushName(v->Num);
  }

  if(v->Frozen) {
    glPointSize(CTX.geom.point_sel_size);
    gl2psPointSize(CTX.geom.point_sel_size * CTX.print.eps_point_size_factor);
    glColor4ubv((GLubyte *) & CTX.color.geom.point_sel);
  }
  else {
    glPointSize(CTX.geom.point_size);
    gl2psPointSize(CTX.geom.point_size * CTX.print.eps_point_size_factor);
    glColor4ubv((GLubyte *) & CTX.color.geom.point);
  }

  if(CTX.geom.points) {
    if(CTX.geom.point_type == 1) {
      if(v->Frozen)
	Draw_Sphere(CTX.geom.point_sel_size, v->Pos.X, v->Pos.Y, v->Pos.Z, 
		    CTX.geom.light);
      else
	Draw_Sphere(CTX.geom.point_size, v->Pos.X, v->Pos.Y, v->Pos.Z,
		    CTX.geom.light);
    }
    else if(CTX.geom.point_type == 2) {
      GMSH_Solve_Plugin *sp = GMSH_PluginManager::instance()->findSolverPlugin();
      if(sp) {
	sp-> GL_enhancePoint (v);
      }
      glBegin(GL_POINTS);
      glVertex3d(v->Pos.X, v->Pos.Y, v->Pos.Z);
      glEnd();
    }
    else {
      glBegin(GL_POINTS);
      glVertex3d(v->Pos.X, v->Pos.Y, v->Pos.Z);
      glEnd();
    }

  }

  if(CTX.geom.points_num) {
    sprintf(Num, "%d", v->Num);
    double offset = (0.5 * CTX.geom.point_size + 0.3 * CTX.gl_fontsize) * CTX.pixel_equiv_x;
    glRasterPos3d(v->Pos.X + offset / CTX.s[0],
		  v->Pos.Y + offset / CTX.s[1],
		  v->Pos.Z + offset / CTX.s[2]);
    Draw_String(Num);
  }

  if(CTX.render_mode == GMSH_SELECT) {
    glPopName();
    glPopName();
  }
}

// Curves

void Draw_Curve(void *a, void *b)
{
  int N;
  double mod, x[2], y[2], z[2];
  char Num[100];
  Vertex v, dv;

  Curve *c = *(Curve **) a;

  if(c->Num < 0 || !(c->Visible & VIS_GEOM))
    return;

  if(CTX.render_mode == GMSH_SELECT) {
    glPushName(1);
    glPushName(c->Num);
  }

  if(c->ipar[3] > 0) {
    glLineWidth(CTX.geom.line_sel_width);
    gl2psLineWidth(CTX.geom.line_sel_width * CTX.print.eps_line_width_factor);
    glColor4ubv((GLubyte *) & CTX.color.geom.line_sel);
  }
  else {
    glLineWidth(CTX.geom.line_width);
    gl2psLineWidth(CTX.geom.line_width * CTX.print.eps_line_width_factor);
    glColor4ubv((GLubyte *) & CTX.color.geom.line);
  }

  if(CTX.geom.lines) {
    int n = List_Nbr(c->Control_Points);
    switch (c->Typ) {
    case MSH_SEGM_LINE:
      N = n;
      break;
    case MSH_SEGM_CIRC:
    case MSH_SEGM_CIRC_INV:
    case MSH_SEGM_ELLI:
    case MSH_SEGM_ELLI_INV:
      N = CTX.geom.circle_points;
      break;
    default:
      N = 10 * n;
      break;
    }
    if(c->Typ == MSH_SEGM_DISCRETE && THEM->bds) {
      BDS_GeomEntity *g = THEM->bds->get_geom ( c->Num,1);	
      std::list<BDS_Edge*>::iterator it  = g->e.begin();
      std::list<BDS_Edge*>::iterator ite = g->e.end();
      while (it!=ite){
	BDS_Edge *e = (*it);
	if(CTX.geom.line_type < 1) {
	  glBegin(GL_LINES);
	  glVertex3d(e->p1->X,e->p1->Y,e->p1->Z);
	  glVertex3d(e->p2->X,e->p2->Y,e->p2->Z);
	  glEnd();
	}
	else{
	  x[0] = e->p1->X;
	  y[0] = e->p1->Y;
	  z[0] = e->p1->Z;
	  x[1] = e->p2->X;
	  y[1] = e->p2->Y;
	  z[1] = e->p2->Z;
	  Draw_Cylinder(c->ipar[3] > 0 ? CTX.geom.line_sel_width : CTX.geom.line_width,
			x, y, z, CTX.geom.light);
	}
	++it;
      }
    }
    else if(c->Typ == MSH_SEGM_DISCRETE) {
      // do nothing: we draw the elements in the mesh drawing routines
    }
    else {
      if(CTX.geom.line_type >= 1) {
	for(int i = 0; i < N - 1; i++) {
	  v = InterpolateCurve(c, (double)i / (double)(N - 1), 0);
	  dv = InterpolateCurve(c, (double)(i + 1) / (double)(N - 1), 0);
	  x[0] = v.Pos.X;
	  y[0] = v.Pos.Y;
	  z[0] = v.Pos.Z;
	  x[1] = dv.Pos.X;
	  y[1] = dv.Pos.Y;
	  z[1] = dv.Pos.Z;
	  Draw_Cylinder(c->ipar[3] > 0 ? CTX.geom.line_sel_width : CTX.geom.line_width,
			x, y, z, CTX.geom.light);
	}
	if(CTX.geom.line_type == 2) {
	  GMSH_Solve_Plugin *sp = GMSH_PluginManager::instance()->findSolverPlugin();
	  if(sp) {
	    int NN=(N>1)?N:1;
	    const double eps=0.e-2;
	    for(int i = 0; i < NN - 1; i++) {
	      v = InterpolateCurve(c, (double)i / (double)(NN - 1)-eps, 0);
	      dv = InterpolateCurve(c, (double)(i + 1) / (double)(NN - 1)+eps, 0);
	      sp-> GL_enhanceLine (c->Num,&v,&dv);
	    }
	  }
	}
      }
      else {
	glBegin(GL_LINE_STRIP);
	for(int i = 0; i < N; i++) {
	  v = InterpolateCurve(c, (double)i / (double)(N - 1), 0);
	  glVertex3d(v.Pos.X, v.Pos.Y, v.Pos.Z);
	}
	glEnd();
      }
    }
  }

  if(CTX.geom.lines_num) {
    v = InterpolateCurve(c, 0.5, 0);
    sprintf(Num, "%d", c->Num);
    double offset = (0.5 * CTX.geom.line_width + 0.3 * CTX.gl_fontsize) * CTX.pixel_equiv_x;
    glRasterPos3d(v.Pos.X + offset / CTX.s[0],
		  v.Pos.Y + offset / CTX.s[1],
		  v.Pos.Z + offset / CTX.s[2]);
    Draw_String(Num);
  }

  if(CTX.geom.tangents) {
    v = InterpolateCurve(c, 0.5, 0);
    dv = InterpolateCurve(c, 0.5, 1);
    mod = sqrt(dv.Pos.X * dv.Pos.X + dv.Pos.Y * dv.Pos.Y + dv.Pos.Z * dv.Pos.Z);
    dv.Pos.X = dv.Pos.X / mod * CTX.geom.tangents * CTX.pixel_equiv_x / CTX.s[0];
    dv.Pos.Y = dv.Pos.Y / mod * CTX.geom.tangents * CTX.pixel_equiv_x / CTX.s[1];
    dv.Pos.Z = dv.Pos.Z / mod * CTX.geom.tangents * CTX.pixel_equiv_x / CTX.s[2];
    glColor4ubv((GLubyte *) & CTX.color.geom.tangents);
    Draw_Vector(CTX.vector_type, 0, CTX.arrow_rel_head_radius, 
		CTX.arrow_rel_stem_length, CTX.arrow_rel_stem_radius,
		v.Pos.X, v.Pos.Y, v.Pos.Z,
		dv.Pos.X, dv.Pos.Y, dv.Pos.Z, CTX.geom.light);
  }

  if(CTX.render_mode == GMSH_SELECT) {
    glPopName();
    glPopName();
  }
}

// Surfaces

void putZ(Vertex * v, Surface * s)
{
  Vertex V;
  V.Pos.X = s->a;
  V.Pos.Y = s->b;
  V.Pos.Z = s->c;
  Projette(&V, s->plan);
  if(V.Pos.Z != 0.0)
    v->Pos.Z = (s->d - V.Pos.X * v->Pos.X - V.Pos.Y * v->Pos.Y) / V.Pos.Z;
  else
    v->Pos.Z = 0.0;

  Projette(v, s->invplan);
}

int isPointOnPlanarSurface(Surface * S, double X, double Y, double Z,
                           double n[3])
{
  double Angle = 0.0;
  Vertex V;
  V.Pos.X = X;
  V.Pos.Y = Y;
  V.Pos.Z = Z;

  for(int i = 0; i < List_Nbr(S->Generatrices); i++) {

    Curve *C;
    List_Read(S->Generatrices, i, &C);

    int N = (C->Typ == MSH_SEGM_LINE) ? 1 : 10;

    for(int j = 0; j < N; j++) {
      double u1 = (double)j / (double)(N);
      double u2 = (double)(j + 1) / (double)(N);
      Vertex P1 = InterpolateCurve(C, u1, 0);
      Vertex P2 = InterpolateCurve(C, u2, 0);
      Angle += angle_plan(&V, &P1, &P2, n);
    }

  }

  // if Angle == 2*pi, we're inside

  if(fabs(Angle) > 2*M_PI-0.5 && fabs(Angle) < 2*M_PI+0.5) 
    return 1;
  return 0;
}

void getPlaneSurfaceNormal(Surface *s, double x, double y, double z, double n[3])
{
  double t1[3], t2[3];
  Curve *c;

  for(int i = 0; i < 3; i++)
    n[i] = 0.0;

  if(s->Typ != MSH_SURF_PLAN)
    return;

  // We cannot use s->plan or InterpolateSurface here (they both rely
  // on the mean plane, which borks the orientation). So we need to
  // use this trick:

  if(List_Nbr(s->Generatrices) > 1){
    List_Read(s->Generatrices, 0, &c);
    if(c->beg){
      t1[0] = x - c->beg->Pos.X;
      t1[1] = y - c->beg->Pos.Y;
      t1[2] = z - c->beg->Pos.Z;
      // 1) try to get a point close to 'beg' on the same curve
      // 2) if we are really unlucky and these two points are aligned 
      // with (x,y,z), which we know is inside or on the boundary of
      // the surface, then get a point from the next generatrice
      // 3) repeat
      for(int i = 0; i < List_Nbr(s->Generatrices); i++){
	List_Read(s->Generatrices, i, &c);
	Vertex v = InterpolateCurve(c, 0.1, 0);
	t2[0] = x - v.Pos.X;
	t2[1] = y - v.Pos.Y;
	t2[2] = z - v.Pos.Z;
	prodve(t1, t2, n);
	if(norme(n))
	  break;
      }
    }
  }
  if(List_Nbr(s->Generatrices) < 2 || !norme(n)){
    Msg(WARNING, "Reverting to mean plane normal for surface %d", s->Num);
    n[0] = s->a;
    n[1] = s->b;
    n[2] = s->c;
    norme(n);
  }
}

void Draw_Polygonal_Surface(Surface * s)
{
  if(CTX.geom.surfaces) {
    if(CTX.geom.light) glEnable(GL_LIGHTING);
    glEnable(GL_POLYGON_OFFSET_FILL); // always!
    BDS_GeomEntity *g = THEM->bds->get_geom ( s->Num,2);	
    std::list<BDS_Triangle*>::iterator it  = g->t.begin();
    std::list<BDS_Triangle*>::iterator ite = g->t.end();
    while (it!=ite) {
      glBegin(GL_TRIANGLES);
      BDS_Point *n[3];
      BDS_Triangle *t = (*it);
      double c[3];
      t->getNodes (n);
      normal_triangle (n[0],n[1],n[2],c);
      if(CTX.geom.light) glNormal3dv(c);
      glVertex3d(n[0]->X,n[0]->Y,n[0]->Z);
      if(CTX.geom.light) glNormal3dv(c);
      glVertex3d(n[1]->X,n[1]->Y,n[1]->Z);
      if(CTX.geom.light) glNormal3dv(c);
      glVertex3d(n[2]->X,n[2]->Y,n[2]->Z);
      glEnd();
      ++it;
    }
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_LIGHTING);
  }  
}

void Draw_Plane_Surface(Surface * s)
{
  Curve *c;
  Vertex V[4], vv, vv1, vv2;
  double n[3];

  if(!CTX.threads_lock && List_Nbr(s->Orientations) < 1) {
    CTX.threads_lock = 1;

    List_T *points = List_Create(10, 10, sizeof(Vertex *));
    for(int i = 0; i < List_Nbr(s->Generatrices); i++) {
      List_Read(s->Generatrices, i, &c);
      for(int j = 0; j < List_Nbr(c->Control_Points); j++) {
	List_Add(points, List_Pointer(c->Control_Points, j));
      }
    }

    if(!List_Nbr(points)){
      List_Delete(points);      
      CTX.threads_lock = 0;
      return;
    }
    MeanPlane(points, s);
    List_Delete(points);
    
    // compute (rough) bounding box of surface
    double minx = 0., miny = 0., maxx = 0., maxy = 0.;
    for(int i = 0; i < List_Nbr(s->Generatrices); i++) {
      List_Read(s->Generatrices, i, &c);
      Vertex P1 = InterpolateCurve(c, 0.0, 0);
      Vertex P2 = InterpolateCurve(c, 0.5, 0);
      Vertex P3 = InterpolateCurve(c, 1.0, 0);
      Projette(&P1, s->plan);
      Projette(&P2, s->plan);
      Projette(&P3, s->plan);
      if(!i) {
	minx = maxx = P1.Pos.X;
	miny = maxy = P1.Pos.Y;
      }
      minx = DMIN(DMIN(DMIN(minx, P1.Pos.X), P2.Pos.X), P3.Pos.X);
      miny = DMIN(DMIN(DMIN(miny, P1.Pos.Y), P2.Pos.Y), P3.Pos.Y);
      maxx = DMAX(DMAX(DMAX(maxx, P1.Pos.X), P2.Pos.X), P3.Pos.X);
      maxy = DMAX(DMAX(DMAX(maxy, P1.Pos.Y), P2.Pos.Y), P3.Pos.Y);
    }

    V[0].Pos.X = minx;
    V[0].Pos.Y = miny;
    V[1].Pos.X = maxx;
    V[1].Pos.Y = miny;
    V[2].Pos.X = maxx;
    V[2].Pos.Y = maxy;
    V[3].Pos.X = minx;
    V[3].Pos.Y = maxy;

    for(int i = 0; i < 4; i++) {
      V[i].Pos.Z = 0.0;
      putZ(&V[i], s);
    }

    n[0] = s->plan[2][0];
    n[1] = s->plan[2][1];
    n[2] = s->plan[2][2];
    norme(n);

    // compute which parts of the "middle cross" lie inside the surface
    const int numPoints = 200;
    for(int cross = 0; cross < 2; cross++) {
      int k = 0;
      for(int i = 0; i < numPoints; i++) {
	double t = (double)i / (double)(numPoints-1);
	if(!cross){
	  vv.Pos.X = t * 0.5 * (V[0].Pos.X + V[1].Pos.X) + 
	    (1. - t) * 0.5 * (V[2].Pos.X + V[3].Pos.X);
	  vv.Pos.Y = t * 0.5 * (V[0].Pos.Y + V[1].Pos.Y) + 
	    (1. - t) * 0.5 * (V[2].Pos.Y + V[3].Pos.Y);
	  vv.Pos.Z = t * 0.5 * (V[0].Pos.Z + V[1].Pos.Z) + 
	    (1. - t) * 0.5 * (V[2].Pos.Z + V[3].Pos.Z);
	}
	else{
	  vv.Pos.X = t * .5 * (V[0].Pos.X + V[3].Pos.X) + 
	    (1. - t) * .5 * (V[2].Pos.X + V[1].Pos.X);
	  vv.Pos.Y = t * .5 * (V[0].Pos.Y + V[3].Pos.Y) + 
	    (1. - t) * .5 * (V[2].Pos.Y + V[1].Pos.Y);
	  vv.Pos.Z = t * .5 * (V[0].Pos.Z + V[3].Pos.Z) + 
	    (1. - t) * .5 * (V[2].Pos.Z + V[1].Pos.Z);
	}
	if(isPointOnPlanarSurface(s, vv.Pos.X, vv.Pos.Y, vv.Pos.Z, n)) {
	  if(!k) {
	    List_Add(s->Orientations, &vv);
	    k = 1;
	  }
	}
	else {
	  if(k) {
	    List_Add(s->Orientations, &vv);
	    k = 0;
	  }
	}
      }
      if(k)
	List_Add(s->Orientations, &vv);
    }

    Msg(STATUS2, "Plane Surface %d (%d cross points)", 
	s->Num, List_Nbr(s->Orientations));
    
    if(!List_Nbr(s->Orientations)){ // add dummy
      List_Add(s->Orientations, &vv);
      //printf("surf %d: min=%g %g max=%g %g\n", s->Num, minx, miny, maxx, maxy);
    }

    CTX.threads_lock = 0;
  }
  
  if(List_Nbr(s->Orientations) > 1) {

    if(CTX.geom.surfaces) {
      glEnable(GL_LINE_STIPPLE);
      glLineStipple(1, 0x1F1F);
      gl2psEnable(GL2PS_LINE_STIPPLE);
      glBegin(GL_LINES);
      for(int i = 0; i < List_Nbr(s->Orientations); i++) {
	List_Read(s->Orientations, i, &vv);
	glVertex3d(vv.Pos.X, vv.Pos.Y, vv.Pos.Z);
      }
      glEnd();
      glDisable(GL_LINE_STIPPLE);
      gl2psDisable(GL2PS_LINE_STIPPLE);
    }

    if(CTX.geom.surfaces_num) {
      List_Read(s->Orientations, 0, &vv1);
      List_Read(s->Orientations, 1, &vv2);
      char Num[100];
      sprintf(Num, "%d", s->Num);
      double offset = 0.3 * CTX.gl_fontsize * CTX.pixel_equiv_x;
      glRasterPos3d((vv2.Pos.X + vv1.Pos.X) / 2. + offset / CTX.s[0],
		    (vv2.Pos.Y + vv1.Pos.Y) / 2. + offset / CTX.s[1],
		    (vv2.Pos.Z + vv1.Pos.Z) / 2. + offset / CTX.s[2]);
      Draw_String(Num);
    }

    if(CTX.geom.normals) {
      List_Read(s->Orientations, 0, &vv1);
      List_Read(s->Orientations, 1, &vv2);
      double x = (vv1.Pos.X + vv2.Pos.X) / 2.;
      double y = (vv1.Pos.Y + vv2.Pos.Y) / 2.;
      double z = (vv1.Pos.Z + vv2.Pos.Z) / 2.;
      getPlaneSurfaceNormal(s, x, y, z, n);
      n[0] *= CTX.geom.normals * CTX.pixel_equiv_x / CTX.s[0];
      n[1] *= CTX.geom.normals * CTX.pixel_equiv_x / CTX.s[1];
      n[2] *= CTX.geom.normals * CTX.pixel_equiv_x / CTX.s[2];
      glColor4ubv((GLubyte *) & CTX.color.geom.normals);
      Draw_Vector(CTX.vector_type, 0, CTX.arrow_rel_head_radius, 
		  CTX.arrow_rel_stem_length, CTX.arrow_rel_stem_radius, 
		  x, y, z, n[0], n[1], n[2], CTX.geom.light);
    }

  }
}

void Draw_NonPlane_Surface(Surface * s)
{
  if(CTX.geom.surfaces) {
    if(s->Typ == MSH_SURF_NURBS) {
      if(CTX.geom.light) glEnable(GL_LIGHTING);
      if(CTX.polygon_offset) glEnable(GL_POLYGON_OFFSET_FILL);
      GLUnurbsObj *nurb;
      nurb = gluNewNurbsRenderer();
#if defined(GLU_VERSION_1_3)
      gluNurbsProperty(nurb, (GLenum) GLU_SAMPLING_TOLERANCE, 50.0);
      gluNurbsProperty(nurb, (GLenum) GLU_DISPLAY_MODE, GLU_FILL);
#endif
      gluBeginSurface(nurb);
      gluNurbsSurface(nurb, s->Nu + s->OrderU + 1, s->ku,
		      s->Nv + s->OrderV + 1, s->kv, 4, 4 * s->Nu, s->cp,
		      s->OrderU + 1, s->OrderV + 1, GL_MAP2_VERTEX_4);
      gluEndSurface(nurb);
      gluDeleteNurbsRenderer(nurb);
      glDisable(GL_POLYGON_OFFSET_FILL);
      glDisable(GL_LIGHTING);
    }
    else{
      glEnable(GL_LINE_STIPPLE);
      glLineStipple(1, 0x1F1F);
      gl2psEnable(GL2PS_LINE_STIPPLE);
      int N = 50;
      glBegin(GL_LINE_STRIP);
      for(int i = 0; i < N + 1; i++) {
	double u = (double)i / (double)N;
	Vertex v = InterpolateSurface(s, u, 0.5, 0, 0);
	glVertex3d(v.Pos.X, v.Pos.Y, v.Pos.Z);
      }
      glEnd();
      glBegin(GL_LINE_STRIP);
      for(int i = 0; i < N + 1; i++) {
	double u = (double)i / (double)N;
	Vertex v = InterpolateSurface(s, 0.5, u, 0, 0);
	glVertex3d(v.Pos.X, v.Pos.Y, v.Pos.Z);
      }
      glEnd();
      glDisable(GL_LINE_STIPPLE);
      gl2psDisable(GL2PS_LINE_STIPPLE);
    }
  }

  if(CTX.geom.surfaces_num) {
    Vertex v = InterpolateSurface(s, 0.5, 0.5, 0, 0);
    char Num[100];
    sprintf(Num, "%d", s->Num);
    double offset = 0.3 * CTX.gl_fontsize * CTX.pixel_equiv_x;
    glRasterPos3d(v.Pos.X + offset / CTX.s[0],
		  v.Pos.Y + offset / CTX.s[1],
		  v.Pos.Z + offset / CTX.s[2]);
    Draw_String(Num);
  }

  if(CTX.geom.normals) {
    const double eps = 1.e-3;
    Vertex v1 = InterpolateSurface(s, 0.5, 0.5, 0, 0);
    Vertex v2 = InterpolateSurface(s, 0.5 + eps, 0.5, 0, 0);
    Vertex v3 = InterpolateSurface(s, 0.5, 0.5 + eps, 0, 0);
    double n[3];
    normal3points(v1.Pos.X, v1.Pos.Y, v1.Pos.Z, 
		  v2.Pos.X, v2.Pos.Y, v2.Pos.Z, 
		  v3.Pos.X, v3.Pos.Y, v3.Pos.Z, n);
    n[0] *= CTX.geom.normals * CTX.pixel_equiv_x / CTX.s[0];
    n[1] *= CTX.geom.normals * CTX.pixel_equiv_x / CTX.s[1];
    n[2] *= CTX.geom.normals * CTX.pixel_equiv_x / CTX.s[2];
    glColor4ubv((GLubyte *) & CTX.color.geom.normals);
    Draw_Vector(CTX.vector_type, 0, CTX.arrow_rel_head_radius, 
		CTX.arrow_rel_stem_length, CTX.arrow_rel_stem_radius,
		v1.Pos.X, v1.Pos.Y, v1.Pos.Z, n[0], n[1], n[2],
		CTX.geom.light);
  }
}

void Draw_Surface(void *a, void *b)
{
  Surface *s = *(Surface **) a;

  if(!s || !s->Support || !(s->Visible & VIS_GEOM))
    return;

  if(CTX.render_mode == GMSH_SELECT) {
    glPushName(2);
    glPushName(s->Num);
  }

  if(s->ipar[4] > 0) {
    glLineWidth(CTX.geom.line_sel_width / 2.);
    gl2psLineWidth(CTX.geom.line_sel_width / 2. *
		   CTX.print.eps_line_width_factor);
    glColor4ubv((GLubyte *) & CTX.color.geom.surface_sel);
  }
  else {
    glLineWidth(CTX.geom.line_width / 2.);
    gl2psLineWidth(CTX.geom.line_width / 2. * CTX.print.eps_line_width_factor);
    glColor4ubv((GLubyte *) & CTX.color.geom.surface);
  }

  if(THEM->bds){
    Draw_Polygonal_Surface(s);
  }
  else if(s->Typ == MSH_SURF_DISCRETE){
    // do nothing: we draw the elements in the mesh drawing routines
  }
  else if(s->Typ == MSH_SURF_PLAN){
    Draw_Plane_Surface(s);
  }
  else{
    Draw_NonPlane_Surface(s);
  }

  if(CTX.render_mode == GMSH_SELECT) {
    glPopName();
    glPopName();
  }
}

// Volumes

void DrawVolumes(Mesh * m)
{
}

// Draw geometry

void Draw_Geom(Mesh * m)
{
  if(m->status == -1)
    return;

  if(CTX.geom.points || CTX.geom.points_num)
    Tree_Action(m->Points, Draw_Geo_Point);
  if(CTX.geom.lines || CTX.geom.lines_num || CTX.geom.tangents)
    Tree_Action(m->Curves, Draw_Curve);
  if(CTX.geom.surfaces || CTX.geom.surfaces_num || CTX.geom.normals)
    Tree_Action(m->Surfaces, Draw_Surface);
  if(CTX.geom.volumes || CTX.geom.volumes_num)
    DrawVolumes(m);
}

// Highlight routines

void HighlightEntity(Vertex * v, Curve * c, Surface * s, int permanent)
{
  Curve *cc;
  char Message[256], temp[256];

  // Note: in Gmsh < 1.61, we used to draw permanent highlights (the
  // "red" selected lines, etc.) using incremental drawing, i.e., by
  // drawing "over" the current picture in-between Draw() calls. This
  // was fine for simple overlays on points and lines (that we could
  // draw with a slightly larger width so that they would cover the
  // old ones), but it does not work well when drawing surfaces,
  // post-pro views, etc. And since real cross-platform overlays are
  // unmanageable, the best solution is actually to redraw the whole
  // scene. This is the approach we follow now.

  if(v) {
    if(permanent){
      v->Frozen = 1;
    }
    else{
      Msg(STATUS1N, "Point %d {%.5g,%.5g,%.5g} (%.5g)", v->Num, v->Pos.X,
	  v->Pos.Y, v->Pos.Z, v->lc);
    }
  }
  else if(c) {
    if(permanent){
      c->ipar[3] = 2;
    }
    else{
      if(c->beg && c->end)
	Msg(STATUS1N, "Curve %d {%d->%d}", c->Num, c->beg->Num, c->end->Num);
      else
	Msg(STATUS1N, "Curve %d", c->Num);
    }
  }
  else if(s) {
    if(permanent){
      s->ipar[4] = 2;
    }
    else{
      int nbg = List_Nbr(s->Generatrices);
      if(nbg){
	sprintf(Message, "Surface %d {", s->Num);
	if(nbg < 10) {
	  for(int i = 0; i < nbg; i++) {
	    List_Read(s->Generatrices, i, &cc);
	    if(!i)
	      sprintf(temp, "%d", cc->Num);
	    else
	      sprintf(temp, ",%d", cc->Num);
	    strcat(Message, temp);
	  }
	}
	else {
	  strcat(Message, "...");
	}
	strcat(Message, "}");
	Msg(STATUS1N, Message);
      }
      else if (THEM->bds){
	BDS_GeomEntity *g = THEM->bds->get_geom(s->Num, 2);
	if (g && g->surf)
	  Msg(STATUS1N, "Surface %d (%s)", s->Num,g->surf->nameOf().c_str());
	else
	  Msg(STATUS1N, "Surface %d (unknown type)", s->Num);
      }
      else{
	Msg(STATUS1N, "Surface %d", s->Num);
      }
    }
  }
  else{
    if(!permanent)
      Msg(STATUS1N, " ");
  }

}

void HighlightEntityNum(int v, int c, int s, int permanent)
{
  if(v) {
    Vertex *pv = FindPoint(v, THEM);
    if(pv)
      HighlightEntity(pv, NULL, NULL, permanent);
  }
  if(c) {
    Curve *pc = FindCurve(c, THEM);
    if(pc)
      HighlightEntity(NULL, pc, NULL, permanent);
  }
  if(s) {
    Surface *ps = FindSurface(s, THEM);
    if(ps)
      HighlightEntity(NULL, NULL, ps, permanent);
  }
}

void ZeroHighlightPoint(void *a, void *b)
{
  Vertex *v = *(Vertex **) a;
  v->Frozen = 0;
}

void ZeroHighlightCurve(void *a, void *b)
{
  Curve *c = *(Curve **) a;
  c->ipar[3] = -2;
}

void ZeroHighlightSurface(void *a, void *b)
{
  Surface *s = *(Surface **) a;
  s->ipar[4] = -2;
}

void ZeroHighlight(Mesh * m)
{
  Tree_Action(m->Points, ZeroHighlightPoint);
  Tree_Action(m->Curves, ZeroHighlightCurve);
  Tree_Action(m->Surfaces, ZeroHighlightSurface);
}

void ZeroHighlightEntity(Vertex * v, Curve * c, Surface * s)
{
  if(v) ZeroHighlightPoint(&v, NULL);
  if(c) ZeroHighlightCurve(&c, NULL);
  if(s) ZeroHighlightSurface(&s, NULL);
}

void ZeroHighlightEntityNum(int v, int c, int s)
{
  if(v) {
    Vertex *pv = FindPoint(v, THEM);
    if(pv)
      ZeroHighlightPoint(&pv, NULL);
  }
  if(c) {
    Curve *pc = FindCurve(c, THEM);
    if(pc)
      ZeroHighlightCurve(&pc, NULL);
  }
  if(s) {
    Surface *ps = FindSurface(s, THEM);
    if(ps)
      ZeroHighlightSurface(&ps, NULL);
  }
}
