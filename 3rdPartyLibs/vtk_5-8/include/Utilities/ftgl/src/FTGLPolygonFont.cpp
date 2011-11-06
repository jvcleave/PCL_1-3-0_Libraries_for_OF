#include  "FTGLPolygonFont.h"
#include  "FTPolyGlyph.h"
#ifdef FTGL_DEBUG
  #include "mmgr.h"
#endif


FTGLPolygonFont::FTGLPolygonFont()
{}


FTGLPolygonFont::~FTGLPolygonFont()
{}


FTGlyph* FTGLPolygonFont::MakeGlyph( unsigned int g)
{
  FT_Glyph* ftGlyph = face.Glyph( g, FT_LOAD_DEFAULT);

  if( ftGlyph)
  {
    FTPolyGlyph* tempGlyph = new FTPolyGlyph( *ftGlyph);
    return tempGlyph;
  }

  err = face.Error();
  return NULL;
}


