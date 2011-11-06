/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkQtChartMouseFunction.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

/// \file vtkQtChartMouseFunction.cxx
/// \date March 11, 2008

#ifdef _MSC_VER
// Disable warnings that Qt headers give.
#pragma warning(disable:4127)
#endif

#include "vtkQtChartMouseFunction.h"

#include "vtkQtChartArea.h"

#include <QCursor>
#include <QMouseEvent>
#include <QRectF>
#include <QWheelEvent>


vtkQtChartMouseFunction::vtkQtChartMouseFunction(QObject *parentObject)
  : QObject(parentObject)
{
  this->OwnsMouse = false;
}

bool vtkQtChartMouseFunction::wheelEvent(QWheelEvent *, vtkQtChartArea *)
{
  return false;
}


