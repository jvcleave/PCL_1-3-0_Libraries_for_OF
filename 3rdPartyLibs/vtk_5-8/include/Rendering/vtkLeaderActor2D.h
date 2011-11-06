/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkLeaderActor2D.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkLeaderActor2D - create a leader with optional label and arrows
// .SECTION Description
// vtkLeaderActor2D creates a leader with an optional label and arrows. (A
// leader is typically used to indicate distance between points.)
// vtkLeaderActor2D is a type of vtkActor2D; that is, it is drawn on the
// overlay plane and is not occluded by 3D geometry. To use this class, you
// typically specify two points defining the start and end points of the line
// (x-y definition using vtkCoordinate class), whether to place arrows on one
// or both end points, and whether to label the leader. Also, this class has a
// special feature that allows curved leaders to be created by specifying a
// radius.
//
// Use the vtkLeaderActor2D uses its superclass vtkActor2D instance variables
// Position and Position2 vtkCoordinates to place an instance of
// vtkLeaderActor2D (i.e., these two data members represent the start and end
// points of the leader).  Using these vtkCoordinates you can specify the position
// of the leader in a variety of coordinate systems. 
//
// To control the appearance of the actor, use the superclasses
// vtkActor2D::vtkProperty2D and the vtkTextProperty objects associated with
// this actor.
//
// .SECTION See Also
// vtkAxisActor2D vtkActor2D vtkCoordinate vtkTextProperty

#ifndef __vtkLeaderActor2D_h
#define __vtkLeaderActor2D_h

#include "vtkActor2D.h"

class vtkPoints;
class vtkCellArray;
class vtkPolyData;
class vtkPolyDataMapper2D;
class vtkTextMapper;
class vtkTextProperty;

class VTK_RENDERING_EXPORT vtkLeaderActor2D : public vtkActor2D
{
public:
  vtkTypeMacro(vtkLeaderActor2D,vtkActor2D);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate object.
  static vtkLeaderActor2D *New();

  // Description:
  // Set/Get a radius which can be used to curve the leader.  If a radius is
  // specified whose absolute value is greater than one half the distance
  // between the two points defined by the superclasses' Position and
  // Position2 ivars, then the leader will be curved. A positive radius will
  // produce a curve such that the center is to the right of the line from
  // Position and Position2; a negative radius will produce a curve in the 
  // opposite sense. By default, the radius is set to zero and thus there
  // is no curvature. Note that the radius is expresses as a multiple of
  // the distance between (Position,Position2); this avoids issues relative
  // to coordinate system transformations.
  vtkSetMacro(Radius,double);
  vtkGetMacro(Radius,double);

  // Description:
  // Set/Get the label for the leader. If the label is an empty string, then
  // it will not be drawn.
  vtkSetStringMacro(Label);
  vtkGetStringMacro(Label);

  // Description:
  // Set/Get the text property of the label.
  virtual void SetLabelTextProperty(vtkTextProperty *p);
  vtkGetObjectMacro(LabelTextProperty,vtkTextProperty);
  
  // Description:
  // Set/Get the factor that controls the overall size of the fonts used
  // to label the leader. 
  vtkSetClampMacro(LabelFactor, double, 0.1, 2.0);
  vtkGetMacro(LabelFactor, double);

//BTX
  // Enums defined to support methods for control of arrow placement and
  // and appearance of arrow heads.
  enum {VTK_ARROW_NONE=0,VTK_ARROW_POINT1,VTK_ARROW_POINT2,VTK_ARROW_BOTH};
  enum {VTK_ARROW_FILLED=0,VTK_ARROW_OPEN,VTK_ARROW_HOLLOW};
//ETX  

  // Description:
  // Control whether arrow heads are drawn on the leader. Arrows may be
  // drawn on one end, both ends, or not at all.
  vtkSetClampMacro(ArrowPlacement,int,VTK_ARROW_NONE,VTK_ARROW_BOTH);
  vtkGetMacro(ArrowPlacement,int);
  void SetArrowPlacementToNone() {this->SetArrowPlacement(VTK_ARROW_NONE);}
  void SetArrowPlacementToPoint1() {this->SetArrowPlacement(VTK_ARROW_POINT1);}
  void SetArrowPlacementToPoint2() {this->SetArrowPlacement(VTK_ARROW_POINT2);}
  void SetArrowPlacementToBoth() {this->SetArrowPlacement(VTK_ARROW_BOTH);}

