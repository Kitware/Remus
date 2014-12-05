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

#include <cstring> //for memcpy
#include <boost/shared_array.hpp>
#include <boost/make_shared.hpp>
#include <vector>


namespace remus {
namespace common {

//a special struct that will copy the data passed in, only if the item
//coming in has a size greater than zero.
struct ConditionalStorage
{
  //construct an empty storage container
  ConditionalStorage():
    Space(),
    Size(0)
  {
  }

  //construct a storage container which holds a reference to the shared_array
  //that is passed in. This is a way to create a ConditionalStorage that
  //uses existing allocated memory
  ConditionalStorage(const boost::shared_array<char>& t,
                     std::size_t size):
    Space(t),
    Size(size)
  {
  }

  //T here needs to be support the .size() and .data() methods
  //this will copy the contents of T into memory owned by ConditionalStorage
  template<typename T>
  ConditionalStorage(const T& t):
    Space(),
    Size(t.size())
  { //copy the contents of t into our storage
  if(this->Size > 0)
    {
    this->Space = boost::shared_array<char>( new char[this->Size] );
    std::memcpy(this->Space.get(),t.data(),t.size());
    }
  }

  //Because of windows 2008. vector does not have .data()
  template<typename T>
  ConditionalStorage(const std::vector<T>& t):
    Space(),
    Size(t.size())
  { //copy the contents of t into our storage
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
