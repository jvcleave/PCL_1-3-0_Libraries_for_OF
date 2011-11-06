/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkBridgeExport.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkBridgeExport - manage Windows system differences
// .SECTION Description
// The vtkBridgeExport captures some system differences between Unix and
// Windows operating systems. 

#ifndef __vtkBridgeExport_h
#define __vtkBridgeExport_h

#include "vtkSystemIncludes.h"

#if 1
# define VTK_BRIDGE_EXPORT
#else

#if defined(WIN32) && defined(VTK_BUILD_SHARED_LIBS)

 #if defined(vtkBridge_EXPORTS)
  #define VTK_BRIDGE_EXPORT __declspec( dllexport ) 
 #else
  #define VTK_BRIDGE_EXPORT __declspec( dllimport ) 
 #endif
#else
 #define VTK_BRIDGE_EXPORT
#endif

#endif //#if 1

#endif
