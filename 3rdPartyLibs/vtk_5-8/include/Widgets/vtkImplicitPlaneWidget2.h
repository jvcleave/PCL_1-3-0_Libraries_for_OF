/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImplicitPlaneWidget2.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImplicitPlaneWidget2 - 3D widget for manipulating an infinite plane
// .SECTION Description
// This 3D widget defines an infinite plane that can be interactively placed
// in a scene. The widget is assumed to consist of four parts: 1) a plane
// contained in a 2) bounding box, with a 3) plane normal, which is rooted
// at a 4) point on the plane. (The representation paired with this widget
// determines the actual geometry of the widget.)
//
// To use this widget, you generally pair it with a vtkImplicitPlaneRepresentation
// (or a subclass). Variuos options are available for controlling how the 
// representation appears, and how the widget functions.

// .SECTION Event Bindings
// By default, the widget responds to the following VTK events (i.e., it
// watches the vtkRenderWindowInteractor for these events):
// <pre>
// If the plane normal is selected:
//   LeftButtonPressEvent - select normal
//   LeftButtonReleaseEvent - release normal
//   MouseMoveEvent - orient the normal vector
// If the origin point is selected:
//   LeftButtonPressEvent - select slider (if on slider)
//   LeftButtonReleaseEvent - release slider (if selected)
//   MouseMoveEvent - move the origin point (constrained to the plane)
// If the plane is selected:
//   LeftButtonPressEvent - select slider (if on slider)
//   LeftButtonReleaseEvent - release slider (if selected)
//   MouseMoveEvent - move the plane
// If the outline is selected:
//   LeftButtonPressEvent - select slider (if on slider)
//   LeftButtonReleaseEvent - release slider (if selected)
//   MouseMoveEvent - move the outline
// In all the cases, independent of what is picked, the widget responds to the 
// following VTK events:
//   MiddleButtonPressEvent - move the plane
//   MiddleButtonReleaseEvent - release the plane
//   RightButtonPressEvent - scale the widget's representation
//   RightButtonReleaseEvent - stop scaling the widget
//   MouseMoveEvent - scale (if right button) or move (if middle button) the widget
// </pre>
//
// Note that the event bindings described above can be changed using this
// class's vtkWidgetEventTranslator. This class translates VTK events 
// into the vtkImplicitPlaneWidget2's widget events:
// <pre>
//   vtkWidgetEvent::Select -- some part of the widget has been selected
//   vtkWidgetEvent::EndSelect -- the selection process has completed
//   vtkWidgetEvent::Move -- a request for slider motion has been invoked
// </pre>
//
// In turn, when these widget events are processed, the vtkImplicitPlaneWidget2
// invokes the following VTK events on itself (which observers can listen for):
// <pre>
//   vtkCommand::StartInteractionEvent (on vtkWidgetEvent::Select)
//   vtkCommand::EndInteractionEvent (on vtkWidgetEvent::EndSelect)
//   vtkCommand::InteractionEvent (on vtkWidgetEvent::Move)
// </pre>
//

// .SECTION Caveats
// Note that the widget can be picked even when it is "behind"
// other actors.  This is an intended feature and not a bug.
// 
// This class, and vtkImplicitPlaneRepresentation, are next generation VTK
// widgets. An earlier version of this functionality was defined in the class
// vtkImplicitPlaneWidget.

// .SECTION See Also
// vtk3DWidget vtkBoxWidget vtkPlaneWidget vtkLineWidget vtkPointWidget
// vtkSphereWidget vtkImagePlaneWidget

#ifndef __vtkImplicitPlaneWidget2_h
#define __vtkImplicitPlaneWidget2_h

#include "vtkAbstractWidget.h"

class vtkImplicitPlaneRepresentation;


class VTK_WIDGETS_EXPORT vtkImplicitPlaneWidget2 : public vtkAbstractWidget
{
public:
  // Description:
  // Instantiate the object.
  static vtkImplicitPlaneWidget2 *New();

  // Description:
  // Standard vtkObject methods
  vtkTypeMacro(vtkImplicitPlaneWidget2,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify an instance of vtkWidgetRepresentation used to represent this
  // widget in the scene. Note that the representation is a subclass of vtkProp
  // so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkImplicitPlaneRepresentation *r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}
  
  // Description:
  // Return the representation as a vtkImplicitPlaneRepresentation.
  vtkImplicitPlaneRepresentation *GetImplicitPlaneRepresentation()
    {return reinterpret_cast<vtkImplicitPlaneRepresentation*>(this->WidgetRep);}

  // Description:
  // Create the default widget representation if one is not set. 
  void CreateDefaultRepresentation();

protected:
  vtkImplicitPlaneWidget2();
  ~vtkImplicitPlaneWidget2();

//BTX - manage the state of the widget
  int WidgetState;
  enum _WidgetState {Start=0,Active};
//ETX
    
  // These methods handle events
  static void SelectAction(vtkAbstractWidget*);
  static void TranslateAction(vtkAbstractWidget*);
  static void ScaleAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);

  // Description:
  // Update the cursor shape based on the interaction state. Returns 1
  // if the cursor shape requested is different from the existing one.
  int UpdateCursorShape( int interactionState );

private:
  vtkImplicitPlaneWidget2(const vtkImplicitPlaneWidget2&);  //Not implemented
  void operator=(const vtkImplicitPlaneWidget2&);  //Not implemented
};

#endif
