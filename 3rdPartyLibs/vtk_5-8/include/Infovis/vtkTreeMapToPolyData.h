/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkTreeMapToPolyData.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/
// .NAME vtkTreeMapToPolyData - converts a tree to a polygonal data representing a tree map
//
// .SECTION Description
// This algorithm requires that the vtkTreeMapLayout filter has already applied to the
// data in order to create the quadruple array (min x, max x, min y, max y) of
// bounds for each vertex of the tree.

#ifndef __vtkTreeMapToPolyData_h
#define __vtkTreeMapToPolyData_h

#include "vtkPolyDataAlgorithm.h"

class VTK_INFOVIS_EXPORT vtkTreeMapToPolyData : public vtkPolyDataAlgorithm 
{
public:
  static vtkTreeMapToPolyData *New();
  vtkTypeMacro(vtkTreeMapToPolyData,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The field containing quadruples of the form (min x, max x, min y, max y)
  // representing the bounds of the rectangles for each vertex.
  // This array may be added to the tree using vtkTreeMapLayout.
  virtual void SetRectanglesArrayName(const char* name)
    { this->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_VERTICES, name); }

  // Description:
  // The field containing the level of each tree node.
  // This can be added using vtkTreeLevelsFilter before this filter.
  // If this is not present, the filter simply calls tree->GetLevel(v) for
  // each vertex, which will produce the same result, but
  // may not be as efficient.
  virtual void SetLevelArrayName(const char* name)
    { this->SetInputArrayToProcess(1, 0, 0, vtkDataObject::FIELD_ASSOCIATION_VERTICES, name); }

  // Description:
  // The spacing along the z-axis between tree map levels.
  vtkGetMacro(LevelDeltaZ, double);
  vtkSetMacro(LevelDeltaZ, double);

  // Description:
  // The spacing along the z-axis between tree map levels.
  vtkGetMacro(AddNormals, bool);
  vtkSetMacro(AddNormals, bool);

  int FillInputPortInformation(int port, vtkInformation* info);

protected:
  vtkTreeMapToPolyData();
  ~vtkTreeMapToPolyData();

  double LevelDeltaZ;
  bool AddNormals;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
private:
  vtkTreeMapToPolyData(const vtkTreeMapToPolyData&);  // Not implemented.
  void operator=(const vtkTreeMapToPolyData&);  // Not implemented.
};

#endif
