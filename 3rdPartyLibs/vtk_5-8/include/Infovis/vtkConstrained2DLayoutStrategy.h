/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkConstrained2DLayoutStrategy.h

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
// .NAME vtkConstrained2DLayoutStrategy - a simple fast 2D graph layout
// that looks for a 'constraint' array (vtkDoubleArray). Any entry in the 
// constraint array will indicate the level of impedence a node has to 
// the force calculations during the layout optimization. The array is 
// assumed to be normalized between zero and one, with one being totally 
// constrained, so no force will be applied to the node (i.e. no movement), 
// and zero being full range of movement (no constraints). 
//
// .SECTION Description
// This class is a density grid based force directed layout strategy.
// Also please note that 'fast' is relative to quite slow. :)
// The layout running time is O(V+E) with an extremely high constant.
// .SECTION Thanks
// We would like to thank Mothra for distracting Godzilla while we
// wrote this class. 

#ifndef __vtkConstrained2DLayoutStrategy_h
#define __vtkConstrained2DLayoutStrategy_h

#include "vtkGraphLayoutStrategy.h"

#include "vtkSmartPointer.h"    // Required for smart pointer internal ivars.

class vtkFastSplatter;
class vtkImageData;
class vtkFloatArray;

class VTK_INFOVIS_EXPORT vtkConstrained2DLayoutStrategy : public vtkGraphLayoutStrategy 
{
public:
  static vtkConstrained2DLayoutStrategy *New();

  vtkTypeMacro(vtkConstrained2DLayoutStrategy, vtkGraphLayoutStrategy);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Seed the random number generator used to jitter point positions.
  // This has a significant effect on their final positions when
  // the layout is complete.
  vtkSetClampMacro(RandomSeed, int, 0, VTK_LARGE_INTEGER);
  vtkGetMacro(RandomSeed, int);

  // Description:
  // Set/Get the maximum number of iterations to be used.
  // The higher this number, the more iterations through the algorithm
  // is possible, and thus, the more the graph gets modified.
  // The default is '100' for no particular reason
  // Note: The strong recommendation is that you do not change
  // this parameter. :)
  vtkSetClampMacro(MaxNumberOfIterations, int, 0, VTK_LARGE_INTEGER);
  vtkGetMacro(MaxNumberOfIterations, int);
  
  // Description:
  // Set/Get the number of iterations per layout.
  // The only use for this ivar is for the application
  // to do visualizations of the layout before it's complete.
  // The default is '100' to match the default 'MaxNumberOfIterations'
  // Note: Changing this parameter is just fine :)
  vtkSetClampMacro(IterationsPerLayout, int, 0, VTK_LARGE_INTEGER);
  vtkGetMacro(IterationsPerLayout, int);

  // Description:
  // Set the initial temperature.  The temperature default is '5'
  // for no particular reason
  // Note: The strong recommendation is that you do not change
  // this parameter. :)
  vtkSetClampMacro(InitialTemperature, float, 0.0, VTK_FLOAT_MAX);
  vtkGetMacro(InitialTemperature, float);
  
  // Description:
  // Set/Get the Cool-down rate.
  // The higher this number is, the longer it will take to "cool-down",
  // and thus, the more the graph will be modified. The default is '10'
  // for no particular reason. 
  // Note: The strong recommendation is that you do not change
  // this parameter. :)
  vtkSetClampMacro(CoolDownRate, double, 0.01, VTK_DOUBLE_MAX);
  vtkGetMacro(CoolDownRate, double);
  
  
  // Description:
  // Manually set the resting distance. Otherwise the
  // distance is computed automatically.
  vtkSetMacro(RestDistance, float);
  vtkGetMacro(RestDistance, float);

  // Description:
  // This strategy sets up some data structures
  // for faster processing of each Layout() call
  virtual void Initialize();
  
  // Description:
  // This is the layout method where the graph that was
  // set in SetGraph() is laid out. The method can either
  // entirely layout the graph or iteratively lay out the
  // graph. If you have an iterative layout please implement
  // the IsLayoutComplete() method.
  virtual void Layout();
  
  // Description:
  // I'm an iterative layout so this method lets the caller
  // know if I'm done laying out the graph
  virtual int IsLayoutComplete() {return this->LayoutComplete;}
  
  // Description:
  // Set/Get the input constraint array name. If no input array 
  // name is set then the name 'constraint' is used.
  vtkSetStringMacro(InputArrayName);
  vtkGetStringMacro(InputArrayName);

protected:
  vtkConstrained2DLayoutStrategy();
  ~vtkConstrained2DLayoutStrategy();

  int    MaxNumberOfIterations;  //Maximum number of iterations.
  float  InitialTemperature;
  float  CoolDownRate;  //Cool-down rate.  Note:  Higher # = Slower rate.

private:

  //BTX

  // An edge consists of two vertices joined together.
  // This struct acts as a "pointer" to those two vertices.
  typedef struct 
  {
    vtkIdType from;
    vtkIdType to;
    float weight;
  } vtkLayoutEdge;
  
  // This class 'has a' vtkFastSplatter for the density grid
  vtkSmartPointer<vtkFastSplatter>        DensityGrid;
  vtkSmartPointer<vtkImageData>           SplatImage;
  vtkSmartPointer<vtkFloatArray>          RepulsionArray;
  vtkSmartPointer<vtkFloatArray>          AttractionArray;
  //ETX
  
  vtkLayoutEdge *EdgeArray;

  int RandomSeed;
  int IterationsPerLayout;
  int TotalIterations;
  int LayoutComplete;
  float Temp;
  float RestDistance;
  
  char* InputArrayName;
  
  // Private helper methods
  void GenerateCircularSplat(vtkImageData *splat, int x, int y);
  void GenerateGaussianSplat(vtkImageData *splat, int x, int y);
  void ResolveCoincidentVertices();

  vtkConstrained2DLayoutStrategy(const vtkConstrained2DLayoutStrategy&);  // Not implemented.
  void operator=(const vtkConstrained2DLayoutStrategy&);  // Not implemented.
};

#endif

