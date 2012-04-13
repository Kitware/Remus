/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __omicron_worker_h
#define __omicron_worker_h

#include <meshserver/worker/Worker.h>

namespace meshserver{
namespace worker{
class OmicronWorker : public Worker
{
public:
  //construct a worker that can mesh a single type
  OmicronWorker(meshserver::MESH_TYPE mtype);
  virtual ~Worker();

  void startWorking();

protected:
  void launchOmicron();

  void parseOmicronProgress();  
};

}
}
#endif
