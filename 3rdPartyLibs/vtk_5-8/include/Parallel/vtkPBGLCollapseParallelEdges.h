/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPBGLCollapseParallelEdges.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*----------------------------------------------------------------------------
 Copyright (c) Sandia Corporation
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
----------------------------------------------------------------------------*/
// .NAME vtkPBGLCollapseParallelEdges - collapse multiple vertices into a single vertex
//
// .SECTION Description
//
// Uses single input array specified with SetInputArrayToProcess(0,...)
// to collapse groups of vertices with the same value into a single vertex.

#ifndef __vtkPBGLCollapseParallelEdges_h
#define __vtkPBGLCollapseParallelEdges_h

#include "vtkGraphAlgorithm.h"

class VTK_PARALLEL_EXPORT vtkPBGLCollapseParallelEdges : public vtkGraphAlgorithm
{
public:
  static vtkPBGLCollapseParallelEdges* New();
  vtkTypeMacro(vtkPBGLCollapseParallelEdges,vtkGraphAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkPBGLCollapseParallelEdges();
  ~vtkPBGLCollapseParallelEdges();

  virtual int RequestData(
    vtkInformation*,
    vtkInformationVector**,
    vtkInformationVector*);

private:
  vtkPBGLCollapseParallelEdges(const vtkPBGLCollapseParallelEdges&); // Not implemented
  void operator=(const vtkPBGLCollapseParallelEdges&);   // Not implemented
};

#endif

