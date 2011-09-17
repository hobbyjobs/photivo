/*******************************************************************************
**
** Photivo
**
** Copyright (C) 2011 Bernd Schoeler <brjohn@brother-john.net>
**
** This file is part of Photivo.
**
** Photivo is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License version 3
** as published by the Free Software Foundation.
**
** Photivo is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Photivo.  If not, see <http://www.gnu.org/licenses/>.
**
*******************************************************************************/

#ifndef PTFILEMGRWINDOW_h
#define PTFILEMGRWINDOW_h

//==============================================================================

#include <QWidget>
//#include <QTreeView>
#include <QFileSystemModel>

#include "ui_ptFileMgrWindow.h"
#include "ptFileMgrDM.h"

//==============================================================================

class ptFileMgrWindow: public QWidget, public Ui::ptFileMgrWindow {
Q_OBJECT

public:
  explicit ptFileMgrWindow(QWidget *parent = 0);
  ~ptFileMgrWindow();


private:
  QFileSystemModel* m_FSModel;
  ptFileMgrDM*      m_FileMgrDM;

};

//==============================================================================

#endif // PTFILEMGRWINDOW_h
