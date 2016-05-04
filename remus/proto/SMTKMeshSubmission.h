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

#ifndef remus_proto_SMTKMeshSubmission_h
#define remus_proto_SMTKMeshSubmission_h


#include <remus/proto/JobSubmission.h>
#include <remus/proto/JobContent.h>
#include <remus/proto/JobRequirements.h>

#include <remus/proto/ProtoExports.h>

namespace remus{
namespace proto{

class REMUSPROTO_EXPORT SMTKMeshSubmission : public remus::proto::JobSubmission
{
public:
  //construct an invalid SMTKMeshSubmission. This constructor is designed
  //to allows this class to be stored in containers.
  SMTKMeshSubmission();

  SMTKMeshSubmission( const remus::proto::JobRequirements& reqs );
  //converts a JobSubmission into a SMTKMeshSubmission
  SMTKMeshSubmission(const remus::proto::JobSubmission& submission);

  //returns if this SMTKMeshSubmission has all the required fields
  //we require all three keys (model, attribute, modelItems) be filled in
  //currently
  bool hasAllComponents() const;

  const std::string& model_key( ) const { return this->ModelKey; }
  const std::string& attribute_key( ) const { return this->AttributeKey; }
  const std::string& modelItems_key( ) const { return this->ModelItemKey; }

  //Zero copy creation, content needs to exist while this instance
  //of SMTKMeshSubmission exists
  void model(const char* content, std::size_t len, remus::common::ContentFormat::Type format);

  //Copy creation, content doesn't to exist while this instance
  //of SMTKMeshSubmission exists
  void model(const std::string& content, remus::common::ContentFormat::Type);
  void model(const remus::proto::JobContent& content);

  //Zero copy creation, content needs to exist while this instance
  //of SMTKMeshSubmission exists
  void attributes(const char* content, std::size_t len, remus::common::ContentFormat::Type format);

  //Copy creation, content doesn't to exist while this instance
  //of SMTKMeshSubmission exists
  void attributes(const std::string& content, remus::common::ContentFormat::Type format);
  void attributes(const remus::proto::JobContent& content);

  //Zero copy creation, content needs to exist while this instance
  //of SMTKMeshSubmission exists
  void modelItemsToMesh(const char* content, std::size_t len, remus::common::ContentFormat::Type);

  //Copy creation, content doesn't to exist while this instance
  //of SMTKMeshSubmission exists
  void modelItemsToMesh(const std::string& content, remus::common::ContentFormat::Type);
  void modelItemsToMesh(const remus::proto::JobContent& content);

  //Getters if this isn't a valid SMTKMeshSubmission they will return empty
  //JobContents
  remus::proto::JobContent model() const;
  remus::proto::JobContent attributes() const;
  remus::proto::JobContent modelItemsToMesh() const;

  //needed to decode the object from the wire
  friend std::istream& operator>>(std::istream &is,
                                  SMTKMeshSubmission &submission)
    { submission = SMTKMeshSubmission(is); return is; }

private:
  //we don't need a serialize constructor as that is handled by our parent
  //deserialize constructor function
  explicit SMTKMeshSubmission(std::istream& buffer);

  std::string ModelKey;
  std::string AttributeKey;
  std::string ModelItemKey;
};

//------------------------------------------------------------------------------
REMUSPROTO_EXPORT
remus::proto::SMTKMeshSubmission to_SMTKMeshSubmission(const char* data, std::size_t size);

//------------------------------------------------------------------------------
inline remus::proto::SMTKMeshSubmission to_SMTKMeshSubmission(const std::string& msg)
{
  return to_SMTKMeshSubmission(msg.c_str(), msg.size());
}

}
}

#endif
