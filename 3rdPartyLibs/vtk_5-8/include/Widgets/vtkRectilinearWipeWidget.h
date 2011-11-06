/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRectilinearWipeWidget.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkRectilinearWipeWidget - interactively control an instance of vtkImageRectilinearWipe filter
// .SECTION Description
// The vtkRectilinearWipeWidget is used to interactively control an instance
// of vtkImageRectilinearWipe (and an associated vtkImageActor used to
// display the rectilinear wipe). A rectilinear wipe is a 2x2 checkerboard
// pattern created by combining two separate images, where various
// combinations of the checker squares are possible. Using this widget, the
// user can adjust the layout of the checker pattern, such as moving the
// center point, moving the horizontal separator, or moving the vertical
// separator. These capabilities are particularly useful for comparing two
// images.
// 
// To use this widget, specify its representation (by default the
// representation is an instance of vtkRectilinearWipeProp). The
// representation generally requires that you specify an instance of
// vtkImageRectilinearWipe and an instance of vtkImageActor. Other instance
// variables may also be required to be set -- see the documentation for
// vtkRectilinearWipeProp (or appropriate subclass).
//
// By default, the widget responds to the following events:
// <pre>
// Selecting the center point, horizontal separator, and verticel separator:
//   LeftButtonPressEvent - move the separators
//   LeftButtonReleaseEvent - release the separators 
//   MouseMoveEvent - move the separators
// </pre>
// Selecting the center point allows you to move the horizontal and vertical
// separators simultaneously. Otherwise only horizontal or vertical motion
// is possible/
//
// Note that the event bindings described above can be changed using this
// class's vtkWidgetEventTranslator. This class translates VTK events into
// the vtkRectilinearWipeWidget's widget events:
// <pre>
//   vtkWidgetEvent::Select -- some part of the widget has been selected
//   vtkWidgetEvent::EndSelect -- the selection process has completed
//   vtkWidgetEvent::Move -- a request for motion has been invoked
// </pre>
//
// In turn, when these widget events are processed, the
// vtkRectilinearWipeWidget invokes the following VTK events (which
// observers can listen for):
// <pre>
//   vtkCommand::StartInteractionEvent (on vtkWidgetEvent::Select)
//   vtkCommand::EndInteractionEvent (on vtkWidgetEvent::EndSelect)
//   vtkCommand::InteractionEvent (on vtkWidgetEvent::Move)
// </pre>
//
// .SECTION Caveats
// The appearance of this widget is defined by its representation, including
// any properties associated with the representation.  The widget
// representation is a type of vtkProp that defines a particular API that
// works with this widget. If desired, the vtkProp may be subclassed to
// create new looks for the widget.

// .SECTION See Also
// vtkRectilinearWipeProp vtkImageRectilinearWipe vtkImageActor 
// vtkCheckerboardWidget


#ifndef __vtkRectilinearWipeWidget_h
#define __vtkRectilinearWipeWidget_h

#include "vtkAbstractWidget.h"

class vtkRectilinearWipeRepresentation;


class VTK_WIDGETS_EXPORT vtkRectilinearWipeWidget : public vtkAbstractWidget
{
public:
  // Description:
  // Instantiate the class.
  static vtkRectilinearWipeWidget *New();

  // Description:
  // Standard macros.
  vtkTypeMacro(vtkRectilinearWipeWidget,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify an instance of vtkWidgetRepresentation used to represent this
  // widget in the scene. Note that the representation is a subclass of vtkProp
  // so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkRectilinearWipeRepresentation *r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}
  
  // Description:
  // Return the representation as a vtkRectilinearWipeRepresentation.
  vtkRectilinearWipeRepresentation *GetRectilinearWipeRepresentation()
    {return reinterpret_cast<vtkRectilinearWipeRepresentation*>(this->WidgetRep);}

  // Description:
  // Create the default widget representation if one is not set. 
  virtual void CreateDefaultRepresentation();

protected:
  vtkRectilinearWipeWidget();
  ~vtkRectilinearWipeWidget();
  
  // These methods handle events
  static void SelectAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);

  // helper methods for cursor management
  void SetCursor(int state);

//BTX - manage the state of the widget
  int WidgetState;
  enum _WidgetState
  {
    Start=0,
    Selected
  };
//ETX

private:
  vtkRectilinearWipeWidget(const vtkRectilinearWipeWidget&);  //Not implemented
  void operator=(const vtkRectilinearWipeWidget&);  //Not implemented
};

#endif
