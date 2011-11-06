/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkProcessIdScalars.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkProcessIdScalars - Sets cell or point scalars to the processor rank.
//
// .SECTION Description
// vtkProcessIdScalars is meant to display which processor owns which cells
// and points.  It is useful for visualizing the partitioning for
// streaming or distributed pipelines.
//
// .SECTION See Also
// vtkPolyDataStreamer

#ifndef __vtkProcessIdScalars_h
#define __vtkProcessIdScalars_h

#include "vtkDataSetAlgorithm.h"

class vtkFloatArray;
class vtkIntArray;
class vtkMultiProcessController;

class VTK_PARALLEL_EXPORT vtkProcessIdScalars : public vtkDataSetAlgorithm
{
public:
  static vtkProcessIdScalars *New();

  vtkTypeMacro(vtkProcessIdScalars,vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Option to centerate cell scalars of points scalars.  Default is point
  // scalars.
  void SetScalarModeToCellData() {this->SetCellScalarsFlag(1);}
  void SetScalarModeToPointData() {this->SetCellScalarsFlag(0);}
  int GetScalarMode() {return this->CellScalarsFlag;}

  // Dscription:
  // This option uses a random mapping between pieces and scalar values.
  // The scalar values are choosen between 0 and 1.  By default, random
  // mode is off.
  vtkSetMacro(RandomMode, int);
  vtkGetMacro(RandomMode, int);
  vtkBooleanMacro(RandomMode, int);

  // Description:
  // By defualt this filter uses the global controller,
  // but this method can be used to set another instead.
  virtual void SetController(vtkMultiProcessController*);
  vtkGetObjectMacro(Controller, vtkMultiProcessController);
  
  
protected:
  vtkProcessIdScalars();
  ~vtkProcessIdScalars();
  
  // Append the pieces.
  int RequestData(
    vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  
  vtkIntArray *MakeProcessIdScalars(int piece, vtkIdType numScalars);
  vtkFloatArray *MakeRandomScalars(int piece, vtkIdType numScalars);
  
  vtkSetMacro(CellScalarsFlag,int);
  int CellScalarsFlag;
  int RandomMode;

  vtkMultiProcessController* Controller;

private:
  vtkProcessIdScalars(const vtkProcessIdScalars&);  // Not implemented.
  void operator=(const vtkProcessIdScalars&);  // Not implemented.
};

#endif
