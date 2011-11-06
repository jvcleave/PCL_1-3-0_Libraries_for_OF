/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSeedRepresentation.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSeedRepresentation - represent the vtkSeedWidget
// .SECTION Description
// The vtkSeedRepresentation is a superclass for classes representing the
// vtkSeedWidget. This representation consists of one or more handles
// (vtkHandleRepresentation) which are used to place and manipulate the
// points defining the collection of seeds. 

// .SECTION See Also
// vtkSeedWidget vtkHandleRepresentation vtkSeedRepresentation


#ifndef __vtkSeedRepresentation_h
#define __vtkSeedRepresentation_h

#include "vtkWidgetRepresentation.h"

class vtkHandleList;
class vtkHandleRepresentation;


class VTK_WIDGETS_EXPORT vtkSeedRepresentation : public vtkWidgetRepresentation
{
public:
  // Description:
  // Instantiate class.
  static vtkSeedRepresentation *New();

  // Description:
  // Standard VTK methods.
  vtkTypeMacro(vtkSeedRepresentation,vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Methods to Set/Get the coordinates of seed points defining
  // this representation. Note that methods are available for both
  // display and world coordinates. The seeds are accessed by a seed
  // number.
  virtual void GetSeedWorldPosition( unsigned int seedNum, double pos[3] );
  virtual void SetSeedDisplayPosition( unsigned int seedNum, double pos[3] );
  virtual void GetSeedDisplayPosition( unsigned int seedNum, double pos[3] );

  // Description:
  // Return the number of seeds (or handles) that have been created.
  int GetNumberOfSeeds();

  // Description:
  // This method is used to specify the type of handle representation to use
  // for the internal vtkHandleWidgets within vtkSeedWidget.  To use this
  // method, create a dummy vtkHandleWidget (or subclass), and then invoke
  // this method with this dummy. Then the vtkSeedRepresentation uses this
  // dummy to clone vtkHandleWidgets of the same type. Make sure you set the
  // handle representation before the widget is enabled. 
  void SetHandleRepresentation( vtkHandleRepresentation *handle );

  // Description:
  // Get the handle representations used for a particular seed. A side effect of
  // this method is that it will create a handle representation in the list of
  // representations if one has not yet been created.
  vtkHandleRepresentation *GetHandleRepresentation( unsigned int num );

  // Description:
  // Returns the model HandleRepresentation.
  vtkHandleRepresentation *GetHandleRepresentation()
    {
    return this->HandleRepresentation;
    };

  // Description:
  // The tolerance representing the distance to the widget (in pixels) in
  // which the cursor is considered near enough to the seed points of
  // the widget to be active.
  vtkSetClampMacro( Tolerance, int, 1, 100 );
  vtkGetMacro( Tolerance, int );

//BTX -- used to communicate about the state of the representation
  enum
    {
    Outside = 0,
    NearSeed
    };
//ETX

  // Description:
  // These are methods specific to vtkSeedRepresentation and which are
  // invoked from vtkSeedWidget.
  virtual int GetActiveHandle();
  // Returns the id of the seed created, -1 on failure. e is the display position.
  virtual int CreateHandle( double e[2] );
  // Delete last handle created
  virtual void RemoveLastHandle();
  // Delete the currently active handle
  virtual void RemoveActiveHandle();

  // Description:
  // Remove the nth handle.
  virtual void RemoveHandle( int n );

  // Description:
  // These are methods that satisfy vtkWidgetRepresentation's API.
  virtual void BuildRepresentation();
  virtual int ComputeInteractionState( int X, int Y, int modify = 0 );
  
protected:
  vtkSeedRepresentation();
  ~vtkSeedRepresentation();

  // The handle and the rep used to close the handles
  vtkHandleRepresentation  *HandleRepresentation;
  vtkHandleList            *Handles;

  // Selection tolerance for the handles
  int Tolerance;
  
  // The active seed (handle) based on the last ComputeInteractionState()
  int ActiveHandle;

private:
  vtkSeedRepresentation(const vtkSeedRepresentation&);  //Not implemented
  void operator=(const vtkSeedRepresentation&);  //Not implemented
};

#endif
