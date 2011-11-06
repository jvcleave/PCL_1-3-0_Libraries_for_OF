/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDSPFilterGroup.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*----------------------------------------------------------------------------
 Copyright (c) Sandia Corporation
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
----------------------------------------------------------------------------*/
// .NAME vtkDSPFilterGroup - used by the Exodus readers
// .SECTION Description
// vtkDSPFilterGroup is used by vtkExodusReader, vtkExodusIIReader and
// vtkPExodusReader to do temporal smoothing of data
// .SECTION See Also
// vtkDSPFilterDefinition vtkExodusReader vtkExodusIIReader vtkPExodusReader

#ifndef __vtkDSPFilterGroup_h
#define __vtkDSPFilterGroup_h

#include "vtkObject.h"

class vtkDSPFilterGroupVectorIntSTLCloak;
class vtkDSPFilterGroupVectorVectorIntSTLCloak;
class vtkDSPFilterGroupVectorArraySTLCloak;
class vtkDSPFilterGroupVectorVectorArraySTLCloak;
class vtkDSPFilterGroupVectorStringSTLCloak;
class vtkDSPFilterGroupVectorDefinitionSTLCloak;
class vtkFloatArray;
class vtkDSPFilterDefinition;

class VTK_HYBRID_EXPORT vtkDSPFilterGroup : public vtkObject
{
public:
  static vtkDSPFilterGroup *New();
  vtkTypeMacro(vtkDSPFilterGroup,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);


  void AddFilter(vtkDSPFilterDefinition *filter);
  void RemoveFilter(char *a_outputVariableName);

  bool IsThisInputVariableInstanceNeeded( const char *a_name, int a_timestep, int a_outputTimestep );
  bool IsThisInputVariableInstanceCached( const char *a_name, int a_timestep );
  void AddInputVariableInstance( const char *a_name, int a_timestep, vtkFloatArray *a_data );

  vtkFloatArray *GetOutput( int a_whichFilter, int a_whichTimestep, int &a_instancesCalculated );

  vtkFloatArray *GetCachedInput( int a_whichFilter, int a_whichTimestep );
  vtkFloatArray *GetCachedOutput( int a_whichFilter, int a_whichTimestep );

  const char *GetInputVariableName(int a_whichFilter);

  int GetNumFilters();

  void Copy( vtkDSPFilterGroup *other );

  vtkDSPFilterDefinition *GetFilter(int a_whichFilter);

  vtkDSPFilterGroupVectorDefinitionSTLCloak * /*vtkstd::vector<vtkDSPFilterDefinition *>*/ FilterDefinitions;

protected:
  vtkDSPFilterGroup();
  ~vtkDSPFilterGroup();


  vtkDSPFilterGroupVectorArraySTLCloak * /*vtkstd::vector<vtkFloatArray *>*/ CachedInputs;
  vtkDSPFilterGroupVectorStringSTLCloak * /*vtkstd::vector<vtkstd::string>*/ CachedInputNames;
  vtkDSPFilterGroupVectorIntSTLCloak * /*vtkstd::vector<int>*/ CachedInputTimesteps;

  vtkDSPFilterGroupVectorVectorArraySTLCloak * /*vtkstd::vector< vtkstd::vector<vtkFloatArray *> >*/ CachedOutputs;
  vtkDSPFilterGroupVectorVectorIntSTLCloak * /*vtkstd::vector< vtkstd::vector<int> >*/ CachedOutputTimesteps;

private:
  vtkDSPFilterGroup(const vtkDSPFilterGroup&); // Not implemented
  void operator=(const vtkDSPFilterGroup&); // Not implemented
};

#endif
