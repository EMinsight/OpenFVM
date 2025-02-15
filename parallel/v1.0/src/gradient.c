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

#include "variables.h"
#include "vector.h"

#include "mesh.h"

#include "geocalc.h"
#include "globals.h"
#include "bcond.h"
#include "gradient.h"

msh_vector
Gradient (Vec * phi, Vec * phif, int bound, int element)
{

  int j;

  int neighbor, face, pair;

  msh_element ghost;

  double phij;

  //double dNf, dPf;
  double lambda;

  double v1, v2;
  double v1min, v1max, v2min, v2max;

  double fv;
  msh_vector rv;

  rv.x = 0.0;
  rv.y = 0.0;
  rv.z = 0.0;

  for (j = 0; j < elements[element].nbfaces; j++)
    {

      face = elements[element].face[j];

      pair = faces[face].pair;

      if (pair != -1)
	{

	  neighbor = faces[pair].element;

	  /*
	  dNf =
	    GeoMagVector (GeoSubVectorVector
			  (elements[neighbor].celement, faces[face].cface));
	  dPf =
	    GeoMagVector (GeoSubVectorVector
			  (elements[element].celement, faces[face].cface));

	  lambda = dPf / (dPf + dNf);
	  */

	  lambda = 0.5;

	  // Element face variable

	  phij =
	    V_GetCmp (phi, neighbor) * lambda +
	    V_GetCmp (phi, element) * (1.0 - lambda);

	  // Element center gradient

	  rv.x += 1 / elements[element].Vp * phij * faces[face].A.x;
	  rv.y += 1 / elements[element].Vp * phij * faces[face].A.y;
	  rv.z += 1 / elements[element].Vp * phij * faces[face].A.z;

	}
      else
	{

	  if (faces[face].bc == PROCESSOR)
	    {

	      ghost.index = faces[face].physreg;

	      ghost.celement.x = V_GetCmp (&cexl, faces[face].ghost);
	      ghost.celement.y = V_GetCmp (&ceyl, faces[face].ghost);
	      ghost.celement.z = V_GetCmp (&cezl, faces[face].ghost);

	      /*
	      dNf =
		GeoMagVector (GeoSubVectorVector
			      (ghost.celement, faces[face].cface));

	      dPf =
		GeoMagVector (GeoSubVectorVector
			      (elements[element].celement,
			       faces[face].cface));

	      lambda = dPf / (dPf + dNf);
	      */

	      lambda = 0.5;

	      // Element face variable

	      phij =
		V_GetCmp (phi, faces[face].ghost) * lambda +
		V_GetCmp (phi, element) * (1.0 - lambda);

	      // Element center gradient

	      rv.x += 1 / elements[element].Vp * phij * faces[face].A.x;
	      rv.y += 1 / elements[element].Vp * phij * faces[face].A.y;
	      rv.z += 1 / elements[element].Vp * phij * faces[face].A.z;

	    }
	  else
	    {

	      // Element face variable

	      if (bound == LOGICAL_TRUE)
		phij = V_GetCmp (phif, face);
	      else
		phij = V_GetCmp (phi, element);

	      // Element center gradient

	      rv.x += 1 / elements[element].Vp * phij * faces[face].A.x;
	      rv.y += 1 / elements[element].Vp * phij * faces[face].A.y;
	      rv.z += 1 / elements[element].Vp * phij * faces[face].A.z;

	    }
	}

    }

  // Eliminate local extrema

  v1min = +GREAT;
  v1max = -GREAT;

  v2min = +GREAT;
  v2max = -GREAT;

  for (j = 0; j < elements[element].nbfaces; j++)
    {

      face = elements[element].face[j];

      v1 = V_GetCmp (phi, element) + rv.x * (faces[face].cface.x - elements[element].celement.x) + 
				     rv.y * (faces[face].cface.y - elements[element].celement.y) + 
				     rv.z * (faces[face].cface.z - elements[element].celement.z);

      v1min = LMIN(v1min, v1);
      v1max = LMAX(v1max, v1);

      pair = faces[face].pair;

      if (pair != -1)
	{

	  neighbor = faces[pair].element;

          v2 = (V_GetCmp (phi, element) + V_GetCmp (phi, neighbor)) * 0.5;
	  
          v2min = LMIN(v2min, v2);
          v2max = LMAX(v2max, v2);
	
        }
      else
	{

	  if (faces[face].bc == PROCESSOR)
	    {

	      v2 = (V_GetCmp (phi, element) + V_GetCmp (phi, faces[face].ghost)) * 0.5;
	  
              v2min = LMIN(v2min, v2);
              v2max = LMAX(v2max, v2);

          }
       }
    }

  fv = 1.0;

  if (LABS(v1max - v1min) > 2 * LABS(v2max - v2min)) fv = LABS(v2max - v2min) / LABS(v1max - v1min);

  rv.x *= fv;
  rv.y *= fv;
  rv.z *= fv;

  return rv;

}
