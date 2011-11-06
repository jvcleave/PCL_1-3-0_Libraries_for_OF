/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageViewer2.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageViewer2 - Display a 2D image.
// .SECTION Description
// vtkImageViewer2 is a convenience class for displaying a 2D image.  It
// packages up the functionality found in vtkRenderWindow, vtkRenderer,
// vtkImageActor and vtkImageMapToWindowLevelColors into a single easy to use
// class.  This class also creates an image interactor style
// (vtkInteractorStyleImage) that allows zooming and panning of images, and
// supports interactive window/level operations on the image. Note that
// vtkImageViewer2 is simply a wrapper around these classes.
//
// vtkImageViewer2 uses the 3D rendering and texture mapping engine
// to draw an image on a plane.  This allows for rapid rendering,
// zooming, and panning. The image is placed in the 3D scene at a
// depth based on the z-coordinate of the particular image slice. Each
// call to SetSlice() changes the image data (slice) displayed AND
// changes the depth of the displayed slice in the 3D scene. This can
// be controlled by the AutoAdjustCameraClippingRange ivar of the
// InteractorStyle member.
//
// It is possible to mix images and geometry, using the methods:
//
// viewer->SetInput( myImage );
// viewer->GetRenderer()->AddActor( myActor );
//
// This can be used to annotate an image with a PolyData of "edges" or
// or highlight sections of an image or display a 3D isosurface
// with a slice from the volume, etc. Any portions of your geometry
// that are in front of the displayed slice will be visible; any
// portions of your geometry that are behind the displayed slice will
// be obscured. A more general framework (with respect to viewing
// direction) for achieving this effect is provided by the
// vtkImagePlaneWidget .
//
// Note that pressing 'r' will reset the window/level and pressing
// shift+'r' or control+'r' will reset the camera.
//
// .SECTION See Also
// vtkRenderWindow vtkRenderer vtkImageActor vtkImageMapToWindowLevelColors

#ifndef __vtkImageViewer2_h
#define __vtkImageViewer2_h

#include "vtkObject.h"

class vtkAlgorithmOutput;
class vtkImageActor;
class vtkImageData;
class vtkImageMapToWindowLevelColors;
class vtkInteractorStyleImage;
class vtkRenderWindow;
class vtkRenderer;
class vtkRenderWindowInteractor;

