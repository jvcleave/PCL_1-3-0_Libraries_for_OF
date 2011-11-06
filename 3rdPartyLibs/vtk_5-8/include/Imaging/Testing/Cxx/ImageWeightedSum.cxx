/*=========================================================================

  Program:   Visualization Toolkit
  Module:    ImageWeightedSum.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageReader.h"
#include "vtkImageWeightedSum.h"
#include "vtkDoubleArray.h"
#include "vtkImageMathematics.h"
#include "vtkImageData.h"
#include "vtkImageShiftScale.h"
#include "vtkStructuredPointsWriter.h"

#include "vtkTestUtilities.h"
#include "vtkRegressionTestImage.h"

int ImageWeightedSum(int argc, char *argv[])
{
  int rval = 0;
  char* fname =
    vtkTestUtilities::ExpandDataFileName(argc, argv, "Data/headsq/quarter");

  vtkImageReader *reader = vtkImageReader::New();
  reader->SetDataByteOrderToLittleEndian();
  reader->SetDataExtent(0,63,0,63,1,93);
  reader->SetDataSpacing(3.2, 3.2, 1.5);
  reader->SetFilePrefix(fname);
  reader->SetDataMask(0x7fff);

  delete [] fname;

  // Test when weight is equal to 0
  vtkImageWeightedSum *sum = vtkImageWeightedSum::New();
  sum->SetWeight(0,0.);
  sum->AddInputConnection( reader->GetOutputPort() );
  sum->Update();
  double range[2];
  sum->GetOutput()->GetScalarRange( range );
  if( range[0] != 0 || range[1] != 0 )
    {
    cerr << "Range: " << range[0] << "," << range[1] << endl;
    rval++;
    }

  // Set dummy values
  vtkDoubleArray *weights = vtkDoubleArray::New();
  weights->SetNumberOfTuples(5);
  weights->SetValue(0, 10.0);
  weights->SetValue(1, 20.0);
  weights->SetValue(2, 30.0);
  weights->SetValue(3, 40.0);
  weights->SetValue(4, 50.0);

  // Pass in the same images multiple times
  sum->RemoveAllInputs();
  sum->SetWeights( weights );
  sum->AddInputConnection( reader->GetOutputPort() );
  sum->AddInputConnection( reader->GetOutputPort() );
  sum->AddInputConnection( reader->GetOutputPort() );
  sum->AddInputConnection( reader->GetOutputPort() );
  sum->AddInputConnection( reader->GetOutputPort() );

  // Substract the original image
  vtkImageMathematics *math = vtkImageMathematics::New();
  math->SetOperationToSubtract();
  math->SetInput1( reader->GetOutput() );
  math->SetInput2( sum->GetOutput() );
  math->Update();

  math->GetOutput()->GetScalarRange( range );
  if( range[0] != 0 || range[1] != 0 )
    {
    cerr << "Range: " << range[0] << "," << range[1] << endl;
    rval++;
    }

  // Get scalar range:
  reader->GetOutput()->GetScalarRange( range );
  vtkImageShiftScale *shift = vtkImageShiftScale::New();
  shift->SetInputConnection( reader->GetOutputPort() );
  shift->SetScale( 1./(range[1]-range[0]));
  shift->SetShift( -range[0] );
  shift->SetOutputScalarTypeToDouble ();

  // Test multiple scalar type inputs
  sum->RemoveAllInputs();
  weights->SetNumberOfTuples(2);
  weights->SetValue(0, 0.0);
  weights->SetValue(1, 1.0);
  sum->AddInputConnection( reader->GetOutputPort() );
  sum->AddInputConnection( shift->GetOutputPort() );

  math->SetInput1( shift->GetOutput() );
  math->SetInput2( sum->GetOutput() );
  //math->Update();

  //math->GetOutput()->GetScalarRange( range );
  //if( range[0] != 0 || range[1] != 0 )
  //  {
  //  cerr << "Range2: " << range[0] << "," << range[1] << endl;
  //  rval++;
  //  }

  // Cleanup
  reader->Delete();
  weights->Delete();
  sum->Delete();
  math->Delete();
  shift->Delete();

  return rval;
}

