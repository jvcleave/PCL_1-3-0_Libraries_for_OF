/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCaptionWidget.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCaptionWidget - widget for placing a caption (text plus leader)
// .SECTION Description
// This class provides support for interactively placing a caption on the 2D
// overlay plane. A caption is defined by some text with a leader (e.g.,
// arrow) that points from the text to a point in the scene. The caption is
// represented by a vtkCaptionRepresentation. It uses the event bindings of
// its superclass (vtkBorderWidget) to control the placement of the text, and
// adds the ability to move the attachment point around. In addition, when
// the caption text is selected, the widget emits a ActivateEvent that
// observers can watch for. This is useful for opening GUI dialogoues to
// adjust font characteristics, etc. (Please see the superclass for a
// description of event bindings.)
//
// Note that this widget extends the behavior of its superclass 
// vtkBorderWidget. The end point of the leader can be selected and
// moved around with an internal vtkHandleWidget.

// .SECTION See Also
// vtkBorderWidget vtkTextWidget


#ifndef __vtkCaptionWidget_h
#define __vtkCaptionWidget_h

#include "vtkBorderWidget.h"

class vtkCaptionRepresentation;
class vtkCaptionActor2D;
class vtkHandleWidget;
class vtkPointHandleRepresentation3D;
class vtkCaptionAnchorCallback;


class VTK_WIDGETS_EXPORT vtkCaptionWidget : public vtkBorderWidget
{
public:
  // Description:
  // Instantiate this class.
  static vtkCaptionWidget *New();

  // Description:
  // Standard VTK class methods.
  vtkTypeMacro(vtkCaptionWidget,vtkBorderWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Override superclasses' SetEnabled() method because the caption leader
  // has its own dedicated widget.
  virtual void SetEnabled(int enabling);

  // Description:
  // Specify an instance of vtkWidgetRepresentation used to represent this
  // widget in the scene. Note that the representation is a subclass of vtkProp
  // so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkCaptionRepresentation *r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}
  
  // Description:
  // Specify a vtkCaptionActor2D to manage. This is convenient, alternative
  // method to SetRepresentation(). It internally create a vtkCaptionRepresentation
  // and then invokes vtkCaptionRepresentation::SetCaptionActor2D().
  void SetCaptionActor2D(vtkCaptionActor2D *capActor);
  vtkCaptionActor2D *GetCaptionActor2D();

  // Description:
  // Create the default widget representation if one is not set. 
  void CreateDefaultRepresentation();

protected:
  vtkCaptionWidget();
  ~vtkCaptionWidget();

  // Handles callbacks from the anchor point
  vtkCaptionAnchorCallback *AnchorCallback;

  // Widget for the anchor point
  vtkHandleWidget *HandleWidget;
  
  // Special callbacks for the anchor interaction
  void StartAnchorInteraction();
  void AnchorInteraction();
  void EndAnchorInteraction();

//BTX
  friend class vtkCaptionAnchorCallback;
//ETX  

private:
  vtkCaptionWidget(const vtkCaptionWidget&);  //Not implemented
  void operator=(const vtkCaptionWidget&);  //Not implemented
};

#endif
