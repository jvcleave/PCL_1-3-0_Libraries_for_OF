/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkEarthSource.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkEarthSource - create the continents of the Earth as a sphere
// .SECTION Description
// vtkEarthSource creates a spherical rendering of the geographical shapes
// of the major continents of the earth. The OnRatio determines
// how much of the data is actually used. The radius defines the radius
// of the sphere at which the continents are placed. Obtains data from
// an imbedded array of coordinates.

#ifndef __vtkEarthSource_h
#define __vtkEarthSource_h

#include "vtkPolyDataAlgorithm.h"

class VTK_HYBRID_EXPORT vtkEarthSource : public vtkPolyDataAlgorithm 
{
public:
  static vtkEarthSource *New();
  vtkTypeMacro(vtkEarthSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set radius of earth.
  vtkSetClampMacro(Radius,double,0.0,VTK_LARGE_FLOAT);
  vtkGetMacro(Radius,double);

  // Description:
  // Turn on every nth entity. This controls how much detail the model
  // will have. The maximum ratio is sixteen. (The smaller OnRatio, the more
  // detail there is.)
  vtkSetClampMacro(OnRatio,int,1,16);
  vtkGetMacro(OnRatio,int);

  // Description:
  // Turn on/off drawing continents as filled polygons or as wireframe outlines.
  // Warning: some graphics systems will have trouble with the very large, concave 
  // filled polygons. Recommend you use OutlienOn (i.e., disable filled polygons) 
  // for now.
  vtkSetMacro(Outline,int);
  vtkGetMacro(Outline,int);
  vtkBooleanMacro(Outline,int);

protected:
  vtkEarthSource();
  ~vtkEarthSource() {};

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  double Radius;
  int OnRatio;
  int Outline;
private:
  vtkEarthSource(const vtkEarthSource&);  // Not implemented.
  void operator=(const vtkEarthSource&);  // Not implemented.
};

#endif










