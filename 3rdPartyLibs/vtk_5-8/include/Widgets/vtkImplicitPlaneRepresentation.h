/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImplicitPlaneRepresentation.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImplicitPlaneRepresentation - a class defining the representation for a vtkImplicitPlaneWidget2
// .SECTION Description
// This class is a concrete representation for the
// vtkImplicitPlaneWidget2. It represents an infinite plane defined by a
// normal and point in the context of a bounding box. Through interaction
// with the widget, the plane can be manipulated by adjusting the plane
// normal or moving the origin point.
//
// To use this representation, you normally define a (plane) origin and (plane)
// normal. The PlaceWidget() method is also used to intially position the
// representation.

// .SECTION Caveats
// This class, and vtkImplicitPlaneWidget2, are next generation VTK
// widgets. An earlier version of this functionality was defined in the
// class vtkImplicitPlaneWidget.

// .SECTION See Also
// vtkImplicitPlaneWidget2 vtkImplicitPlaneWidget


#ifndef __vtkImplicitPlaneRepresentation_h
#define __vtkImplicitPlaneRepresentation_h

#include "vtkWidgetRepresentation.h"

class vtkActor;
class vtkPolyDataMapper;
class vtkCellPicker;
class vtkConeSource;
class vtkLineSource;
class vtkSphereSource;
class vtkTubeFilter;
class vtkPlane;
class vtkCutter;
class vtkProperty;
class vtkImageData;
class vtkOutlineFilter;
class vtkFeatureEdges;
class vtkPolyData;
class vtkPolyDataAlgorithm;
class vtkTransform;
class vtkBox;

class VTK_WIDGETS_EXPORT vtkImplicitPlaneRepresentation : public vtkWidgetRepresentation
{
public:
  // Description:
  // Instantiate the class.
  static vtkImplicitPlaneRepresentation *New();

