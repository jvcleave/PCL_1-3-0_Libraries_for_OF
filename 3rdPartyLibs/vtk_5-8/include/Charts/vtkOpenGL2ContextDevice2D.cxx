/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOpenGL2ContextDevice2D.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkOpenGL2ContextDevice2D.h"

#ifdef VTK_USE_QT
# include <QApplication>
# include "vtkQtStringToImage.h"
#endif
#include "vtkFreeTypeStringToImage.h"

#include "vtkVector.h"
#include "vtkRect.h"
#include "vtkPen.h"
#include "vtkBrush.h"
#include "vtkTextProperty.h"
#include "vtkPoints2D.h"
#include "vtkMatrix3x3.h"
#include "vtkFloatArray.h"
#include "vtkSmartPointer.h"

#include "vtkMath.h"
#include "vtkObjectFactory.h"

#include "vtkViewport.h"
#include "vtkWindow.h"

#include "vtkTexture.h"
#include "vtkImageData.h"

#include "vtkRenderer.h"
#include "vtkOpenGLRenderer.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkShaderProgram2.h"
#include "vtkgl.h"

#include "vtkObjectFactory.h"

#include "vtkOpenGLContextDevice2DPrivate.h"

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkOpenGL2ContextDevice2D);

//-----------------------------------------------------------------------------
bool vtkOpenGL2ContextDevice2D::IsSupported(vtkViewport *viewport)
{
  bool supported = false;
  vtkOpenGLRenderer *gl = vtkOpenGLRenderer::SafeDownCast(viewport);
  if (gl)
    {
    vtkOpenGLRenderWindow *win =
        vtkOpenGLRenderWindow::SafeDownCast(gl->GetRenderWindow());
    vtkOpenGLExtensionManager *man = win->GetExtensionManager();
    if (man->ExtensionSupported("GL_VERSION_2_0"))
      {
      supported = true;
      }
    }

  if (supported)
    {
    // Workaround for a bug in mesa - support for non-power of two textures is
    // poor at best. Disable, and use power of two textures for mesa rendering.
    const char *gl_version =
      reinterpret_cast<const char *>(glGetString(GL_VERSION));
    const char *mesa_version = strstr(gl_version, "Mesa");
    if (mesa_version != 0)
      {
      supported = false;
      }
    }

  return supported;
}

//-----------------------------------------------------------------------------
vtkOpenGL2ContextDevice2D::vtkOpenGL2ContextDevice2D()
{
}

//-----------------------------------------------------------------------------
vtkOpenGL2ContextDevice2D::~vtkOpenGL2ContextDevice2D()
{
}

//-----------------------------------------------------------------------------
void vtkOpenGL2ContextDevice2D::DrawPointSprites(vtkImageData *sprite,
                                                 float *points, int n,
                                                 unsigned char *colors,
                                                 int nc_comps)
{
  if (points && n > 0)
    {
    // glColor4ubv(this->Pen->GetColor());
    glPointSize(this->Pen->GetWidth());
    if (sprite)
      {
      if (!this->Storage->SpriteTexture)
        {
        this->Storage->SpriteTexture = vtkTexture::New();
        this->Storage->SpriteTexture->SetRepeat(false);
        }
      this->Storage->SpriteTexture->SetInput(sprite);
      this->Storage->SpriteTexture->Render(this->Renderer);
      }

    // We can actually use point sprites here
    glEnable(vtkgl::POINT_SPRITE);
    glTexEnvi(vtkgl::POINT_SPRITE, vtkgl::COORD_REPLACE, GL_TRUE);
    vtkgl::PointParameteri(vtkgl::POINT_SPRITE_COORD_ORIGIN, vtkgl::LOWER_LEFT);

    this->DrawPoints(points, n, colors, nc_comps);

    glTexEnvi(vtkgl::POINT_SPRITE, vtkgl::COORD_REPLACE, GL_FALSE);
    glDisable(vtkgl::POINT_SPRITE);

    if (sprite)
      {
      this->Storage->SpriteTexture->PostRender(this->Renderer);
      glDisable(GL_TEXTURE_2D);
      }
    }
  else
    {
    vtkWarningMacro(<< "Points supplied without a valid image or pointer.");
    }
}

//-----------------------------------------------------------------------------
void vtkOpenGL2ContextDevice2D::DrawImage(float p[2], float scale,
                                         vtkImageData *image)
{
  this->SetTexture(image);
  this->Storage->Texture->Render(this->Renderer);
  int *extent = image->GetExtent();
  float points[] = { p[0]                    , p[1],
                     p[0]+scale*extent[1]+1.0, p[1],
                     p[0]+scale*extent[1]+1.0, p[1]+scale*extent[3]+1.0,
                     p[0]                    , p[1]+scale*extent[3]+1.0 };

  float texCoord[] = { 0.0, 0.0,
                       1.0, 0.0,
                       1.0, 1.0,
                       0.0, 1.0 };

  glColor4ub(255, 255, 255, 255);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, &points[0]);
  glTexCoordPointer(2, GL_FLOAT, 0, &texCoord[0]);
  glDrawArrays(GL_QUADS, 0, 4);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  this->Storage->Texture->PostRender(this->Renderer);
  glDisable(GL_TEXTURE_2D);
}

//-----------------------------------------------------------------------------
void vtkOpenGL2ContextDevice2D::DrawImage(const vtkRectf& pos,
                                         vtkImageData *image)
{
  GLuint index = this->Storage->TextureFromImage(image);
//  this->SetTexture(image);
//  this->Storage->Texture->Render(this->Renderer);
  float points[] = { pos.X()              , pos.Y(),
                     pos.X() + pos.Width(), pos.Y(),
                     pos.X() + pos.Width(), pos.Y() + pos.Height(),
                     pos.X()              , pos.Y() + pos.Height() };

  float texCoord[] = { 0.0, 0.0,
                       1.0, 0.0,
                       1.0, 1.0,
                       0.0, 1.0 };

  glColor4ub(255, 255, 255, 255);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, &points[0]);
  glTexCoordPointer(2, GL_FLOAT, 0, &texCoord[0]);
  glDrawArrays(GL_QUADS, 0, 4);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

//  this->Storage->Texture->PostRender(this->Renderer);
  glDisable(GL_TEXTURE_2D);
  glDeleteTextures(1, &index);
}

//----------------------------------------------------------------------------
void vtkOpenGL2ContextDevice2D::ReleaseGraphicsResources(vtkWindow *window)
{
  this->vtkOpenGLContextDevice2D::ReleaseGraphicsResources(window);
}

//-----------------------------------------------------------------------------
bool vtkOpenGL2ContextDevice2D::LoadExtensions(vtkOpenGLExtensionManager *m)
{
  if(m->ExtensionSupported("GL_VERSION_2_0"))
    {
    m->LoadExtension("GL_VERSION_2_0");
    this->Storage->OpenGL20 = true;
    this->Storage->PowerOfTwoTextures = false;
    }
  else
    {
    this->Storage->OpenGL20 = false;
    }

  this->Storage->GLExtensionsLoaded = true;

  return this->Storage->OpenGL20;
}

//-----------------------------------------------------------------------------
void vtkOpenGL2ContextDevice2D::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
