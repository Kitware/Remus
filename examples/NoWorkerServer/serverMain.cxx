/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <remus/server/Server.h>
#include <remus/server/WorkerFactory.h>

#include <remus/proto/JobRequirements.h>

namespace factory
{
//we want a custom factory that acts like it can create workers when queried
//but actually doesn't when asked too. This will allow the server to queue
//jobs when no worker is connected, instead of rejecting all jobs that are
//submitted
class DoNothingFactory: public remus::server::WorkerFactory
{
public:

  remus::common::MeshIOTypeSet supportedIOTypes() const
  {
    //we return that we support all types!
    remus::common::MeshIOTypeSet tmp( remus::testing::GenerateAllIOTypes() );
    return tmp;
  }

  remus::proto::JobRequirementsSet workerRequirements(
                                          remus::common::MeshIOType type) const
  {
    //return a fake set of requirements which makes it look like we can mesh
    //the given input & output type.
    remus::proto::JobRequirementsSet result;
    remus::proto::JobRequirements reqs =
        remus::proto::make_JobRequirements(type, std::string(), std::string());
    result.insert(reqs);
    return result;
  }

  bool haveSupport(const remus::proto::JobRequirements& reqs) const
    {
    (void) reqs;
    //we want to return true here so that the server always queues
    return true;
    }

  bool createWorker(const remus::proto::JobRequirements& type,
                    WorkerFactory::FactoryDeletionBehavior lifespan)
    {
    (void) type;
    (void) lifespan;
    //we want to return false here so that server never thinks we are creating
    //a worker and assigns a job to a worker we didn't create
    return false;
    }
};

}

int main ()
{
  //create a custom worker factory that creates no workers but says it can mesh
  //so that we queue jobs even when we have no workers connected.
  boost::shared_ptr<factory::DoNothingFactory> factory(new factory::DoNothingFactory());
  factory->setMaxWorkerCount(1);

  //create a default server with the factory
  remus::server::Server b(factory);

  //start accepting connections for clients and workers
  bool valid = b.startBrokeringWithoutSignalHandling();
  b.waitForBrokeringToFinish();
  return valid ? 0 : 1;
}
