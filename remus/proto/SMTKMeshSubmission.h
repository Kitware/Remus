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

/* SMTKMeshSubmission is designed to allow SMTK based mesh workers to have
* a common submission model. Instead of requiring each worker to explicitly
* know all the key/values that a job submission should have, they can
* instead use SMTKMeshSubmission to simplify the process.
*
*
* Components:
*
* model:
*   Model represents a serialized smtk::model::Manager and all sessions and
*   models within those sessions. Because this representation can hold multiple
*   models, you need to use the modelItemsToMesh to determine what specific
*   models you should be meshing.
*
*   In the future it is planned that this stream will only include sessions
*   that have models to mesh, but currently that has been implemented
*
*   Model is primarily stored in the JSON format with a ContentSource of Memory,
*   but could also be a stored as a file. To deserialize the contents of the
*   model jobcontent you do something like:
*
* ```
  smtk::model::ManagerPtr mgr = smtk::model::Manager::create();
  remus::proto::JobContent model = smtkSubmission.model();
  if(model.sourceType() == remus::common::ContentSource::Memory &&
     model.sourceForamt() == remus::common::ContentFormat::JSON)
    {
     smtk::io::ImportJSON::intoModelManager( model.data(), mgr);
    }
  else if(model.sourceType() == remus::common::ContentSource::File &&
         model.sourceForamt() == remus::common::ContentFormat::JSON)
    {
    std::ifstream file;
    file.open( model.data() );
    if(file.good())
      { //please use faster i/o code than this. This is used for brevity.
      std::string data(
      (std::istreambuf_iterator<char>(file)),
      (std::istreambuf_iterator<char>()));
      smtk::io::ImportJSON::intoModelManager( data, mgr);
      }

    file.close();
    }
* ```
* attributes:
*   Attributes holds all the mesher based attributes for meshing controls. This
*   includes both global sizing, per model element sizing controls, and any
*   other mesher options ( control points, logging, etc )
*
*   Attributes is always a SMTK attribute system which has been serialized to
*   memory. The format could be XML or JSON but since the smtk attribute
*   reader can deduce that information your deserializing code doesn't need
*   to worry about it.
*
* ```
  remus::proto::JobContent attributes = smtkSubmission.attributes();
  smtk::attribute::SystemPtr attSystem( new smtk::attribute::System() );
  attSystem->setRefModelManager(mgr);
  smtk::io::AttributeReader reader;
  smtk::io::Logger inputLogger;

  if(attributes.sourceType() == remus::common::Memory && attributes.dataSize() > 0)
    {
    reader.readContents(*attSystem, attributes.data(), attributes.dataSize(), inputLogger);
    }
* ```
*
* modelItemsToMesh:
*   modelItemsToMesh holds a serialized json string of the form
{
        "ids":  ["2add8c09-01f6-457e-9ed9-a75cc833411a", "5ade8c11-6f06-e754-9ad3-e75dd8334325"]
}
*
*   Which represents the exact subset of models/submodules that should be meshed.
*
*   To deserialize the modelItemsToMesh
* ```
    smtk::common::UUIDArray uuidsToMesh;
    remus::proto::JobContent modelItemsToMesh = smtkSubmission.modelItemsToMesh();
    if(modelItemsToMesh.sourceType() == remus::common::Memory && modelItemsToMesh.dataSize() > 0)
    {
      cJSON* root = cJSON_Parse( modelItemsToMesh.data() );
      cJSON* ids = cJSON_GetObjectItem(root, "ids");
      smtk::io::ImportJSON::getUUIDArrayFromJSON( ids->child, uuidsToMesh );
      cJSON_Delete( root );
    }
* ```
*
*
*/


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
