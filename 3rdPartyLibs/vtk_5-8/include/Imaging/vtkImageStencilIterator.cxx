/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageStencilIterator.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkImageStencilIterator.txx"

#ifndef VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION

template class VTK_IMAGING_EXPORT vtkImageStencilIterator<signed char>;
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<char>;
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<int>;
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<long>;
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<short>;
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<float>;
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<double>;
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<unsigned long>;
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<unsigned short>;
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<unsigned char>;
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<unsigned int>;
#if defined(VTK_TYPE_USE_LONG_LONG)
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<long long>;
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<unsigned long long>;
#endif
#if defined(VTK_TYPE_USE___INT64)
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<__int64>;
template class VTK_IMAGING_EXPORT vtkImageStencilIterator<unsigned __int64>;
#endif

#endif
