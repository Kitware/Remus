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

#ifndef remus_proto_JobProgress_h
#define remus_proto_JobProgress_h

#include <string>
#include <sstream>

#include <remus/common/remusGlobals.h>
#include <remus/proto/conversionHelpers.h>

namespace remus {
namespace proto {

//Job progress is a helper class to easily state what the progress of a currently
//running job is. Progress can be numeric, textual or both.
struct JobProgress
{
  JobProgress():
    Value(-1),
    Message()
    {}

  explicit JobProgress(remus::STATUS_TYPE status):
    Value(-1),
    Message()
    {
    if(status==remus::IN_PROGRESS)
      {//if in_progress than make value 0
      this->Value = 0;
      }
    }

  explicit JobProgress(int v):
    Value(valid_progress_value(v)),
    Message()
    {}

  explicit JobProgress(const std::string& msg):
    Value(0),
    Message(msg)
    {}

  JobProgress(int v, const std::string& msg):
    Value(valid_progress_value(v)),
    Message(msg)
    {}

  //overload on the progress object to make it easier to detect when
  //progress has been changed.
  bool operator ==(const JobProgress& b) const
  {
    return this->Value == b.Value && this->Message == b.Message;
  }

  //overload on the progress object to make it easier to detect when
  //progress has been changed.
  bool operator !=(const JobProgress& b) const
  {
    return !(this->operator ==(b));
  }

  int value() const { return Value; }
  const std::string message() const { return Message; }

  void setValue(int v) { Value = JobProgress::valid_progress_value(v); }
  void setMessage(const std::string& msg) { Message = msg; }


  //make sure that we can't set progress to be outside the valid range.
  static inline int valid_progress_value(int v)
  {
    v = std::min<int>(v,100);
    v = std::max<int>(v,1);
    return v;
  }

  friend std::ostream& operator<<(std::ostream &os, const JobProgress &prog)
    { prog.serialize(os); return os; }
  friend std::istream& operator>>(std::istream &is, JobProgress &prog)
    { prog = JobProgress(is); return is; }

private:
  //serialize function
  void serialize(std::ostream& buffer) const
  {
    buffer << this->value() << std::endl;
    buffer << this->message().size() << std::endl;
    remus::internal::writeString(buffer,this->message());
  }

  //deserialize constructor function
  explicit JobProgress(std::istream& buffer)
  {
    //this is really important, the progress message can have multiple words and/or
    //new line characters. so we want all of the left over characters in the
    //buffer to be the progress message.
    int progressMessageLen;
    buffer >> this->Value;
    buffer >> progressMessageLen;
    this->Message = remus::internal::extractString(buffer,progressMessageLen);
  }

  int Value;
  std::string Message;
};

}
}
#endif
