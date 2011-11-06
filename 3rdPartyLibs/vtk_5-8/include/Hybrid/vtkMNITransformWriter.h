/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMNITransformWriter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*=========================================================================

Copyright (c) 2006 Atamai, Inc.

Use, modification and redistribution of the software, in source or
binary forms, are permitted provided that the following terms and
conditions are met:

1) Redistribution of the source code, in verbatim or modified
   form, must retain the above copyright notice, this license,
   the following disclaimer, and any notices that refer to this
   license and/or the following disclaimer.

2) Redistribution in binary form must include the above copyright
   notice, a copy of this license and the following disclaimer
   in the documentation or with other materials provided with the
   distribution.

3) Modified copies of the source code must be clearly marked as such,
   and must not be misrepresented as verbatim copies of the source code.

THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE SOFTWARE "AS IS"
WITHOUT EXPRESSED OR IMPLIED WARRANTY INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE.  IN NO EVENT SHALL ANY COPYRIGHT HOLDER OR OTHER PARTY WHO MAY
MODIFY AND/OR REDISTRIBUTE THE SOFTWARE UNDER THE TERMS OF THIS LICENSE
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, LOSS OF DATA OR DATA BECOMING INACCURATE
OR LOSS OF PROFIT OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF
THE USE OR INABILITY TO USE THE SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.

=========================================================================*/
// .NAME vtkMNITransformWriter - A writer for MNI transformation files.
// .SECTION Description
// The MNI .xfm file format is used to store geometrical
// transformations.  Three kinds of transformations are supported by
// the file format: affine, thin-plate spline, and grid transformations.
// This file format was developed at the McConnell Brain Imaging Centre
// at the Montreal Neurological Institute and is used by their software.
// .SECTION See Also
// vtkMINCImageWriter vtkMNITransformReader
// .SECTION Thanks
// Thanks to David Gobbi for writing this class and Atamai Inc. for
// contributing it to VTK.

#ifndef __vtkMNITransformWriter_h
#define __vtkMNITransformWriter_h

#include "vtkAlgorithm.h"

class vtkAbstractTransform;
class vtkHomogeneousTransform;
class vtkThinPlateSplineTransform;
class vtkGridTransform;
class vtkCollection;

class VTK_HYBRID_EXPORT vtkMNITransformWriter : public vtkAlgorithm
{
public:
  vtkTypeMacro(vtkMNITransformWriter,vtkAlgorithm);

  static vtkMNITransformWriter *New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the file name.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get the entension for this file format.
  virtual const char* GetFileExtensions() {
    return ".xfm"; }

  // Description:
  // Get the name of this file format.
  virtual const char* GetDescriptiveName() {
    return "MNI Transform"; }

  // Description:
  // Set the transform.
  virtual void SetTransform(vtkAbstractTransform *transform);
  virtual vtkAbstractTransform *GetTransform() {
    return this->Transform; };

  // Description:
  // Add another transform to the file.  The next time that
  // SetTransform is called, all added transforms will be
  // removed.
  virtual void AddTransform(vtkAbstractTransform *transform);

  // Description:
  // Get the number of transforms that will be written.
  virtual int GetNumberOfTransforms();

  // Description:
  // Set comments to be added to the file.
  vtkSetStringMacro(Comments);
  vtkGetStringMacro(Comments);

  // Description:
  // Write the file.
  virtual void Write();

protected:
  vtkMNITransformWriter();
  ~vtkMNITransformWriter();

  char *FileName;
  vtkAbstractTransform *Transform;
  vtkCollection *Transforms;
  char *Comments;

  int WriteLinearTransform(ostream &outfile,
                           vtkHomogeneousTransform *transform);
  int WriteThinPlateSplineTransform(ostream &outfile,
                                    vtkThinPlateSplineTransform *transform);
  int WriteGridTransform(ostream &outfile,
                         vtkGridTransform *transform);

  virtual int WriteTransform(ostream &outfile,
                             vtkAbstractTransform *transform);

  virtual int WriteFile();

  virtual int ProcessRequest(vtkInformation* request,
                             vtkInformationVector** inInfo,
                             vtkInformationVector* outInfo);

private:
  vtkMNITransformWriter(const vtkMNITransformWriter&); // Not implemented
  void operator=(const vtkMNITransformWriter&);  // Not implemented

};

#endif
