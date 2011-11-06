/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtk_freetype.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __vtk_freetype_h
#define __vtk_freetype_h

/* Use the freetype library configured for VTK.  */
#include "vtkToolkits.h"

#ifdef VTK_USE_SYSTEM_FREETYPE
# include <ft2build.h>
#else
# include <vtkfreetype/include/ft2build.h>
#  if defined(VTKFREETYPE)
#    include "vtkFreeTypeConfig.h"
#  endif
#endif

#endif // #ifndef __vtk_freetype_h
