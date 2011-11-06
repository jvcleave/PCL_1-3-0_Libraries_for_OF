/*
** libproj -- library of cartographic projections
**
** Copyright (c) 2003, 2006   Gerald I. Evenden
*/
static const char
LIBPROJ_ID[] = "Id";
/*
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#define PROJ_LIB__
# include  <lib_proj.h>
PROJ_HEAD(eck2, "Eckert II") "\n\tPCyl. Sph.";
#define FXC  0.46065886596178063902
#define FYC  1.44720250911653531871
#define C13  0.33333333333333333333
#define ONEEPS  1.0000001
FORWARD(s_forward); /* spheroid */
  (void) P; /* avoid warning */
  xy.x = FXC * lp.lam * (xy.y = sqrt(4. - 3. * sin(fabs(lp.phi))));
  xy.y = FYC * (2. - xy.y);
  if ( lp.phi < 0.) xy.y = -xy.y;
  return (xy);
}
INVERSE(s_inverse); /* spheroid */
  (void) P; /* avoid warning */
  lp.lam = xy.x / (FXC * ( lp.phi = 2. - fabs(xy.y) / FYC) );
  lp.phi = (4. - lp.phi * lp.phi) * C13;
  if (fabs(lp.phi) >= 1.) {
    if (fabs(lp.phi) > ONEEPS)  I_ERROR
    else
      lp.phi = lp.phi < 0. ? -HALFPI : HALFPI;
  } else
    lp.phi = asin(lp.phi);
  if (xy.y < 0)
    lp.phi = -lp.phi;
  return (lp);
}
FREEUP; if (P) free(P); }
ENTRY0(eck2); P->es = 0.; P->inv = s_inverse; P->fwd = s_forward; ENDENTRY(P)
/*
** Log: proj_eck2.c
** Revision 3.1  2006/01/11 01:38:18  gie
** Initial
**
*/
