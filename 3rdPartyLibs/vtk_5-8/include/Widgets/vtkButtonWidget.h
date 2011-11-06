/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkButtonWidget.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkButtonWidget - activate an n-state button
// .SECTION Description
// The vtkButtonWidget is used to interface with an n-state button. That is
// each selection moves to the next button state (e.g., moves from "on" to
// "off"). The widget uses modulo list traversal to transition through one or
// more states. (A single state is simply a "selection" event; traversal
// through the list can be in the forward or backward direction.)
//
// Depending on the nature of the representation the appearance of the button
// can change dramatically, the specifics of appearance changes are a
// function of the associated vtkButtonRepresentation (or subclass).
//
// .SECTION Event Bindings
// By default, the widget responds to the following VTK events (i.e., it
// watches the vtkRenderWindowInteractor for these events):
// <pre>
//   LeftButtonPressEvent - select button
//   LeftButtonReleaseEvent - end the button selection process
// </pre>
//
// Note that the event bindings described above can be changed using this
// class's vtkWidgetEventTranslator. This class translates VTK events
// into the vtkButtonWidget's widget events:
// <pre>
//   vtkWidgetEvent::Select -- some part of the widget has been selected
//   vtkWidgetEvent::EndSelect -- the selection process has completed
// </pre>
//
// In turn, when these widget events are processed, the vtkButtonWidget
// invokes the following VTK events on itself (which observers can listen for):
// <pre>
//   vtkCommand::StateChangedEvent (on vtkWidgetEvent::EndSelect)
// </pre>
//

#ifndef __vtkButtonWidget_h
#define __vtkButtonWidget_h

#include "vtkAbstractWidget.h"

class vtkButtonRepresentation;


class VTK_WIDGETS_EXPORT vtkButtonWidget : public vtkAbstractWidget
{
public:
  // Description:
  // Instantiate the class.
  static vtkButtonWidget *New();

  // Description:
  // Standard macros.
  vtkTypeMacro(vtkButtonWidget,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify an instance of vtkWidgetRepresentation used to represent this
  // widget in the scene. Note that the representation is a subclass of vtkProp
  // so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkButtonRepresentation *r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}

  // Description:
  // Return the representation as a vtkButtonRepresentation.
  vtkButtonRepresentation *GetSliderRepresentation()
    {return reinterpret_cast<vtkButtonRepresentation*>(this->WidgetRep);}

  // Description:
  // Create the default widget representation if one is not set.
  void CreateDefaultRepresentation();

protected:
  vtkButtonWidget();
  ~vtkButtonWidget() {}

  // These are the events that are handled
  static void SelectAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);

//BTX - manage the state of the widget
  int WidgetState;
  enum _WidgetState
  {
    Start=0,
    Hovering,
    Selecting
  };
  //ETX

private:
  vtkButtonWidget(const vtkButtonWidget&);  //Not implemented
  void operator=(const vtkButtonWidget&);  //Not implemented
};

#endif
