/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkQtChartKeyboardPan.cxx

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

/// \file vtkQtChartKeyboardPan.cxx
/// \date February 23, 2009

#ifdef _MSC_VER
// Disable warnings that Qt headers give.
#pragma warning(disable:4127)
#endif

#include "vtkQtChartKeyboardPan.h"

#include "vtkQtChartArea.h"
#include "vtkQtChartContentsSpace.h"


//-----------------------------------------------------------------------------
vtkQtChartKeyboardPan::vtkQtChartKeyboardPan(QObject *parentObject)
  : vtkQtChartKeyboardFunction(parentObject)
{
}

void vtkQtChartKeyboardPan::activate()
{
  if(this->Chart)
    {
    vtkQtChartContentsSpace *space = this->Chart->getContentsSpace();
    space->panRight();
    }
}


//-----------------------------------------------------------------------------
vtkQtChartKeyboardPanLeft::vtkQtChartKeyboardPanLeft(QObject *parentObject)
  : vtkQtChartKeyboardFunction(parentObject)
{
}

void vtkQtChartKeyboardPanLeft::activate()
{
  if(this->Chart)
    {
    vtkQtChartContentsSpace *space = this->Chart->getContentsSpace();
    space->panLeft();
    }
}


//-----------------------------------------------------------------------------
vtkQtChartKeyboardPanDown::vtkQtChartKeyboardPanDown(QObject *parentObject)
  : vtkQtChartKeyboardFunction(parentObject)
{
}

void vtkQtChartKeyboardPanDown::activate()
{
  if(this->Chart)
    {
    vtkQtChartContentsSpace *space = this->Chart->getContentsSpace();
    space->panDown();
    }
}


//-----------------------------------------------------------------------------
vtkQtChartKeyboardPanUp::vtkQtChartKeyboardPanUp(QObject *parentObject)
  : vtkQtChartKeyboardFunction(parentObject)
{
}

void vtkQtChartKeyboardPanUp::activate()
{
  if(this->Chart)
    {
    vtkQtChartContentsSpace *space = this->Chart->getContentsSpace();
    space->panUp();
    }
}


