/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPMultiCorrelativeStatistics.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPMultiCorrelativeStatistics - A class for parallel bivariate correlative statistics
// .SECTION Description
// vtkPMultiCorrelativeStatistics is vtkMultiCorrelativeStatistics subclass for parallel datasets.
// It learns and derives the global statistical model on each node, but assesses each 
// individual data points on the node that owns it.

// .SECTION Thanks
// Thanks to Philippe Pebay and David Thompson from Sandia National Laboratories for implementing this class.

#ifndef __vtkPMultiCorrelativeStatistics_h
#define __vtkPMultiCorrelativeStatistics_h

#include "vtkMultiCorrelativeStatistics.h"

class vtkMultiProcessController;

class VTK_INFOVIS_EXPORT vtkPMultiCorrelativeStatistics : public vtkMultiCorrelativeStatistics
{
public:
  static vtkPMultiCorrelativeStatistics* New();
  vtkTypeMacro(vtkPMultiCorrelativeStatistics, vtkMultiCorrelativeStatistics);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the multiprocess controller. If no controller is set,
  // single process is assumed.
  virtual void SetController(vtkMultiProcessController*);
  vtkGetObjectMacro(Controller, vtkMultiProcessController);

  // Description:
  // Performs Reduction
  static void GatherStatistics( vtkMultiProcessController *curController,
                                vtkTable *sparseCov );

protected:
  vtkPMultiCorrelativeStatistics();
  ~vtkPMultiCorrelativeStatistics();

  vtkMultiProcessController* Controller;

  // Execute the parallel calculations required by the Learn option.
  virtual void Learn( vtkTable* inData,
                      vtkTable* inParameters,
                      vtkMultiBlockDataSet* outMeta );


private:
  vtkPMultiCorrelativeStatistics(const vtkPMultiCorrelativeStatistics&); // Not implemented.
  void operator=(const vtkPMultiCorrelativeStatistics&); // Not implemented.
};

#endif