  // Description:
  // Standard methods for the class.
  vtkTypeMacro(vtkImplicitPlaneRepresentation,vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the origin of the plane.
  void SetOrigin(double x, double y, double z);
  void SetOrigin(double x[3]);
  double* GetOrigin();
  void GetOrigin(double xyz[3]);

  // Description:
  // Get the normal to the plane.
  void SetNormal(double x, double y, double z);
  void SetNormal(double x[3]);
  double* GetNormal();
  void GetNormal(double xyz[3]);

  // Description:
  // Force the plane widget to be aligned with one of the x-y-z axes.
  // If one axis is set on, the other two will be set off.
  // Remember that when the state changes, a ModifiedEvent is invoked.
  // This can be used to snap the plane to the axes if it is orginally
  // not aligned.
  void SetNormalToXAxis(int);
  vtkGetMacro(NormalToXAxis,int);
  vtkBooleanMacro(NormalToXAxis,int);
  void SetNormalToYAxis(int);
  vtkGetMacro(NormalToYAxis,int);
  vtkBooleanMacro(NormalToYAxis,int);
  void SetNormalToZAxis(int);
  vtkGetMacro(NormalToZAxis,int);
  vtkBooleanMacro(NormalToZAxis,int);

  // Description:
  // Turn on/off tubing of the wire outline of the plane. The tube thickens
  // the line by wrapping with a vtkTubeFilter.
  vtkSetMacro(Tubing,int);
  vtkGetMacro(Tubing,int);
  vtkBooleanMacro(Tubing,int);

  // Description:
  // Enable/disable the drawing of the plane. In some cases the plane
  // interferes with the object that it is operating on (i.e., the
  // plane interferes with the cut surface it produces producing
  // z-buffer artifacts.)
  void SetDrawPlane(int plane);
  vtkGetMacro(DrawPlane,int);
  vtkBooleanMacro(DrawPlane,int);

  // Description:
  // Turn on/off the ability to translate the bounding box by grabbing it
  // with the left mouse button.
  vtkSetMacro(OutlineTranslation,int);
  vtkGetMacro(OutlineTranslation,int);
  vtkBooleanMacro(OutlineTranslation,int);

  // Description:
  // Turn on/off the ability to move the widget outside of the bounds
  // specified in the initial PlaceWidget() invocation.
  vtkSetMacro(OutsideBounds,int);
  vtkGetMacro(OutsideBounds,int);
  vtkBooleanMacro(OutsideBounds,int);

  // Description:
  // Turn on/off the ability to scale the widget with the mouse.
  vtkSetMacro(ScaleEnabled,int);
  vtkGetMacro(ScaleEnabled,int);
  vtkBooleanMacro(ScaleEnabled,int);

  // Description:
  // Grab the polydata that defines the plane. The polydata contains a single
  // polygon that is clipped by the bounding box.
  void GetPolyData(vtkPolyData *pd);

  // Description:
  // Satisfies superclass API.  This returns a pointer to the underlying
  // PolyData (which represents the plane).
  vtkPolyDataAlgorithm* GetPolyDataAlgorithm();

  // Description:
  // Get the implicit function for the plane. The user must provide the
  // instance of the class vtkPlane. Note that vtkPlane is a subclass of
  // vtkImplicitFunction, meaning that it can be used by a variety of filters
  // to perform clipping, cutting, and selection of data.
  void GetPlane(vtkPlane *plane);

  // Description:
  // Satisfies the superclass API.  This will change the state of the widget
  // to match changes that have been made to the underlying PolyDataSource
  void UpdatePlacement(void);

  // Description:
  // Get the properties on the normal (line and cone).
  vtkGetObjectMacro(NormalProperty,vtkProperty);
  vtkGetObjectMacro(SelectedNormalProperty,vtkProperty);

  // Description:
  // Get the plane properties. The properties of the plane when selected
  // and unselected can be manipulated.
  vtkGetObjectMacro(PlaneProperty,vtkProperty);
  vtkGetObjectMacro(SelectedPlaneProperty,vtkProperty);

  // Description:
  // Get the property of the outline.
  vtkGetObjectMacro(OutlineProperty,vtkProperty);
  vtkGetObjectMacro(SelectedOutlineProperty,vtkProperty);

  // Description:
  // Get the property of the intersection edges. (This property also
  // applies to the edges when tubed.)
  vtkGetObjectMacro(EdgesProperty,vtkProperty);

  // Description:
  // Methods to interface with the vtkSliderWidget.
  virtual int ComputeInteractionState(int X, int Y, int modify=0);
  virtual void PlaceWidget(double bounds[6]);
  virtual void BuildRepresentation();
  virtual void StartWidgetInteraction(double eventPos[2]);
  virtual void WidgetInteraction(double newEventPos[2]);
  virtual void EndWidgetInteraction(double newEventPos[2]);

  // Decsription:
  // Methods supporting the rendering process.
  virtual double *GetBounds();
  virtual void GetActors(vtkPropCollection *pc);
  virtual void ReleaseGraphicsResources(vtkWindow*);
  virtual int RenderOpaqueGeometry(vtkViewport*);
  virtual int RenderTranslucentPolygonalGeometry(vtkViewport*);
  virtual int HasTranslucentPolygonalGeometry();

//BTX - manage the state of the widget
  enum _InteractionState
  {
    Outside=0,
    Moving,
    MovingOutline,
    MovingOrigin,
    Rotating,
    Pushing,
    MovingPlane,
    Scaling
  };
//ETX

  // Description:
  // The interaction state may be set from a widget (e.g.,
  // vtkImplicitPlaneWidget2) or other object. This controls how the
  // interaction with the widget proceeds. Normally this method is used as
  // part of a handshaking process with the widget: First
  // ComputeInteractionState() is invoked that returns a state based on
  // geometric considerations (i.e., cursor near a widget feature), then
  // based on events, the widget may modify this further.
  vtkSetClampMacro(InteractionState,int,Outside,Scaling);

  // Description:
  // Sets the visual appearance of the representation based on the
  // state it is in. This state is usually the same as InteractionState.
  virtual void SetRepresentationState(int);
  vtkGetMacro(RepresentationState, int);

protected:
  vtkImplicitPlaneRepresentation();
  ~vtkImplicitPlaneRepresentation();

  int RepresentationState;

  // Keep track of event positions
  double LastEventPosition[3];

  // Controlling ivars
  int NormalToXAxis;
  int NormalToYAxis;
  int NormalToZAxis;

  // The actual plane which is being manipulated
  vtkPlane *Plane;

  // The bounding box is represented by a single voxel image data
  vtkImageData      *Box;
  vtkOutlineFilter  *Outline;
  vtkPolyDataMapper *OutlineMapper;
  vtkActor          *OutlineActor;
  void HighlightOutline(int highlight);
  int  OutlineTranslation; //whether the outline can be moved
  int  ScaleEnabled; //whether the widget can be scaled
  int  OutsideBounds; //whether the widget can be moved outside input's bounds

  // The cut plane is produced with a vtkCutter
  vtkCutter         *Cutter;
  vtkPolyDataMapper *CutMapper;
  vtkActor          *CutActor;
  int                DrawPlane;
  void HighlightPlane(int highlight);

  // Optional tubes are represented by extracting boundary edges and tubing
  vtkFeatureEdges   *Edges;
  vtkTubeFilter     *EdgesTuber;
  vtkPolyDataMapper *EdgesMapper;
  vtkActor          *EdgesActor;
  int                Tubing; //control whether tubing is on

  // The + normal cone
  vtkConeSource     *ConeSource;
  vtkPolyDataMapper *ConeMapper;
  vtkActor          *ConeActor;
  void HighlightNormal(int highlight);

  // The + normal line
  vtkLineSource     *LineSource;
  vtkPolyDataMapper *LineMapper;
  vtkActor          *LineActor;

  // The - normal cone
  vtkConeSource     *ConeSource2;
  vtkPolyDataMapper *ConeMapper2;
  vtkActor          *ConeActor2;

  // The - normal line
  vtkLineSource     *LineSource2;
  vtkPolyDataMapper *LineMapper2;
  vtkActor          *LineActor2;

  // The origin positioning handle
  vtkSphereSource   *Sphere;
  vtkPolyDataMapper *SphereMapper;
  vtkActor          *SphereActor;

  // Do the picking
  vtkCellPicker *Picker;

  // Transform the normal (used for rotation)
  vtkTransform *Transform;

  // Methods to manipulate the plane
  void ConstrainOrigin(double x[3]);
  void Rotate(double X, double Y, double *p1, double *p2, double *vpn);
  void TranslatePlane(double *p1, double *p2);
  void TranslateOutline(double *p1, double *p2);
  void TranslateOrigin(double *p1, double *p2);
  void Push(double *p1, double *p2);
  void Scale(double *p1, double *p2, double X, double Y);
  void SizeHandles();

  // Properties used to control the appearance of selected objects and
  // the manipulator in general.
  vtkProperty *NormalProperty;
  vtkProperty *SelectedNormalProperty;
  vtkProperty *PlaneProperty;
  vtkProperty *SelectedPlaneProperty;
  vtkProperty *OutlineProperty;
  vtkProperty *SelectedOutlineProperty;
  vtkProperty *EdgesProperty;
  void CreateDefaultProperties();

  void GeneratePlane();

  // Support GetBounds() method
  vtkBox *BoundingBox;

private:
  vtkImplicitPlaneRepresentation(const vtkImplicitPlaneRepresentation&);  //Not implemented
  void operator=(const vtkImplicitPlaneRepresentation&);  //Not implemented
};

#endif
