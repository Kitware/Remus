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
#include <remus/proto/JobProgress.h>

#include <algorithm>

#include <remus/proto/conversionHelpers.h>

namespace remus {
namespace proto {

//------------------------------------------------------------------------------
JobProgress::JobProgress():
  Value(-1),
  Message()
  {}

//------------------------------------------------------------------------------
JobProgress::JobProgress(remus::STATUS_TYPE status):
  Value( (status==remus::IN_PROGRESS) ? 0 : -1),
  Message()
  {
  }

//------------------------------------------------------------------------------
JobProgress::JobProgress(int v):
  Value(valid_progress_value(v)),
  Message()
  {
  }

//------------------------------------------------------------------------------
JobProgress::JobProgress(const std::string& msg):
  Value(0),
  Message(msg)
  {
  }

//------------------------------------------------------------------------------
JobProgress::JobProgress(int v, const std::string& msg):
  Value(valid_progress_value(v)),
  Message(msg)
  {}

//------------------------------------------------------------------------------
int JobProgress::valid_progress_value(int v)
{
  v = std::min<int>(v,100);
  v = std::max<int>(v,1);
  return v;
}

//------------------------------------------------------------------------------
void JobProgress::serialize(std::ostream& buffer) const
{
  buffer << this->value() << std::endl;
  buffer << this->message().size() << std::endl;
  remus::internal::writeString(buffer,this->message());
}

//------------------------------------------------------------------------------
//deserialize constructor function
JobProgress::JobProgress(std::istream& buffer)
{
  //this is really important, the progress message can have multiple words and/or
  //new line characters. so we want all of the left over characters in the
  //buffer to be the progress message.
  int progressMessageLen;
  buffer >> this->Value;
  buffer >> progressMessageLen;
  this->Message = remus::internal::extractString(buffer,progressMessageLen);
}

}
}
