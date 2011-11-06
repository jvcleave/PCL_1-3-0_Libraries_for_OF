/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestTemporalCacheSimple.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkActor.h"
#include "vtkCommand.h"
#include "vtkCompositeDataPipeline.h"
#include "vtkContourFilter.h"
#include "vtkInformation.h"
#include "vtkCompositePolyDataMapper.h"
#include "vtkPolyDataMapper.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkTemporalDataSet.h"
#include "vtkTemporalDataSetCache.h"
#include "vtkTemporalInterpolator.h"
#include "vtkThreshold.h"
#include "vtkSphereSource.h"
#include "vtkObjectFactory.h"
#include "vtkInformationVector.h"
#include <vtkstd/algorithm>
#include <vtkstd/vector>

//
// This test is intended  to test the ability of the temporal pipeline 
// to loop a simple source over T and pass Temporal data downstream.
//

//-------------------------------------------------------------------------
// This is a dummy class which accepts time from the pipeline
// It doesn't do anything with the time, but it is useful for testing
//-------------------------------------------------------------------------
class vtkTemporalSphereSource : public vtkSphereSource {

public:
  static vtkTemporalSphereSource *New();
  vtkTypeMacro(vtkTemporalSphereSource, vtkSphereSource);

  // Description:
  // Set/Get the time value at which to get the value.
  // These are not used. We get our time from the UPDATE_TIME_STEPS
  // information key
  vtkSetMacro(TimeStep, int);
  vtkGetMacro(TimeStep, int);

  // Save the range of valid timestep index values.
  vtkGetVector2Macro(TimeStepRange, int);

  //BTX
//  void GetTimeStepValues(vtkstd::vector<double> &steps);
  //ETX

 protected:
   vtkTemporalSphereSource();

  virtual int RequestInformation(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector);

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector);

