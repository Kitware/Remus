//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#ifndef TetGenResult_h
#define TetGenResult_h

#include <string>
#include <sstream>
#include <vector>

#include <remus/client/JobResult.h>

#include "TetGenInput.h"

// The TetGenResult class has two purposes.
// For the client side interface it is what the users gets back after
// request a meshing job with the TetGen input data
// For the worker it is how we pack the data from the TetGen process
// and send it back inside a remus::JobResult

//For now the results will really only works with poly, and vtk input formats
class TetGenResult
{
public:
  enum OutputFileType
  {
    POLY_TYPE=2,
    VTK_TYPE=7
  };

  TetGenResult( const std::string& file_base, OutputFileType type );

  TetGenResult(const remus::client::JobResult& result);

  //returns the base file path that is used to
  std::string baseFilePath() const { return this->FileBase; }

  //returns all the  files that the tetgen mesher has written.
  std::vector<std::string> resultFiles() const;

  //casting operator to convert this class into a string
  operator std::string(void) const;

private:
  std::string FileBase;
  OutputFileType FileType;

};

//------------------------------------------------------------------------------
inline TetGenResult::TetGenResult( const std::string& file_base,
                                    OutputFileType type ):
    FileBase(file_base),
    FileType(type)
{
}


//------------------------------------------------------------------------------
inline TetGenResult::TetGenResult(const remus::client::JobResult& result)
{
  //convert the data into a massive string
  std::stringstream buffer(result.Data);

  buffer >> this->FileBase;

  int type;
  buffer >> type;
  this->FileType = static_cast<TetGenResult::OutputFileType>(type);
}

  //returns all the  files that the tetgen mesher has written.
//------------------------------------------------------------------------------
inline std::vector<std::string> TetGenResult::resultFiles() const
{

  std::vector<std::string> result_files;
  if(this->FileType == TetGenResult::VTK_TYPE)
    {
    result_files.push_back( this->FileBase + ".vtk" );
    }
  else if(this->FileType == TetGenResult::POLY_TYPE)
    {
    result_files.push_back( this->FileBase + ".ele" );
    result_files.push_back( this->FileBase + ".face" );
    result_files.push_back( this->FileBase + ".node" );
    }
  return result_files;
}

//casting operator to convert this class into a string
//------------------------------------------------------------------------------
inline TetGenResult::operator std::string(void) const
{
  //convert the data into a massive string
  std::stringstream buffer;

  buffer << FileBase << std::endl;
  buffer << FileType << std::endl;

  return buffer.str();
}

#endif
