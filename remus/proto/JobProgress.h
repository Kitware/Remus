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
#include <remus/common/StatusTypes.h>

//included for export symbols
#include <remus/proto/ProtoExports.h>

namespace remus {
namespace proto {

//Job progress is a helper class to easily state what the progress of a currently
//running job is. Progress can be numeric, textual or both.
class REMUSPROTO_EXPORT JobProgress
{
public:
  JobProgress();
  explicit JobProgress(remus::STATUS_TYPE status);

  explicit JobProgress(int v);

  explicit JobProgress(const std::string& msg);

  JobProgress(int v, const std::string& msg);

  //overload on the progress object to make it easier to detect when
  //progress has been changed.
  bool operator ==(const JobProgress& b) const
    { return this->Value == b.Value && this->Message == b.Message; }

  //overload on the progress object to make it easier to detect when
  //progress has been changed.
  bool operator !=(const JobProgress& b) const
    { return !(this->operator ==(b)); }

  int value() const { return Value; }
  const std::string& message() const { return Message; }

  void setValue(int v)
    { Value = JobProgress::valid_progress_value(v); }
  void setMessage(const std::string& msg)
    { Message = msg; }

  //make sure that we can't set progress to be outside the valid range.
  int valid_progress_value(int v);

  friend std::ostream& operator<<(std::ostream &os, const JobProgress &prog)
    { prog.serialize(os); return os; }
  friend std::istream& operator>>(std::istream &is, JobProgress &prog)
    { prog = JobProgress(is); return is; }

private:
  //serialize function
  void serialize(std::ostream& buffer) const;

  //deserialize constructor function
  explicit JobProgress(std::istream& buffer);

  int Value;
  std::string Message;
};

}
}
#endif
