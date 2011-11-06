/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCollectTable.h

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
// .NAME vtkCollectTable - Collect distributed table.
// .SECTION Description
// This filter has code to collect a table from across processes onto node 0.
// Collection can be turned on or off using the "PassThrough" flag.


#ifndef __vtkCollectTable_h
#define __vtkCollectTable_h

#include "vtkTableAlgorithm.h"

class vtkMultiProcessController;
class vtkSocketController;

class VTK_PARALLEL_EXPORT vtkCollectTable : public vtkTableAlgorithm
{
public:
  static vtkCollectTable *New();
  vtkTypeMacro(vtkCollectTable, vtkTableAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // By defualt this filter uses the global controller,
  // but this method can be used to set another instead.
  virtual void SetController(vtkMultiProcessController*);
  vtkGetObjectMacro(Controller, vtkMultiProcessController);

  // Description:
  // When this filter is being used in client-server mode,
  // this is the controller used to communicate between
  // client and server.  Client should not set the other controller.
  virtual void SetSocketController(vtkSocketController*);
  vtkGetObjectMacro(SocketController, vtkSocketController);

  // Description:
  // To collect or just copy input to output. Off (collect) by default.
  vtkSetMacro(PassThrough, int);
  vtkGetMacro(PassThrough, int);
  vtkBooleanMacro(PassThrough, int);

protected:
  vtkCollectTable();
  ~vtkCollectTable();

  int PassThrough;

  // Data generation method
  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  vtkMultiProcessController *Controller;
  vtkSocketController *SocketController;

private:
  vtkCollectTable(const vtkCollectTable&); // Not implemented
  void operator=(const vtkCollectTable&); // Not implemented
};

#endif
