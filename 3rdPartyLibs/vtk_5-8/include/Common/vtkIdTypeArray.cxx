/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkIdTypeArray.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// We never need to instantiate vtkDataArrayTemplate<vtkIdType> or
// vtkArrayIteratorTemplate<vtkIdType> because they are instantiated
// by the corresponding array for its native type.  Therefore this
// code should not be uncommented and is here for reference:
//   #include "vtkDataArrayTemplate.txx"
//   VTK_DATA_ARRAY_TEMPLATE_INSTANTIATE(vtkIdType);
//   #include "vtkArrayIteratorTemplate.txx"
//   VTK_ARRAY_ITERATOR_TEMPLATE_INSTANTIATE(vtkIdType);

#define __vtkIdTypeArray_cxx
#include "vtkIdTypeArray.h"

#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkIdTypeArray);

//----------------------------------------------------------------------------
vtkIdTypeArray::vtkIdTypeArray(vtkIdType numComp): RealSuperclass(numComp)
{
}

//----------------------------------------------------------------------------
vtkIdTypeArray::~vtkIdTypeArray()
{
}

//----------------------------------------------------------------------------
void vtkIdTypeArray::PrintSelf(ostream& os, vtkIndent indent)
{
  this->RealSuperclass::PrintSelf(os,indent);
}
