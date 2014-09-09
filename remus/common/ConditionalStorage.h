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

#include <cstring> //from memcpy
#include <boost/shared_array.hpp>
#include <vector>
#include <cstring>

namespace remus {
namespace common {

//a special struct that will copy the data passed in, only if the item
//coming in has a size greater than zero.
struct ConditionalStorage
{
  ConditionalStorage()
  {
  this->Size=0;
  }

  //T here needs to be support the .size() and .data() methods
  template<typename T>
  ConditionalStorage(const T& t)
  { //copy the contents of t into our storage
  this->Size = t.size();
  if(this->Size > 0)
    {
    this->Space = boost::shared_array<char>( new char[this->Size] );
    std::memcpy(this->Space.get(),t.data(),t.size());
    }
  }

  //Because of windows 2008. vector does not have .data()
  template<typename T>
  ConditionalStorage(const std::vector<T>& t)
  { //copy the contents of t into our storage
  this->Size = t.size();
  if(this->Size > 0)
    {
    this->Space = boost::shared_array<char>( new char[this->Size] );
    std::memcpy(this->Space.get(),&t[0],t.size());
    }
  }

  std::size_t size() const { return this->Size; }

  const char* get() const { return this->Space.get(); }

  const char* data() const { return this->Space.get(); }

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
