/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef qcontrols_h
#define qcontrols_h

#include <memory>
#include <utility>
#include <QMainWindow>

#include "qserver.h"
#include "qworker.h"

#include <vector>

namespace Ui { class qcontrolsMainWindow; }

class qcontrols : public QMainWindow
{
  Q_OBJECT
public:
  qcontrols();
  ~qcontrols();

  void closeEvent(QCloseEvent *event);

protected slots:
  void spawnWorker();
  void spawnJob();

private:
  Q_DISABLE_COPY(qcontrols)

  qserver Server;
  std::vector< qworker* > Workers;
  Ui::qcontrolsMainWindow* const Ui;
};
#endif
