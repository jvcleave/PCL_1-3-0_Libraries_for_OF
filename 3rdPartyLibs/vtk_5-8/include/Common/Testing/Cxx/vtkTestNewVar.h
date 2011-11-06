/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkTestNewVar.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME Test of vtkNew.
// .SECTION Description
// Tests instantiations of the vtkNew class template.

#ifndef __vtkTestNewVar_h
#define __vtkTestNewVar_h

#include "vtkObject.h"
#include "vtkNew.h"

class vtkPoints2D;

class vtkTestNewVar : public vtkObject
{
public:
  static vtkTestNewVar * New();

  vtkTypeMacro(vtkTestNewVar, vtkObject)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the reference count for the points object.
  vtkIdType GetPointsRefCount();

  // Description:
  // This is just for testing - return the points as a vtkObject so that it can
  // be assigned to a vtkSmartPointer without including the vtkPoints2D header
  // and defeating part of the point of the test.
  vtkObject * GetPoints();

protected:
  vtkTestNewVar();
  ~vtkTestNewVar();

  vtkNew<vtkPoints2D> Points;

private:
  vtkTestNewVar(const vtkTestNewVar&);  // Not implemented.
  void operator=(const vtkTestNewVar&);  // Not implemented.
};

#endif
