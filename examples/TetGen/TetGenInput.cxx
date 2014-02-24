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

#include "TetGenInput.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/case_conv.hpp>


#include <sstream>

//----------------------------------------------------------------------------
TetGenInput::TetGenInput( const std::string& fileName ):
  AbsFilePath(),
  BaseFileName(),
  PathToFile(),
  FileExtension(),
  MergeCoplanarFacets(false),
  MergeCoplanarFacetsCustomToleranceOn(false),
  PerserveSurfaceMesh(false),
  HaveRegionAttributes(false),
  Verbose(false),
  VolumeConstraintOn(false),
  MergeCoplanarFacetsTolerance(0.0E8),
  VolumeConstraint(0)
{
  //we need to split the file name into three parts
  //the baseFileName, the filePath, and lastly the fileExtension
  boost::filesystem::path tetgen_file(fileName);

  if (boost::filesystem::exists(tetgen_file) &&
      boost::filesystem::is_regular_file(tetgen_file))
    {
    this->PathToFile = tetgen_file.parent_path().string();
    this->BaseFileName = tetgen_file.stem().string();
    this->FileExtension = tetgen_file.extension().string();

    tetgen_file = boost::filesystem::absolute(tetgen_file);
    this->AbsFilePath = tetgen_file.string();
    }

}

//----------------------------------------------------------------------------
TetGenInput::TetGenInput(const remus::worker::Job& job):
  AbsFilePath(),
  BaseFileName(),
  PathToFile(),
  FileExtension(),
  MergeCoplanarFacets(false),
  MergeCoplanarFacetsCustomToleranceOn(false),
  PerserveSurfaceMesh(false),
  HaveRegionAttributes(false),
  Verbose(false),
  VolumeConstraintOn(false),
  MergeCoplanarFacetsTolerance(0.0E8),
  VolumeConstraint(0)
{
  //convert the from a string back into the class
  const remus::proto::JobContent& content = job.submission().find("data")->second;
  std::stringstream buffer( std::string(content.data(),content.dataSize()));

  buffer >> AbsFilePath;
  buffer >> BaseFileName;
  buffer >> PathToFile;
  buffer >> FileExtension;
  buffer >> this->MergeCoplanarFacets;
  buffer >> this->MergeCoplanarFacetsCustomToleranceOn;
  buffer >> this->PerserveSurfaceMesh;
  buffer >> this->HaveRegionAttributes;
  buffer >> this->Verbose;
  buffer >> this->VolumeConstraintOn;
  buffer >> this->MergeCoplanarFacetsTolerance;
  buffer >> this->VolumeConstraint;

}

//------------------------------------------------------------------------------
TetGenInput::operator std::string(void) const
{
  //convert the data into a massive string
  std::stringstream buffer;

  buffer << AbsFilePath << std::endl;
  buffer << BaseFileName << std::endl;
  buffer << PathToFile << std::endl;
  buffer << FileExtension << std::endl;
  buffer << this->MergeCoplanarFacets << std::endl;
  buffer << this->MergeCoplanarFacetsCustomToleranceOn << std::endl;
  buffer << this->PerserveSurfaceMesh << std::endl;
  buffer << this->HaveRegionAttributes << std::endl;
  buffer << this->Verbose << std::endl;
  buffer << this->VolumeConstraintOn << std::endl;
  buffer << this->MergeCoplanarFacetsTolerance << std::endl;
  buffer << this->VolumeConstraint << std::endl;

  return buffer.str();
}
