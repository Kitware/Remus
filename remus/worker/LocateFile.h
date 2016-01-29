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

#ifndef remus_worker_LocateFile_h
#define remus_worker_LocateFile_h

#include <remus/common/FileHandle.h>
//included for export symbol
#include <remus/worker/WorkerExports.h>

#include <string>
#include <vector>

namespace remus {
namespace worker {

//One of the more common things a worker needs to do is locate and
//read a file from disk. This is a series of helper methods that
//allow workers to do exactly that.


//This method returns the absolute location of this workers executable on disk
//this can than be used as starting location to find other files.
//On most platforms this will be the current working directory, but for
//OSX bundles we detect the proper root of the bundle
REMUSWORKER_EXPORT
std::string getExecutableLocation();

//This returns a collection of relative paths patterns that commonly
//encountered when packaging or building executables. These when combined
//with the executable location should build a very good set of places to
//search for file both in an installed package, and as part of a build directory
REMUSWORKER_EXPORT
std::vector<std::string> relativeLocationsToSearch();


//Uses the getExecutableLocation and relativeLocationsToSearch methods
//to build a set of locations to search for a given file.
//
//input example:
// name = "foo", ext = "txt" = we search for "foo.txt"
// name = "foo", ext = ".txt" = we search for "foo.txt"
// name = "foo.", ext = ".txt" = we search for "foo..txt"
// name = "foo", ext = "" = we search for foo
// name = "", ext = "txt" = we don't search at all, we require a name
//
//
REMUSWORKER_EXPORT
remus::common::FileHandle findFile( const std::string& name,
                                    const std::string& ext );


//Uses the getExecutableLocation and relativeLocationsToSearch methods
//to build a set of locations to search for a given file. The caller can
//also add more relative and absolute locations to search for.
//Each relative location will be added to getExecutableLocation and each
//entry in absoluteLocation
//
//input example:
// name = "foo", ext = "txt" = we search for "foo.txt"
// name = "foo", ext = ".txt" = we search for "foo.txt"
// name = "foo.", ext = ".txt" = we search for "foo..txt"
// name = "foo", ext = "" = we search for foo
// name = "", ext = "txt" = we don't search at all, we require a name
//
//
REMUSWORKER_EXPORT
remus::common::FileHandle findFile( const std::string& name,
                                    const std::string& ext,
                                    const std::vector<std::string>& relativeLocations,
                                    const std::vector<std::string>& absoluteLocations );

}
}

#endif