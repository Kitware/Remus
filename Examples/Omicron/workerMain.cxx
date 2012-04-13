/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "OmicronWorker.h"

int main (int argc, char* argv[])
{
  OmicronWorker worker;
  worker.setOmicronExecutableName("model");
  worker.meshJob();
  return 1;
}