public:
  int TimeStepRange[2];
  int TimeStep;
  int ActualTimeStep;
  vtkstd::vector<double> TimeStepValues;
};
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkTemporalSphereSource);
//----------------------------------------------------------------------------
vtkTemporalSphereSource::vtkTemporalSphereSource()
{
  this->TimeStepRange[0]        = 0;
  this->TimeStepRange[1]        = 0;
  this->TimeStep                = 0;
  this->ActualTimeStep          = 0;
}
//----------------------------------------------------------------------------
int vtkTemporalSphereSource::RequestInformation(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  //
  if (!this->Superclass::RequestInformation(request, inputVector, outputVector)) {
    return 0;
  }
  this->TimeStepRange[0] = 0;
  this->TimeStepRange[1] = 9;
  this->TimeStepValues.resize(this->TimeStepRange[1]-this->TimeStepRange[0]+1);
  for (int i=0; i<=this->TimeStepRange[1]; ++i) 
  {
    this->TimeStepValues[i] = i;
  }

  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), 
    &this->TimeStepValues[0], static_cast<int>(this->TimeStepValues.size()));
  double timeRange[2];
  timeRange[0] = this->TimeStepValues.front();
  timeRange[1] = this->TimeStepValues.back();
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);

  return 1;
}
//----------------------------------------------------------------------------
class vtkTestTemporalCacheSimpleWithinTolerance: public vtkstd::binary_function<double, double, bool>
{
public:
    result_type operator()(first_argument_type a, second_argument_type b) const
    {
      bool result = (fabs(a-b)<=(a*1E-6));
      return (result_type)result;
    }
};
//----------------------------------------------------------------------------
int vtkTemporalSphereSource::RequestData(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkDataObject* doOutput = outInfo->Get(vtkDataObject::DATA_OBJECT());

  this->ActualTimeStep = this->TimeStep;

  if (this->TimeStep==0 && outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS()))
    {
    double requestedTimeValue = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS())[0];
    this->ActualTimeStep = vtkstd::find_if(
      this->TimeStepValues.begin(), 
      this->TimeStepValues.end(), 
      vtkstd::bind2nd( vtkTestTemporalCacheSimpleWithinTolerance( ), requestedTimeValue )) 
      - this->TimeStepValues.begin();
    this->ActualTimeStep = this->ActualTimeStep + this->TimeStepRange[0];
    int N = outInfo->Length(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS());
    doOutput->GetInformation()->Set(vtkDataObject::DATA_TIME_STEPS(), &requestedTimeValue, 1);
    vtkDebugMacro(<<"Got a timestep request from downstream t= " << requestedTimeValue 
      << " Step : " << this->ActualTimeStep << "(Number of steps requested " << N << ")");
    }
  else 
    {
    double timevalue[1];
    timevalue[0] = this->TimeStepValues[this->ActualTimeStep-this->TimeStepRange[0]];
    vtkDebugMacro(<<"Using manually set t= " << timevalue[0] << " Step : " << this->ActualTimeStep);
    doOutput->GetInformation()->Set(vtkDataObject::DATA_TIME_STEPS(), &timevalue[0], 1);
    }

  cout << "this->ActualTimeStep : " << this->ActualTimeStep << endl;

  return Superclass::RequestData(request, inputVector, outputVector);
} 
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
class vtkTestTemporalCacheSimpleExecuteCallback
  : public vtkCommand
{
public:
  static vtkTestTemporalCacheSimpleExecuteCallback *New()
  { return new vtkTestTemporalCacheSimpleExecuteCallback; }
  
  virtual void Execute(vtkObject *caller, unsigned long, void*)
  { 
    // count the number of timesteps requested
    vtkTemporalSphereSource *sph = vtkTemporalSphereSource::SafeDownCast(caller);
    vtkInformation *info = sph->GetExecutive()->GetOutputInformation(0);
    int Length = info->Length(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS());
    this->Count += Length;
    if (Length>0)
      {
      vtkstd::vector<double> steps;
      steps.resize(Length);
      info->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS(), &steps[0]);
      for (int i=0; i<Length; ++i) 
        {
        cout << steps[i] << " ";
        }
      cout << endl;
      }
  }

  unsigned int Count;
};
//-------------------------------------------------------------------------
int TestTemporalCacheSimple(int , char *[])
{
  // we have to use a compsite pipeline
  vtkCompositeDataPipeline* prototype = vtkCompositeDataPipeline::New();
  vtkAlgorithm::SetDefaultExecutivePrototype(prototype);
  prototype->Delete();

  // create temporal fractals
  vtkSmartPointer<vtkTemporalSphereSource> sphere = 
    vtkSmartPointer<vtkTemporalSphereSource>::New();

  vtkTestTemporalCacheSimpleExecuteCallback *executecb
    =vtkTestTemporalCacheSimpleExecuteCallback::New();
  executecb->Count = 0;
  sphere->AddObserver(vtkCommand::StartEvent,executecb);
  executecb->Delete();

  // cache the data to prevent regenerating some of it
  vtkSmartPointer<vtkTemporalDataSetCache> cache = 
    vtkSmartPointer<vtkTemporalDataSetCache>::New();
  cache->SetInputConnection(sphere->GetOutputPort());
  cache->SetCacheSize(10);

  // interpolate if needed
  vtkSmartPointer<vtkTemporalInterpolator> interp = 
    vtkSmartPointer<vtkTemporalInterpolator>::New();
  //interp->SetInputConnection(fractal->GetOutputPort());
  interp->SetInputConnection(cache->GetOutputPort());
  
  // map them
  vtkSmartPointer<vtkCompositePolyDataMapper> mapper = 
    vtkSmartPointer<vtkCompositePolyDataMapper>::New();
  mapper->SetInputConnection(interp->GetOutputPort());
  
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);

  vtkSmartPointer<vtkRenderer> renderer = 
    vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renWin = 
    vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkRenderWindowInteractor> iren = 
    vtkSmartPointer<vtkRenderWindowInteractor>::New();

  renderer->AddActor( actor );
  renderer->SetBackground(0.5, 0.5, 0.5);

  renWin->AddRenderer( renderer );
  renWin->SetSize( 300, 300 ); 
  iren->SetRenderWindow( renWin );
  renWin->Render();

  // ask for some specific data points
  vtkStreamingDemandDrivenPipeline *sdd = 
    vtkStreamingDemandDrivenPipeline::SafeDownCast(interp->GetExecutive());
  double times[1];
  times[0] = 0;
  int i;
  int j;
  for (j = 0; j < 5; ++j)
    {
    for (i = 0; i < 9; ++i)
      {
      times[0] = i+0.5;
//      vtkDebugMacro(<<"SetUpdateTimeSteps t= " << times[0]);
      sdd->SetUpdateTimeSteps(0, times, 1);
      mapper->Modified();
      renderer->ResetCameraClippingRange();
      renWin->Render();
      }
    }

  vtkAlgorithm::SetDefaultExecutivePrototype(0);

  // there is a bug and ExecuteDataStart gets called twice when inside the 
  // Execute Block(Time), so this number is much too high, it should be
  // be 11 at most and prefereble only 10 (but the first time always
  // gets called twice).
  if (executecb->Count == 22)
    {
    return 0;
    }

  return 1;
}
