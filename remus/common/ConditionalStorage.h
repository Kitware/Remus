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

#ifndef remus_common_ConditionStorage_h
#define remus_common_ConditionStorage_h

#include <boost/shared_array.hpp>

namespace remus {
namespace common {


//a special struct that will copy the data passed in, only if the item
//coming in has a size greater than zero.
struct ConditionalStorage
{
  ConditionalStorage(): Space(), Size(0) { }

  //T here needs to be support the .size() and .data() methods
  template<typename T>
  ConditionalStorage(const T& t):
    Size(t.size()),
    Space( t.size() > 0 ? new char[t.size()] : NULL )
  { //copy the contents of t into our storage
  memcpy(this->Space.get(),t.data(),t.size());
  }

  std::size_t size() const { return this->Size; }
  const char* get() const { return this->Space.get(); }

  void swap(ConditionalStorage& otherStorage )
  {
  const std::size_t otherSize = otherStorage.Size;
  otherStorage.Size = this->Size;
  this->Size = otherSize;

  this->Space.swap(otherStorage.Space);
  }


private:
  boost::shared_array<char> Space;
  std::size_t Size;
};

}
}

#endif