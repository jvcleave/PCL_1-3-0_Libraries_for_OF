/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRenderedSurfaceRepresentation.h

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
// .NAME vtkRenderedSurfaceRepresentation - Displays a geometric dataset as a surface.
//
// .SECTION Description
// vtkRenderedSurfaceRepresentation is used to show a geometric dataset in a view.
// The representation uses a vtkGeometryFilter to convert the dataset to
// polygonal data (e.g. volumetric data is converted to its external surface).
// The representation may then be added to vtkRenderView.

#ifndef __vtkRenderedSurfaceRepresentation_h
#define __vtkRenderedSurfaceRepresentation_h

#include "vtkRenderedRepresentation.h"

class vtkActor;
class vtkAlgorithmOutput;
class vtkApplyColors;
class vtkDataObject;
class vtkGeometryFilter;
class vtkPolyDataMapper;
class vtkRenderView;
class vtkScalarsToColors;
class vtkSelection;
class vtkTransformFilter;
class vtkView;

class VTK_VIEWS_EXPORT vtkRenderedSurfaceRepresentation : public vtkRenderedRepresentation
{
public:
  static vtkRenderedSurfaceRepresentation *New();
  vtkTypeMacro(vtkRenderedSurfaceRepresentation, vtkRenderedRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //Description:
  //Sets the color array name
  virtual void SetCellColorArrayName(const char* arrayName);
  virtual const char* GetCellColorArrayName()
    { return this->GetCellColorArrayNameInternal(); }

  // Description:
  // Apply a theme to this representation.
  virtual void ApplyViewTheme(vtkViewTheme* theme);

protected:
  vtkRenderedSurfaceRepresentation();
  ~vtkRenderedSurfaceRepresentation();

  // Description:
  // Sets the input pipeline connection to this representation.
  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector);
  
  // Description:
  // Performs per-render operations.
  virtual void PrepareForRendering(vtkRenderView* view);
  
  // Description:
  // Adds the representation to the view.  This is called from
  // vtkView::AddRepresentation().
  virtual bool AddToView(vtkView* view);
  
  // Description:
  // Removes the representation to the view.  This is called from
  // vtkView::RemoveRepresentation().
  virtual bool RemoveFromView(vtkView* view);
  
  // Description:
  // Convert the selection to a type appropriate for sharing with other
  // representations through vtkAnnotationLink.
  // If the selection cannot be applied to this representation, returns NULL.
  virtual vtkSelection* ConvertSelection(vtkView* view, vtkSelection* selection);
  
  // Description:
  // Internal pipeline objects.
  vtkTransformFilter*   TransformFilter;
  vtkApplyColors*       ApplyColors;
  vtkGeometryFilter*    GeometryFilter;
  vtkPolyDataMapper*    Mapper;
  vtkActor*             Actor;

  vtkGetStringMacro(CellColorArrayNameInternal);
  vtkSetStringMacro(CellColorArrayNameInternal);
  char* CellColorArrayNameInternal;

private:
  vtkRenderedSurfaceRepresentation(const vtkRenderedSurfaceRepresentation&);  // Not implemented.
  void operator=(const vtkRenderedSurfaceRepresentation&);  // Not implemented.
};

#endif
