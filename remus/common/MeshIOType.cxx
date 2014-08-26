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

#include <remus/common/MeshIOType.h>

namespace remus {
namespace common {

//------------------------------------------------------------------------------
MeshIOType::MeshIOType():
  InputName(),
  OutputName()
{
}

//------------------------------------------------------------------------------
MeshIOType::MeshIOType(const std::string& in, const std::string& out):
  InputName(in),
  OutputName(out)
{
}

//------------------------------------------------------------------------------
MeshIOType::MeshIOType(const boost::shared_ptr<remus::meshtypes::MeshTypeBase>& in,
             const boost::shared_ptr<remus::meshtypes::MeshTypeBase>& out):
  InputName(in->name()),
  OutputName(out->name())
{
}

//------------------------------------------------------------------------------
MeshIOType::MeshIOType(const remus::meshtypes::MeshTypeBase& in,
                       const remus::meshtypes::MeshTypeBase& out):
  InputName(in.name()),
  OutputName(out.name())
{
}

//------------------------------------------------------------------------------
bool MeshIOType::operator ==(const MeshIOType& b) const
{
  return (this->inputType() == b.inputType() &&
          this->outputType() == b.outputType());
}

//------------------------------------------------------------------------------
bool MeshIOType::operator <(const MeshIOType& b) const
  {
  if( this->inputType() == b.inputType() )
    { return ( this->outputType() < b.outputType() ); }
  else
    { return ( this->inputType() < b.inputType() ); }
  }

//------------------------------------------------------------------------------
void MeshIOType::serialize(std::ostream& buffer) const
{
  buffer << this->inputType() << std::endl;
  buffer << this->outputType() << std::endl;
}

//------------------------------------------------------------------------------
MeshIOType::MeshIOType(std::istream& buffer)
{
  buffer >> this->InputName;
  buffer >> this->OutputName;
}

//------------------------------------------------------------------------------
MeshIOTypeSet::MeshIOTypeSet():
Container()
{
}

//------------------------------------------------------------------------------
MeshIOTypeSet::MeshIOTypeSet(const ContainerType& container):
Container(container)
{
}

//------------------------------------------------------------------------------
void MeshIOTypeSet::serialize(std::ostream& buffer) const
{
  buffer << this->Container.size() << std::endl;
  typedef MeshIOTypeSet::ContainerType::const_iterator IteratorType;
  for(IteratorType i = this->Container.begin();
      i != this->Container.end(); ++i)
    {
    buffer << *i << std::endl;
    }
}

//------------------------------------------------------------------------------
MeshIOTypeSet::MeshIOTypeSet(std::istream& buffer)
{
  std::size_t csize = 0;
  buffer >> csize;
  for(std::size_t i = 0; i < csize; ++i)
    {
    MeshIOType m_type;
    buffer >> m_type;
    this->Container.insert(m_type);
    }
}

}
}
