/*******************************************************************************
**
** Photivo
**
** Copyright (C) 2011 Bernd Schoeler <brjohn@brother-john.net>
** Copyright (C) 2011 Michael Munzert <mail@mm-log.com>
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

#include "../ptDefines.h"
#include "../ptSettings.h"
#include "../ptDcRaw.h"
#include "ptFileMgrDM.h"
#include "ptFileMgrConstants.h"

extern ptSettings* Settings;

//==============================================================================

ptFileMgrDM* ptFileMgrDM::m_Instance = NULL;

//==============================================================================

/*static*/
ptFileMgrDM* ptFileMgrDM::GetInstance() {
  if (m_Instance == NULL) {
    m_Instance = new ptFileMgrDM();
  }

  return m_Instance;
}

//==============================================================================

/*static*/
void ptFileMgrDM::DestroyInstance() {
  delete m_Instance;
  m_Instance = NULL;
}

//==============================================================================

ptFileMgrDM::ptFileMgrDM()
: QObject()
{
  m_TreeModel = new QFileSystemModel;
  m_TreeModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Drives);

  // Set root dir for the file system watcher
  #ifdef Q_OS_WIN
    m_TreeModel->setRootPath(m_TreeModel->myComputer().toString());
  #else
    m_TreeModel->setRootPath("/");
  #endif

  m_DirModel    = new ptSingleDirModel;
  m_TagModel    = new ptTagModel;

  // Init stuff for thumbnail generation
  m_ThumbList   = new QList<ptGraphicsThumbGroup*>;
  m_Thumbnailer = new ptThumbnailer();
  m_Thumbnailer->setThumbList(m_ThumbList);

  // Init thumbnail cache
  m_Cache = new ptThumbnailCache(1000);
  m_Thumbnailer->setCache(m_Cache);
}

//==============================================================================

ptFileMgrDM::~ptFileMgrDM() {
  StopThumbnailer();
  DelAndNull(m_TreeModel);
  DelAndNull(m_ThumbList);
  DelAndNull(m_Thumbnailer);
  DelAndNull(m_Cache);
  DelAndNull(m_DirModel);
  DelAndNull(m_TagModel);
}

//==============================================================================

void ptFileMgrDM::Clear() {
  StopThumbnailer();
  m_Cache->Clear();
}

//==============================================================================

int ptFileMgrDM::setThumbnailDir(const QString path) {
  m_CurrentDir = path;
#ifdef Q_OS_WIN
  if (path == MyComputerIdString) {
    return m_Thumbnailer->setDir("");
  }
#endif

  return m_Thumbnailer->setDir(path);
}

//==============================================================================

void ptFileMgrDM::StartThumbnailer() {
  m_Thumbnailer->start();
}

//==============================================================================

void ptFileMgrDM::StopThumbnailer() {
  m_Thumbnailer->blockSignals(true);
  m_Thumbnailer->Abort();
  m_Thumbnailer->blockSignals(false);
}

//==============================================================================

QImage* ptFileMgrDM::getThumbnail(const QString FileName,
                                  const int     MaxSize) {

  ptDcRaw dcRaw;
  bool isRaw = false;
  MagickWand* image = NewMagickWand();
  QSize Size = QSize(MaxSize, MaxSize);

  if (dcRaw.Identify(FileName) == 0 ) {
    // we have a raw image
    isRaw = true;
    QByteArray* ImgData = NULL;
    if (dcRaw.thumbnail(ImgData)) {
      // raw thumbnail read successfully
      Size.setWidth(dcRaw.m_Width);
      Size.setHeight(dcRaw.m_Height);
      ScaleThumbSize(&Size, MaxSize);
      MagickSetSize(image, 2*Size.width(), 2*Size.height());
      MagickReadImageBlob(image, (const uchar*)ImgData->data(), (const size_t)ImgData->length());
    }
    DelAndNull(ImgData);
  }

  if (!isRaw) {
    // no raw, try for bitmap
    MagickPingImage(image, FileName.toAscii().data());
    Size.setWidth(MagickGetImageWidth(image));
    Size.setHeight(MagickGetImageHeight(image));
    ScaleThumbSize(&Size, MaxSize);
    MagickSetSize(image, 2*Size.width(), 2*Size.height());
    MagickReadImage(image, FileName.toAscii().data());
  }

  ExceptionType MagickExcept;
  char* MagickErrMsg = MagickGetException(image, &MagickExcept);
  if (MagickExcept != UndefinedException) {
    // error occurred: no raw thumbnail, no supported image type, any other GM error
    printf("%s\n", QString::fromAscii(MagickErrMsg).toAscii().data());
    DestroyMagickWand(image);
    return new QImage(QString::fromUtf8(":/dark/icons/broken-image-48px.png"));

  } else {
    // no error: scale and rotate thumbnail
    QImage* Temp = GenerateThumbnail(image, Size);
    DestroyMagickWand(image);
    return Temp;
  }
}

//==============================================================================

QImage* ptFileMgrDM::GenerateThumbnail(MagickWand* image, const QSize tSize)
{
  // We want 8bit RGB data without alpha channel, scaled to thumbnail size
  MagickSetImageDepth(image, 8);
  MagickSetImageFormat(image, "RGB");
  MagickSetImageType(image, TrueColorType);
  // 0 is used for escaping the scaling
  if (tSize.width() > 0 && tSize.height() > 0)
    MagickScaleImage(image, tSize.width(), tSize.height());

  // read EXIF orientation and correct image
  int orientation = QString::fromAscii(MagickGetImageAttribute(image, "EXIF:Orientation")).toInt();
  PixelWand* pxWand = NewPixelWand();
  switch (orientation) {
    case 2: MagickFlopImage(image); break;
    case 3: MagickRotateImage(image, pxWand, 180); break;
    case 4: MagickFlipImage(image); break;
    case 5: MagickFlopImage(image); MagickRotateImage(image, pxWand, 270); break;
    case 6: MagickRotateImage(image, pxWand, 90); break;
    case 7: MagickFlipImage(image); MagickRotateImage(image, pxWand, 270); break;
    case 8: MagickRotateImage(image, pxWand, 270); break;
    default: break;
  }
  DestroyPixelWand(pxWand);

  // Get the raw image data from GM.
  uint w = MagickGetImageWidth(image);
  uint h = MagickGetImageHeight(image);

  QImage* thumbImage = new QImage(w, h, QImage::Format_RGB32);
  MagickGetImagePixels(image, 0, 0, w, h, "BGRA", CharPixel, (uchar*)thumbImage->scanLine(0));
  return thumbImage;
}

//==============================================================================

void ptFileMgrDM::ScaleThumbSize(QSize* tSize, const int max) {
  if (tSize->width() == tSize->height()) {    // square image
    tSize->setWidth(max);
    tSize->setHeight(max);
  } else if (tSize->width() > tSize->height()) {    // landscape image
    tSize->setHeight(tSize->height()/(double)tSize->width() * max + 0.5);
    tSize->setWidth(max);
  } else if (tSize->width() < tSize->height()) {    // portrait image
    tSize->setWidth(tSize->width()/(double)tSize->height() * max + 0.5);
    tSize->setHeight(max);
  }
}

//==============================================================================
