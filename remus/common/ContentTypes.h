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

#ifndef remus_common_ContentTypes_h
#define remus_common_ContentTypes_h

namespace remus{
namespace common{

struct ContentFormat{ enum Type{USER=0, XML=1, JSON=2, BSON=3}; };
struct ContentSource{ enum Type{File=0, Memory=1}; };

} }

#endif
