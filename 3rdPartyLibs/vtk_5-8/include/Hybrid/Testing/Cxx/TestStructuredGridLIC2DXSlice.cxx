/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestStructuredGridLIC2DXSlice.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "TestStructuredGridLIC2DSlice.h"
#include <vtksys/SystemTools.hxx>

int TestStructuredGridLIC2DXSlice(int argc, char* argv[])
{
  ZoomFactor    = 3.0;
  RenderingMode = STRUCTURED_GRID_LIC2D_SLICE_TEST;
  
  char* fname =  
    vtkTestUtilities::ExpandDataFileName(argc, argv, "Data/timestep_0_15.vts");

  vtkstd::string filename = fname;
  filename = "--data=" + filename;

  fname = vtkTestUtilities::ExpandDataFileName(argc, argv, "Data/noise.png");
  vtkstd::string noise = fname;
  noise = "--noise=" + noise;

  char** new_argv = new char*[argc+10];
  for (int cc=0; cc < argc; cc++)
    {
    new_argv[cc] = vtksys::SystemTools::DuplicateString(argv[cc]);
    }
  new_argv[argc++] = vtksys::SystemTools::DuplicateString(filename.c_str());
  new_argv[argc++] = vtksys::SystemTools::DuplicateString(noise.c_str());
  new_argv[argc++] = vtksys::SystemTools::DuplicateString("--mag=8");
  new_argv[argc++] = vtksys::SystemTools::DuplicateString("--partitions=1");
  new_argv[argc++] = vtksys::SystemTools::DuplicateString("--num-steps=100");
  new_argv[argc++] = vtksys::SystemTools::DuplicateString("--slice-dir=0");
  new_argv[argc++] = vtksys::SystemTools::DuplicateString("--slice=98");
  int status = ::StructuredGridLIC2DSlice(argc, new_argv);
  for (int kk=0; kk < argc; kk++)
    {
    delete [] new_argv[kk];
    }
  delete [] new_argv;
  return status;
}
