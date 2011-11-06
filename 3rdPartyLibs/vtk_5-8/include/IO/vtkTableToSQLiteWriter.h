/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkTableToSQLiteWriter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkTableToSQLiteWriter - store a vtkTable in an SQLite database
// .SECTION Description
// vtkTableToSQLiteWriter reads a vtkTable and inserts it into an SQLite
// database.

#ifndef __vtkTableToSQLiteWriter_h
#define __vtkTableToSQLiteWriter_h

#include "vtkTableToDatabaseWriter.h"

class vtkSQLiteDatabase;

class VTK_IO_EXPORT vtkTableToSQLiteWriter : public vtkTableToDatabaseWriter
{
public:
  static vtkTableToSQLiteWriter *New();
  vtkTypeMacro(vtkTableToSQLiteWriter,vtkTableToDatabaseWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the input to this writer.
  vtkTable* GetInput();
  vtkTable* GetInput(int port);

protected:
   vtkTableToSQLiteWriter();
  ~vtkTableToSQLiteWriter();
  void WriteData();

  virtual int FillInputPortInformation(int port, vtkInformation *info);

  vtkTable *Input;

private:
  vtkTableToSQLiteWriter(const vtkTableToSQLiteWriter&);  // Not implemented.
  void operator=(const vtkTableToSQLiteWriter&);  // Not implemented.
};

#endif
