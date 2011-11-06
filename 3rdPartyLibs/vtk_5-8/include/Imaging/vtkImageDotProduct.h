/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageDotProduct.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageDotProduct - Dot product of two vector images.
// .SECTION Description
// vtkImageDotProduct interprets the scalar components of two images
// as vectors and takes the dot product vector by vector (pixel by pixel).

#ifndef __vtkImageDotProduct_h
#define __vtkImageDotProduct_h



#include "vtkThreadedImageAlgorithm.h"

class VTK_IMAGING_EXPORT vtkImageDotProduct : public vtkThreadedImageAlgorithm
{
public:
  static vtkImageDotProduct *New();
  vtkTypeMacro(vtkImageDotProduct,vtkThreadedImageAlgorithm);

  // Description:
  // Set the two inputs to this filter
  virtual void SetInput1(vtkDataObject *in) { this->SetInput(0,in); }
  virtual void SetInput2(vtkDataObject *in) { this->SetInput(1,in); }

protected:
  vtkImageDotProduct();
  ~vtkImageDotProduct() {};

  virtual int RequestInformation (vtkInformation *, 
                                  vtkInformationVector **,
                                  vtkInformationVector *);
  
  virtual void ThreadedRequestData(vtkInformation *request, 
                                   vtkInformationVector **inputVector, 
                                   vtkInformationVector *outputVector,
                                   vtkImageData ***inData, 
                                   vtkImageData **outData,
                                   int extent[6], int threadId);

private:
  vtkImageDotProduct(const vtkImageDotProduct&);  // Not implemented.
  void operator=(const vtkImageDotProduct&);  // Not implemented.
};

#endif



