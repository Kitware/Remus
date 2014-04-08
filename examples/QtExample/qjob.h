/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef qjob_h
#define qjob_h

#include <QRunnable>

class qjob : public QRunnable
{
  void run();
};

#endif