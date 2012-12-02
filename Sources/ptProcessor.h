/*******************************************************************************
**
** Photivo
**
** Copyright (C) 2008 Jos De Laender <jos.de_laender@telenet.be>
** Copyright (C) 2009,2010 Michael Munzert <mail@mm-log.com>
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

#ifndef DLPROCESSOR_H
#define DLPROCESSOR_H

#include <QString>
#include <QTime>
#include <QCoreApplication>

#include <exiv2/exif.hpp>

#include "ptDcRaw.h"
#include "ptImage.h"


class ptProcessor {
Q_DECLARE_TR_FUNCTIONS(ptProcessor)


///////////////////////////////////////////////////////////////////////////
//
// PUBLIC members
//
///////////////////////////////////////////////////////////////////////////
public:
  // Cached image versions at different points.
  ptImage*  m_Image_AfterDcRaw;
  ptImage*  m_Image_AfterGeometry;
  ptImage*  m_Image_AfterRGB;
  ptImage*  m_Image_AfterLabCC;
  ptImage*  m_Image_AfterLabSN;
  ptImage*  m_Image_AfterLabEyeCandy;
  ptImage*  m_Image_AfterEyeCandy;

  // Cached image for detail preview
  ptImage*  m_Image_DetailPreview;

  // Sidecar image for texture overlay
  ptImage*  m_Image_TextureOverlay;
  ptImage*  m_Image_TextureOverlay2;

  // Reporting back
  void (*m_ReportProgress)(const QString Message);

  // Constructor
  ptProcessor(void (*ReportProgress)(const QString Message));
  // Destructor
  ~ptProcessor();

  // The associated DcRaw.
  ptDcRaw* m_DcRaw;

  // The real processing.
  void Run(short Phase,
           short SubPhase      = -1,
           short WithIdentify  = 1,
           short ProcessorMode = ptProcessorMode_Preview);

  // Rerun Geometry stage (and stop for crop or rotate tool)
  // Use the ptProcessorStopBefore_{Rotate|Crop} constants to stop early.
  void RunGeometry(const short StopBefore = ptProcessorStopBefore_NoStop);

  // Exif Related
  Exiv2::ExifData m_ExifData;
  unsigned char*  m_ExifBuffer;
  unsigned int    m_ExifBufferLength;
  void            ReadExifBuffer();

  // Reporting
  void ReportProgress(const QString Message);

  // AutoExposure Value in EV.
  double m_AutoExposureValue;
  // CalculateAutoExposure
  // There should be GuiSettings->m_WhiteFraction % pixels above 90%
  double CalculateAutoExposure(ptImage* Image);

  // Factor for size dependend filters
  float           m_ScaleFactor;


///////////////////////////////////////////////////////////////////////////
//
// PRIVATE members
//
///////////////////////////////////////////////////////////////////////////
private:
  QTime m_RunTimer;
};

#endif
