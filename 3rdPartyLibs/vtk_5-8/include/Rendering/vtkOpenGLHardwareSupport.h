/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOpenGLHardwareSupport.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOpenGLHardwareSupport - OpenGL rendering window
// .SECTION Description
// vtkOpenGLHardwareSupport is an implementation of methods used
// to query OpenGL and the hardware of what kind of graphics support
// is available. When VTK supports more than one Graphics API an
// abstract super class vtkHardwareSupport should be implemented
// for this class to derive from.

#ifndef __vtkOpenGLHardwareSupport_h
#define __vtkOpenGLHardwareSupport_h

#include "vtkObject.h"

class vtkOpenGLExtensionManager;

class VTK_RENDERING_EXPORT vtkOpenGLHardwareSupport : public vtkObject //: public vtkHardwareSupport
{
public:
  vtkTypeMacro(vtkOpenGLHardwareSupport,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkOpenGLHardwareSupport *New();

  // Description:
  // Return the number of fixed-function texture units.
  int GetNumberOfFixedTextureUnits();

  // Description:
  // Return the total number of texture image units accessible by a shader
  // program.
  int GetNumberOfTextureUnits();
  
  // Description:
  // Test if MultiTexturing is supported.
  bool GetSupportsMultiTexturing();

  // Description:
  // Set/Get a reference to a vtkRenderWindow which is Required
  // for most methods of this class to work.
  vtkGetObjectMacro(ExtensionManager, vtkOpenGLExtensionManager);
  void SetExtensionManager(vtkOpenGLExtensionManager* extensionManager);
  
protected:
  vtkOpenGLHardwareSupport();
  ~vtkOpenGLHardwareSupport();
  
private:
  vtkOpenGLHardwareSupport(const vtkOpenGLHardwareSupport&);  // Not implemented.
  void operator=(const vtkOpenGLHardwareSupport&);  // Not implemented.

  bool ExtensionManagerSet();

  vtkOpenGLExtensionManager* ExtensionManager;
};

#endif
