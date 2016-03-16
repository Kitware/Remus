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
#include <memory>
#include <utility>
#include <QApplication>
#include "qcontrols.h"


int main(int argc, char* argv[])
{
  QApplication qtapp(argc, argv);
  qtapp.setApplicationVersion("0.1.0");
  qtapp.setOrganizationName("Kitware");
  qtapp.setApplicationName("Qt Remus example");

  qcontrols controls;
  controls.show();

  return qtapp.exec();
}
