/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPointSource.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPointSource.h"

#include "vtkCellArray.h"
#include "vtkMath.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

#include <float.h>
#include <math.h>

vtkStandardNewMacro(vtkPointSource);

//----------------------------------------------------------------------------
vtkPointSource::vtkPointSource(vtkIdType numPts)
{
  this->NumberOfPoints = (numPts > 0 ? numPts : 10);

  this->Center[0] = 0.0;
  this->Center[1] = 0.0;
  this->Center[2] = 0.0;

  this->Radius = 0.5;

  this->Distribution = VTK_POINT_UNIFORM;

  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
int vtkPointSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkIdType i;
  double theta, rho, cosphi, sinphi, radius;
  double x[3];
  vtkPoints *newPoints;
  vtkCellArray *newVerts;

  newPoints = vtkPoints::New();
  newPoints->Allocate(this->NumberOfPoints);
  newVerts = vtkCellArray::New();
  newVerts->Allocate(newVerts->EstimateSize(1,this->NumberOfPoints));

  newVerts->InsertNextCell(this->NumberOfPoints);

  if (this->Distribution == VTK_POINT_SHELL)
    {  // only produce points on the surface of the sphere
    for (i=0; i<this->NumberOfPoints; i++)
      {
      cosphi = 1 - 2*vtkMath::Random();
      sinphi = sqrt(1 - cosphi*cosphi);
      radius = this->Radius * sinphi;
      theta = 6.2831853 * vtkMath::Random();
      x[0] = this->Center[0] + radius*cos(theta);
      x[1] = this->Center[1] + radius*sin(theta);
      x[2] = this->Center[2] + this->Radius*cosphi;
      newVerts->InsertCellPoint(newPoints->InsertNextPoint(x));
      }
    }
  else
    { // uniform distribution throughout the sphere volume
    for (i=0; i<this->NumberOfPoints; i++)
      {
      cosphi = 1 - 2*vtkMath::Random();
      sinphi = sqrt(1 - cosphi*cosphi);
      rho = this->Radius*pow(vtkMath::Random(),0.33333333);
      radius = rho * sinphi;
      theta = 6.2831853 * vtkMath::Random();
      x[0] = this->Center[0] + radius*cos(theta);
      x[1] = this->Center[1] + radius*sin(theta);
      x[2] = this->Center[2] + rho*cosphi;
      newVerts->InsertCellPoint(newPoints->InsertNextPoint(x));
      }
    }
   //
   // Update ourselves and release memory
   //
  output->SetPoints(newPoints);
  newPoints->Delete();

  output->SetVerts(newVerts);
  newVerts->Delete();

  return 1;
}

//----------------------------------------------------------------------------
void vtkPointSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Number Of Points: " << this->NumberOfPoints << "\n";
  os << indent << "Radius: " << this->Radius << "\n";
  os << indent << "Center: (" << this->Center[0] << ", "
                              << this->Center[1] << ", "
                              << this->Center[2] << ")\n";
  os << indent << "Distribution: " << 
     ((this->Distribution == VTK_POINT_SHELL) ? "Shell\n" : "Uniform\n");
}
