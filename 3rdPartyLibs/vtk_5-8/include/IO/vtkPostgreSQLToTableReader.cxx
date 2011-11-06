/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPostgreSQLToTableReader.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPostgreSQLDatabase.h"
#include "vtkPostgreSQLQuery.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStringArray.h"
#include "vtkTable.h"

#include "vtkPostgreSQLToTableReader.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPostgreSQLToTableReader);

//----------------------------------------------------------------------------
vtkPostgreSQLToTableReader::vtkPostgreSQLToTableReader()
{
}

//----------------------------------------------------------------------------
vtkPostgreSQLToTableReader::~vtkPostgreSQLToTableReader()
{
}

//----------------------------------------------------------------------------
int vtkPostgreSQLToTableReader::RequestData(vtkInformation *,
                                      vtkInformationVector **,
                                      vtkInformationVector *outputVector)
{
  //Make sure we have all the information we need to provide a vtkTable
  if(!this->Database)
    {
    vtkErrorMacro(<<"No open database connection");
    return 1;
    }
  if(!this->Database->IsA("vtkPostgreSQLDatabase"))
    {
    vtkErrorMacro(<<"Wrong type of database for this reader");
    return 1;
    }
  if(this->TableName == "")
    {
    vtkErrorMacro(<<"No table selected");
    return 1;
    }

  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // Return all data in the first piece ...
  if(outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
    {
    return 1;
    }

  vtkTable* const output = vtkTable::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));

  //perform a query to get the names and types of the columns
  vtkstd::string queryStr =
    "select column_name, data_type FROM information_schema.columns WHERE table_name = '";
  queryStr += this->TableName;
  queryStr += "';";
  vtkPostgreSQLQuery *query =
    static_cast<vtkPostgreSQLQuery*>(this->Database->GetQueryInstance());
  query->SetQuery(queryStr.c_str());
  if(!query->Execute())
    {
    vtkErrorMacro(<<"Error performing 'show columns' query");
    }

  //use the results of the query to create columns of the proper name & type
  vtkstd::vector<std::string> columnTypes;
  while(query->NextRow())
    {
    vtkstd::string columnName = query->DataValue(0).ToString();
    vtkstd::string columnType = query->DataValue(1).ToString();
    if( (columnType.find("int") != vtkstd::string::npos) ||
        (columnType.find("INT") != vtkstd::string::npos) ||
        (columnType.find("serial") != vtkstd::string::npos) ||
        (columnType.find("SERIAL") != vtkstd::string::npos) )
      {
      vtkSmartPointer<vtkIntArray> column =
        vtkSmartPointer<vtkIntArray>::New();
      column->SetName(columnName.c_str());
      output->AddColumn(column);
      columnTypes.push_back("int");
      }
    else if( (columnType.find("double") != vtkstd::string::npos) ||
             (columnType.find("DOUBLE") != vtkstd::string::npos) ||
             (columnType.find("real") != vtkstd::string::npos) ||
             (columnType.find("REAL") != vtkstd::string::npos) ||
             (columnType.find("decimal") != vtkstd::string::npos) ||
             (columnType.find("DECIMAL") != vtkstd::string::npos) ||
             (columnType.find("numeric") != vtkstd::string::npos) ||
             (columnType.find("NUMERIC") != vtkstd::string::npos) )
      {
      vtkSmartPointer<vtkDoubleArray> column =
        vtkSmartPointer<vtkDoubleArray>::New();
      column->SetName(columnName.c_str());
      output->AddColumn(column);
      columnTypes.push_back("double");
      }
    else
      {
      vtkSmartPointer<vtkStringArray> column =
        vtkSmartPointer<vtkStringArray>::New();
      column->SetName(columnName.c_str());
      output->AddColumn(column);
      columnTypes.push_back("string");
      }
    }

  //do a query to get the contents of the PostgreSQL table
  queryStr = "SELECT * FROM ";
  queryStr += this->TableName;
  query->SetQuery(queryStr.c_str());
  if(!query->Execute())
    {
    vtkErrorMacro(<<"Error performing 'select all' query");
    }

  //use the results of the query to populate the columns
  while(query->NextRow())
    {
    for(int col = 0; col < query->GetNumberOfFields(); ++ col)
      {
      if(columnTypes[col] == "int")
        {
        vtkIntArray *column =
          static_cast<vtkIntArray*>(output->GetColumn(col));
        column->InsertNextValue(query->DataValue(col).ToInt());
        }
      else if(columnTypes[col] == "double")
        {
        vtkDoubleArray *column =
          static_cast<vtkDoubleArray*>(output->GetColumn(col));
        column->InsertNextValue(query->DataValue(col).ToDouble());
        }
      else
        {
        vtkStringArray *column =
          static_cast<vtkStringArray*>(output->GetColumn(col));
        column->InsertNextValue(query->DataValue(col).ToString());
        }
      }
    }

  query->Delete();
  return 1;
}

//----------------------------------------------------------------------------
void vtkPostgreSQLToTableReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