  // Description:
  // Control the appearance of the arrow heads. A solid arrow head is a filled
  // triangle; a open arrow looks like a "V"; and a hollow arrow looks like a
  // non-filled triangle.
  vtkSetClampMacro(ArrowStyle,int,VTK_ARROW_FILLED,VTK_ARROW_HOLLOW);
  vtkGetMacro(ArrowStyle,int);
  void SetArrowStyleToFilled() {this->SetArrowStyle(VTK_ARROW_FILLED);}
  void SetArrowStyleToOpen() {this->SetArrowStyle(VTK_ARROW_OPEN);}
  void SetArrowStyleToHollow() {this->SetArrowStyle(VTK_ARROW_HOLLOW);}

  // Description:
  // Specify the arrow length and base width (in normalized viewport
  // coordinates).
  vtkSetClampMacro(ArrowLength,double,0.0,1.0);
  vtkGetMacro(ArrowLength,double);
  vtkSetClampMacro(ArrowWidth,double,0.0,1.0);
  vtkGetMacro(ArrowWidth,double);

  // Description:
  // Limit the minimum and maximum size of the arrows. These values are
  // expressed in pixels and clamp the minimum/maximum possible size for the
  // width/length of the arrow head. (When clamped, the ratio between length
  // and width is preserved.)
  vtkSetClampMacro(MinimumArrowSize,double,1.0,VTK_LARGE_FLOAT);
  vtkGetMacro(MinimumArrowSize,double);
  vtkSetClampMacro(MaximumArrowSize,double,1.0,VTK_LARGE_FLOAT);
  vtkGetMacro(MaximumArrowSize,double);

  // Description:
  // Enable auto-labelling. In this mode, the label is automatically updated
  // based on distance (in world coordinates) between the two end points; or
  // if a curved leader is being generated, the angle in degrees between the
  // two points.
  vtkSetMacro(AutoLabel,int);
  vtkGetMacro(AutoLabel,int);
  vtkBooleanMacro(AutoLabel,int);

  // Description:
  // Specify the format to use for auto-labelling.
  vtkSetStringMacro(LabelFormat);
  vtkGetStringMacro(LabelFormat);

  // Description:
  // Obtain the length of the leader if the leader is not curved,
  // otherwise obtain the angle that the leader circumscribes.
  vtkGetMacro(Length,double);
  vtkGetMacro(Angle,double);

  // Description:
  // Methods required by vtkProp and vtkActor2D superclasses.
  int RenderOverlay(vtkViewport* viewport);
  int RenderOpaqueGeometry(vtkViewport* viewport);
  virtual int RenderTranslucentPolygonalGeometry(vtkViewport *) {return 0;}
  
  // Description:
  // Does this prop have some translucent polygonal geometry?
  virtual int HasTranslucentPolygonalGeometry();
  
  void ReleaseGraphicsResources(vtkWindow *);
  void ShallowCopy(vtkProp *prop);

protected:
  vtkLeaderActor2D();
  ~vtkLeaderActor2D();

  // Internal helper methods
  virtual void BuildLeader(vtkViewport *viewport);
  int SetFontSize(vtkViewport *viewport, vtkTextMapper *textMapper, 
                  int *targetSize, double factor, int *stringSize);
  int ClipLeader(double xL[3], int stringSize[2], double p1[3], double ray[3],
                 double c1[3], double c2[3]);
  void BuildCurvedLeader(double p1[3], double p2[3], double ray[3], double rayLength, 
                         double theta, vtkViewport *viewport, int viewportChanged);
  int InStringBox(double center[3], int stringSize[2], double x[3]);


  // Characteristics of the leader
  double Radius;
  double Length;
  double Angle;

  int              AutoLabel;
  char            *LabelFormat;
  char            *Label;
  double           LabelFactor;
  vtkTextMapper   *LabelMapper;
  vtkActor2D      *LabelActor;
  vtkTextProperty *LabelTextProperty;

  int    ArrowPlacement;
  int    ArrowStyle;
  double ArrowLength;
  double ArrowWidth;
  double MinimumArrowSize;
  double MaximumArrowSize;

  vtkPoints           *LeaderPoints;
  vtkCellArray        *LeaderLines;
  vtkCellArray        *LeaderArrows;
  vtkPolyData         *Leader;
  vtkPolyDataMapper2D *LeaderMapper;
  vtkActor2D          *LeaderActor;

  // Internal ivars for tracking whether to rebuild
  int LastPosition[2];
  int LastPosition2[2];
  int LastSize[2];
  vtkTimeStamp  BuildTime;

private:
  vtkLeaderActor2D(const vtkLeaderActor2D&);  // Not implemented.
  void operator=(const vtkLeaderActor2D&);  // Not implemented.
};


#endif
