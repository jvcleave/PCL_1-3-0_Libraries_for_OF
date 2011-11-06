package require vtk
package require vtkinteraction

# Simple volume rendering example.
vtkImageReader reader
reader SetDataByteOrderToLittleEndian
reader SetDataExtent 0 63 0 63 1 93
reader SetFilePrefix "$VTK_DATA_ROOT/Data/headsq/quarter"
reader SetDataMask 0x7fff
reader SetDataSpacing 2 2 1
reader SetDataScalarTypeToUnsignedShort
reader Update

# Create transfer functions for opacity and color
vtkPiecewiseFunction opacityTransferFunction
    opacityTransferFunction AddPoint  600   0.0
    opacityTransferFunction AddPoint 2000   1.0

vtkColorTransferFunction colorTransferFunction
    colorTransferFunction ClampingOff
    colorTransferFunction AddHSVPoint      0.0 0.01 1.0 1.0
    colorTransferFunction AddHSVPoint   1000.0 0.50 1.0 1.0
    colorTransferFunction AddHSVPoint   2000.0 0.99 1.0 1.0
    colorTransferFunction SetColorSpaceToHSV


# Create properties, mappers, volume actors, and ray cast function
vtkVolumeProperty volumeProperty
    volumeProperty SetColor colorTransferFunction
    volumeProperty SetScalarOpacity opacityTransferFunction
    volumeProperty SetInterpolationTypeToLinear

vtkVolumeProMapper volumeMapper
    volumeMapper SetInputConnection [reader GetOutputPort]
    volumeMapper SetBlendModeToMaximumIntensity
    volumeMapper CutPlaneOn
    volumeMapper SetCutPlaneEquation -1 0 0 57
    volumeMapper SetCutPlaneThickness 20

vtkVolume volume
    volume SetMapper volumeMapper
    volume SetProperty volumeProperty

# Create geometric sphere
vtkSphereSource sphereSource
    sphereSource SetCenter  63 63 47
    sphereSource SetRadius  65
    sphereSource SetThetaResolution 20
    sphereSource SetPhiResolution 40

vtkPolyDataMapper sphereMapper
    sphereMapper SetInputConnection [sphereSource GetOutputPort]

vtkActor sphereActor
    sphereActor SetMapper sphereMapper

# Set up the planes
vtkPlane plane1
plane1 SetOrigin 63 63 37
plane1 SetNormal 0 0 1

vtkPlane plane2
plane2 SetOrigin 63 63 57
plane2 SetNormal 0 0 -1

sphereMapper AddClippingPlane plane1
sphereMapper AddClippingPlane plane2

# Okay now the graphics stuff
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
    renWin SetSize 256 256
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin
    iren AddObserver UserEvent {wm deiconify .vtkInteract}

[ren1 GetCullers] InitTraversal
set culler [[ren1 GetCullers] GetNextItem]
$culler SetSortingStyleToBackToFront

[ren1 GetActiveCamera] ParallelProjectionOn

ren1 AddViewProp sphereActor
ren1 AddViewProp volume
ren1 SetBackground 0.1 0.2 0.4
renWin Render
ren1 ResetCamera

[ren1 GetActiveCamera] Azimuth 45
[ren1 GetActiveCamera] Elevation 15
[ren1 GetActiveCamera] Roll 45
[ren1 GetActiveCamera] Zoom 2.0

wm withdraw .
 
iren Initialize
