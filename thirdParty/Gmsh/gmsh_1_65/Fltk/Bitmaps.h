#ifndef _BITMAPS_H_
#define _BITMAPS_H_

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

// Standard Gmsh Unix icon

#if !defined(WIN32) && !defined(__APPLE__)
#define gmsh32x32_width 32
#define gmsh32x32_height 32
static char gmsh32x32_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x40, 0x03, 0x00,
   0x00, 0x40, 0x03, 0x00, 0x00, 0x20, 0x07, 0x00, 0x00, 0x20, 0x07, 0x00,
   0x00, 0x10, 0x0f, 0x00, 0x00, 0x10, 0x0f, 0x00, 0x00, 0x08, 0x1f, 0x00,
   0x00, 0x08, 0x1f, 0x00, 0x00, 0x04, 0x3f, 0x00, 0x00, 0x04, 0x3f, 0x00,
   0x00, 0x02, 0x7f, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x01, 0xff, 0x00,
   0x00, 0x01, 0xff, 0x00, 0x80, 0x00, 0xff, 0x01, 0x80, 0x00, 0xff, 0x01,
   0x40, 0x00, 0xff, 0x03, 0x40, 0x00, 0xff, 0x03, 0x20, 0x00, 0xff, 0x07,
   0x20, 0x00, 0xff, 0x07, 0x10, 0x00, 0xff, 0x0f, 0x10, 0x00, 0xff, 0x0f,
   0x08, 0x00, 0xff, 0x1f, 0x08, 0x00, 0xff, 0x1f, 0x04, 0x40, 0xfd, 0x3f,
   0x04, 0xa8, 0xea, 0x3f, 0x02, 0x55, 0x55, 0x7f, 0xa2, 0xaa, 0xaa, 0x7a,
   0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 };
#endif

// 'Abort' bitmap
// disabled until the mesh thread is back
/* 
#define abort_width 13
#define abort_height 13
static char abort_bits[] = {
 0x00,0xe0,0x40,0xe0,0x40,0xe0,0x50,0xe1,0x48,0xe2,0x44,0xe4,0x44,0xe4,0x44,
 0xe4,0x04,0xe4,0x04,0xe4,0x08,0xe2,0xf0,0xe1,0x00,0xe0};
*/

// 'Play button' bitmap

#define start_width 9
#define start_height 13
static char start_bits[] = {
 0x00,0xfe,0x06,0xfe,0x0a,0xfe,0x12,0xfe,0x22,0xfe,0x42,0xfe,0x82,0xfe,0x42,
 0xfe,0x22,0xfe,0x12,0xfe,0x0a,0xfe,0x06,0xfe,0x00,0xfe};

// 'Pause button' bitmap

#define stop_width 9
#define stop_height 13
static char stop_bits[] = {
 0x00,0xfe,0xee,0xfe,0xaa,0xfe,0xaa,0xfe,0xaa,0xfe,0xaa,0xfe,0xaa,0xfe,0xaa,
 0xfe,0xaa,0xfe,0xaa,0xfe,0xaa,0xfe,0xee,0xfe,0x00,0xfe};

// 'Rewind button' bitmap

#define rewind_width 12
#define rewind_height 13
static char rewind_bits[] = {
 0x00,0xf0,0x0e,0xf6,0x0a,0xf5,0x8a,0xf4,0x4a,0xf4,0x2a,0xf4,0x1a,0xf4,0x2a,
 0xf4,0x4a,0xf4,0x8a,0xf4,0x0a,0xf5,0x0e,0xf6,0x00,0xf0};

// 'Orthographic projection' bitmap
#define ortho_width 13
#define ortho_height 13
static unsigned char ortho_bits[] = {
   0x00, 0x00, 0xf0, 0x0f, 0x18, 0x0c, 0x14, 0x0a, 0xfe, 0x09, 0x12, 0x09,
   0x12, 0x09, 0x12, 0x09, 0xf2, 0x0f, 0x0a, 0x05, 0x06, 0x03, 0xfe, 0x01,
   0x00, 0x00 };

// '90 degrees rotation' bitmap
#define rotate_width 12
#define rotate_height 13
static unsigned char rotate_bits[] = {
   0x00, 0x00, 0xf0, 0x00, 0x08, 0x01, 0x04, 0x02, 0x02, 0x04, 0x02, 0x04,
   0x02, 0x04, 0x02, 0x00, 0x24, 0x00, 0x68, 0x00, 0xf0, 0x00, 0x60, 0x00,
   0x20, 0x00 };

#endif