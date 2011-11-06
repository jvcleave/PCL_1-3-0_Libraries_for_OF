/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkBrush.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// .NAME vtkBrush - provides a brush that fills shapes drawn by vtkContext2D.
//
// .SECTION Description
// The vtkBrush defines the fill (or pattern) of shapes that are drawn by
// vtkContext2D. The color is stored as four unsigned chars (RGBA), where the
// opacity defaults to 255, but can be modified separately to the other
// components. Ideally we would use a lightweight color class to store and pass
// around colors.

#ifndef __vtkBrush_h
#define __vtkBrush_h

#include "vtkObject.h"
#include "vtkColor.h" // Needed for vtkColor4ub

class vtkImageData;

class VTK_CHARTS_EXPORT vtkBrush : public vtkObject
{
public:
  vtkTypeMacro(vtkBrush, vtkObject);
  virtual void PrintSelf(ostream &os, vtkIndent indent);

  static vtkBrush *New();

  // Description:
  // Set the color of the brush with three component doubles (RGB), ranging from
  // 0.0 to 1.0.
  void SetColorF(double color[3]);

  // Description:
  // Set the color of the brush with three component doubles (RGB), ranging from
  // 0.0 to 1.0.
  void SetColorF(double r, double g, double b);

  // Description:
  // Set the color of the brush with four component doubles (RGBA), ranging from
  // 0.0 to 1.0.
  void SetColorF(double r, double g, double b, double a);

  // Description:
  // Set the opacity with a double, ranging from 0.0 (transparent) to 1.0
  // (opaque).
  void SetOpacityF(double a);

  // Description:
  // Set the color of the brush with three component unsigned chars (RGB),
  // ranging from 0 to 255.
  void SetColor(unsigned char color[3]);

  // Description:
  // Set the color of the brush with three component unsigned chars (RGB),
  // ranging from 0 to 255.
  void SetColor(unsigned char r, unsigned char g, unsigned char b);

  // Description:
  // Set the color of the brush with four component unsigned chars (RGBA),
  // ranging from 0 to 255.
  void SetColor(unsigned char r, unsigned char g, unsigned char b,
                unsigned char a);
  void SetColor(const vtkColor4ub &color);

  // Description:
  // Set the opacity with an unsigned char, ranging from 0 (transparent) to 255
  // (opaque).
  void SetOpacity(unsigned char a);

  // Description:
  // Get the color of the brush - expects a double of length 4 to copy into.
  void GetColorF(double color[4]);

  // Description:
  // Get the color of the brush - expects an unsigned char of length 4.
  void GetColor(unsigned char color[4]);

  // Description:
  // Get the color of the brush - gives a pointer to the underlying data.
  unsigned char * GetColor() { return &this->Color[0]; }

  // Description:
  // Get the color of the brush.
  vtkColor4ub GetColorObject();

  // Description:
  // Set the texture that will be used to fill polygons
  // By default, no texture is set. The image will be registered with the brush
  // (ref count is incremented)
  // To disable the texture, set Texture to 0.
  void SetTexture(vtkImageData* image);

  // Description
  // Get the texture that is used to fill polygons
  vtkGetObjectMacro(Texture, vtkImageData);

  // Description
  // Texture properties
  enum TextureProperty {
    Nearest = 0x01,
    Linear  = 0x02,
    Stretch = 0x04,
    Repeat  = 0x08
  };

  // Description
  // Set properties to the texture
  // By default, the texture is linearly stretched.
  // The behavior is undefined when Linear and Nearest are both set
  // The behavior is undefined when Stretch and Repeat are both set
  // The behavior is undefined if TextureProperties is 0
  vtkSetMacro(TextureProperties, int);

  // Description
  // Get the properties associated to the texture
  vtkGetMacro(TextureProperties, int);

  // Description:
  // Make a deep copy of the supplied brush.
  void DeepCopy(vtkBrush *brush);

//BTX
protected:
  vtkBrush();
  ~vtkBrush();

  // Storage of the color in RGBA format (0-255 per channel).
  unsigned char* Color;
  vtkColor4ub    BrushColor;
  vtkImageData*  Texture;
  int            TextureProperties;

private:
  vtkBrush(const vtkBrush &); // Not implemented.
  void operator=(const vtkBrush &);   // Not implemented.
//ETX
};

#endif //__vtkBrush_h
