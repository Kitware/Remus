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

#ifndef TetGenInput_h
#define TetGenInput_h

#include <string>
#include <remus/worker/Job.h>

// The TetGenInput class has two purposes.
// For the client side interface it is what the users use to create the
// input data for TetGen.
// For the worker it is how we unpack the data from the string that
// remus::worker::Job has
class TetGenInput
{
public:
  //create a TetGenInput using the input file as the basis for what tetgen
  //is going to mesh.
  //
  //Required:
  // fileName must point to a valid file on the filesystem
  TetGenInput( const std::string& fileName );

  //convert a remus::worker::Job data into a TetGen input
  TetGenInput( const remus::worker::Job& job );

  bool valid_file() const
    { return (this->BaseFileName.size()>0 && this->FileExtension.size()>0); }

  const std::string& fileBaseName() const { return this->BaseFileName; }
  const std::string& fileExt() const { return this->FileExtension; }
  const std::string& filePath() const { return this->PathToFile; }

  const std::string& fullFilePath() const { return this->AbsFilePath; }

  //enable merging of coplanar facets
  void setMergeCoplanarFacets(bool merge)
    { this->MergeCoplanarFacets = merge; }
  bool mergeCoplanarFacets() const
    { return this->MergeCoplanarFacets; }

  //if merging of coplanar facets is enabled use a user provided
  //absolute tolerance instead of the meshers default tolerance
  void setUseCustomMergeCoplanarFacetsTolerance(bool enableCustomTol)
    { this->MergeCoplanarFacetsCustomToleranceOn = enableCustomTol; }
  bool useMergeCoplanarFacetsTolerance() const
    { return this->MergeCoplanarFacetsCustomToleranceOn; }

  //set the user provided coplanar tolerance
  void setCustomMergeCoplanarFacetsTolerance(double tol)
    { this->MergeCoplanarFacetsTolerance = tol; }
  double customMergeCoplanarFacetsTolerance() const
    { return this->MergeCoplanarFacetsTolerance; }

  //enable user defined volume constraint
  bool useVolumeConstraint()const
    { return this->VolumeConstraintOn; }
  void setUseVolumeConstraint(bool useVolCons)
    { this->VolumeConstraintOn=useVolCons; }

  double volumeConstraint() const
    { return this->VolumeConstraint; }
  void setVolumeConstraint(double volCons)
    { this->VolumeConstraint=volCons; }

  //set that we want the input surface mesh in the input file to be
  //saved in the output file
  void setPerserveSurfaceMesh(bool enableSavingInputMesh)
    { this->PerserveSurfaceMesh = enableSavingInputMesh; }
  bool perserveSurfaceMesh() const
    { return this->PerserveSurfaceMesh; }

  //set that the input file has region attributes that we want to use
  //if the input file has the optional region section and this isn't
  //enabled we will ignore the regions
  void setHaveRegionAtributes(bool haveRegionAttributes)
    { this->HaveRegionAttributes = haveRegionAttributes; }
  bool haveRegionAtributes() const
    { return this->HaveRegionAttributes; }

  void setVerbosity(bool verb)
    { this->Verbose=verb; }
  bool verbose() const
    { return this->Verbose; }

  //casting operator to convert this class into a string
  operator std::string(void) const;

private:
  //holds the base fileName for the input file
  //this would be the
  std::string AbsFilePath;
  std::string BaseFileName;
  std::string PathToFile;
  std::string FileExtension;


  bool MergeCoplanarFacets;
  bool MergeCoplanarFacetsCustomToleranceOn;
  bool PerserveSurfaceMesh;
  bool HaveRegionAttributes;
  bool Verbose;
  bool VolumeConstraintOn;

  double MergeCoplanarFacetsTolerance;
  double VolumeConstraint;
};

#endif