class VTK_RENDERING_EXPORT vtkImageViewer2 : public vtkObject 
{
public:
  static vtkImageViewer2 *New();
  vtkTypeMacro(vtkImageViewer2,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the name of rendering window.
  virtual const char *GetWindowName();

  // Description:
  // Render the resulting image.
  virtual void Render(void);
  
  // Description:
  // Set/Get the input image to the viewer.
  virtual void SetInput(vtkImageData *in);
  virtual vtkImageData *GetInput();
  virtual void SetInputConnection(vtkAlgorithmOutput* input);
  
  // Description:
  // Set/get the slice orientation
  //BTX
  enum
  {
    SLICE_ORIENTATION_YZ = 0,
    SLICE_ORIENTATION_XZ = 1,
    SLICE_ORIENTATION_XY = 2
  };
  //ETX
  vtkGetMacro(SliceOrientation, int);
  virtual void SetSliceOrientation(int orientation);
  virtual void SetSliceOrientationToXY()
    { this->SetSliceOrientation(vtkImageViewer2::SLICE_ORIENTATION_XY); };
  virtual void SetSliceOrientationToYZ()
    { this->SetSliceOrientation(vtkImageViewer2::SLICE_ORIENTATION_YZ); };
  virtual void SetSliceOrientationToXZ()
    { this->SetSliceOrientation(vtkImageViewer2::SLICE_ORIENTATION_XZ); };

  // Description:
  // Set/Get the current slice to display (depending on the orientation
  // this can be in X, Y or Z).
  vtkGetMacro(Slice, int);
  virtual void SetSlice(int s);

  // Description:
  // Update the display extent manually so that the proper slice for the
  // given orientation is displayed. It will also try to set a
  // reasonable camera clipping range.
  // This method is called automatically when the Input is changed, but
  // most of the time the input of this class is likely to remain the same,
  // i.e. connected to the output of a filter, or an image reader. When the
  // input of this filter or reader itself is changed, an error message might
  // be displayed since the current display extent is probably outside
  // the new whole extent. Calling this method will ensure that the display
  // extent is reset properly.
  virtual void UpdateDisplayExtent();
  
  // Description:
  // Return the minimum and maximum slice values (depending on the orientation
  // this can be in X, Y or Z).
  virtual int GetSliceMin();
  virtual int GetSliceMax();
  virtual void GetSliceRange(int range[2])
    { this->GetSliceRange(range[0], range[1]); }
  virtual void GetSliceRange(int &min, int &max);
  virtual int* GetSliceRange();
  
  // Description:
  // Set window and level for mapping pixels to colors.
  virtual double GetColorWindow();
  virtual double GetColorLevel();
  virtual void SetColorWindow(double s);
  virtual void SetColorLevel(double s);

  // Description:
  // These are here when using a Tk window.
  virtual void SetDisplayId(void *a);
  virtual void SetWindowId(void *a);
  virtual void SetParentId(void *a);
  
  // Description:
  // Set/Get the position in screen coordinates of the rendering window.
  virtual int* GetPosition();
  virtual void SetPosition(int a,int b);
  virtual void SetPosition(int a[2]) { this->SetPosition(a[0],a[1]); }
  
  // Description:
  // Set/Get the size of the window in screen coordinates in pixels.
  virtual int* GetSize();
  virtual void SetSize(int a, int b);
  virtual void SetSize(int a[2]) { this->SetSize(a[0],a[1]); }
  
  // Description:
  // Get the internal render window, renderer, image actor, and
  // image map instances.
  vtkGetObjectMacro(RenderWindow,vtkRenderWindow);
  vtkGetObjectMacro(Renderer, vtkRenderer);
  vtkGetObjectMacro(ImageActor,vtkImageActor);
  vtkGetObjectMacro(WindowLevel,vtkImageMapToWindowLevelColors);
  vtkGetObjectMacro(InteractorStyle,vtkInteractorStyleImage);
  
  // Description:
  // Set your own renderwindow and renderer
  virtual void SetRenderWindow(vtkRenderWindow *arg);
  virtual void SetRenderer(vtkRenderer *arg);

  // Description:
  // Attach an interactor for the internal render window.
  virtual void SetupInteractor(vtkRenderWindowInteractor*);
  
  // Description:  
  // Create a window in memory instead of on the screen. This may not
  // be supported for every type of window and on some windows you may
  // need to invoke this prior to the first render.
  virtual void SetOffScreenRendering(int);
  virtual int GetOffScreenRendering();
  vtkBooleanMacro(OffScreenRendering,int);

  // Description:
  // @deprecated Replaced by vtkImageViewer2::GetSliceMin() as of VTK 5.0.
  VTK_LEGACY(int GetWholeZMin());

  // Description:
  // @deprecated Replaced by vtkImageViewer2::GetSliceMax() as of VTK 5.0.
  VTK_LEGACY(int GetWholeZMax());

  // Description:
  // @deprecated Replaced by vtkImageViewer2::GetSlice() as of VTK 5.0.
  VTK_LEGACY(int GetZSlice());

  // Description:
  // @deprecated Replaced by vtkImageViewer2::SetSlice() as of VTK 5.0.
  VTK_LEGACY(void SetZSlice(int));

protected:
  vtkImageViewer2();
  ~vtkImageViewer2();

  virtual void InstallPipeline();
  virtual void UnInstallPipeline();

  vtkImageMapToWindowLevelColors  *WindowLevel;
  vtkRenderWindow                 *RenderWindow;
  vtkRenderer                     *Renderer;
  vtkImageActor                   *ImageActor;
  vtkRenderWindowInteractor       *Interactor;
  vtkInteractorStyleImage         *InteractorStyle;

  int SliceOrientation;
  int FirstRender;
  int Slice;

  virtual void UpdateOrientation();

private:
  vtkImageViewer2(const vtkImageViewer2&);  // Not implemented.
  void operator=(const vtkImageViewer2&);  // Not implemented.
};

#endif


