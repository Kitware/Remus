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
//  Copyright 2012 Sandia Corporation.
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//=============================================================================
#ifndef __remus_testing_Testing_h
#define __remus_testing_Testing_h

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>


/// Asserts a condition for a test to pass. A passing condition is when \a
/// condition resolves to true. If \a condition is false, then the test is
/// aborted and failure is returned.

#define REMUS_ASSERT(condition) \
  if(!condition) { std::cerr << "Error at file: "  << __FILE__ << std::endl; \
    std::cerr<<  "On line: " <<  __LINE__ << std::endl; exit(1); }


#endif //__remus_testing_Testing
