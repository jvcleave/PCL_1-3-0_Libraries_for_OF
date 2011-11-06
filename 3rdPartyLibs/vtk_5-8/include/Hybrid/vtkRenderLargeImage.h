/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRenderLargeImage.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkRenderLargeImage - Use tiling to generate a large rendering
// .SECTION Description
// vtkRenderLargeImage provides methods needed to read a region from a file.


#ifndef __vtkRenderLargeImage_h
#define __vtkRenderLargeImage_h

#include "vtkAlgorithm.h"
#include "vtkImageData.h" // makes things a bit easier

class vtkRenderer;
class vtkActor2DCollection;
class vtkCollection;
class vtkRenderLargeImage2DHelperClass;

class VTK_HYBRID_EXPORT vtkRenderLargeImage : public vtkAlgorithm
{
public:
  static vtkRenderLargeImage *New();
  vtkTypeMacro(vtkRenderLargeImage,vtkAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);   

  // Description:
  // The magnification of the current render window
  vtkSetMacro(Magnification,int);
  vtkGetMacro(Magnification,int);

  // Description:
  // Indicates what renderer to get the pixel data from.
  virtual void SetInput(vtkRenderer*);

  // Description:
  // Returns which renderer is being used as the source for the pixel data.
  vtkGetObjectMacro(Input,vtkRenderer);

  // Description:
  // Get the output data object for a port on this algorithm.
  vtkImageData* GetOutput();

  // Description:
  // see vtkAlgorithm for details
  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*);

protected:
  vtkRenderLargeImage();
  ~vtkRenderLargeImage();

  int Magnification;
  vtkRenderer *Input;
  void RequestData(vtkInformation *, 
                   vtkInformationVector **, vtkInformationVector *);
  void RequestInformation (vtkInformation *, 
                           vtkInformationVector **, vtkInformationVector *);  

  // see algorithm for more info
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  // Adjust the coordinates of all 2D actors to fit new window size
  void Rescale2DActors();
  // Shift each actor according to the tile we are rendering
  void Shift2DActors(int x, int y);
  // put them all back to their previous state when finished.
  void Restore2DActors();
  // 2D Actors need to be rescaled and shifted about for each tile
  // use this helper class to make life easier.
  vtkRenderLargeImage2DHelperClass *StoredData;

private:
  vtkRenderLargeImage(const vtkRenderLargeImage&);  // Not implemented.
  void operator=(const vtkRenderLargeImage&);  // Not implemented.
};

#endif
