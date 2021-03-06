/*
    This file is part of Rekall.
    Copyright (C) 2013-2014

    Project Manager: Clarisse Bardiot
    Development & interactive design: Guillaume Jacquemin & Guillaume Marais (http://www.buzzinglight.com)

    This file was written by Guillaume Jacquemin.

    Rekall is a free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "global.h"

bool         Global::falseProject                 = false;
QImage       Global::temporaryScreenshot;
QFileInfo    Global::pathApplication;
QFileInfo    Global::pathDocuments;
QFileInfo    Global::pathCurrent;
QFileInfo    Global::pathCurrentDefault;
GlWidget*    Global::timelineGL                   = 0;
GlWidget*    Global::viewerGL                     = 0;
GlDrawable*  Global::timeline                     = 0;
qreal        Global::thumbsEach                   = 5;
qreal        Global::waveEach                     = 0.1;
qreal        Global::timeUnit                     = 0;
UiReal       Global::timeUnitDest                 = 13;
qreal        Global::timelineTagHeight            = 0;
UiReal       Global::timelineTagHeightDest        = 7;
qreal        Global::timeUnitTick                 = 5;
qreal        Global::timelineTagThumbHeight       = 40;
qreal        Global::timelineTagVSpacing          = 2;
qreal        Global::timelineTagVSpacingSeparator = 5;
QSizeF       Global::timelineHeaderSize           = QSizeF(200, 15);
qreal        Global::timelineGlobalDocsWidth  = 35;
qreal        Global::viewerTagHeight              = 30;
qreal        Global::time                         = 0;
qreal        Global::thumbnailSlider              = 0;
qreal        Global::thumbnailSliderStep          = 0.001;
qreal        Global::tagBlinkTime                 = 2000;
qreal        Global::breathing                    = 0;
qreal        Global::breathingDest                = 1;
qreal        Global::breathingPics                = 0;
qreal        Global::breathingPicsDest            = 1;
qreal        Global::breathingFast                = 0;
qreal        Global::breathingFastDest            = 1;
QTime        Global::timer;
UiBool       Global::showHelp                     = true;
UiBool       Global::showHistory                  = false;
bool         Global::timerPlay                    = false;
QList<void*> Global::selectedTagsInAction;
QList<void*> Global::selectedTags;
void*        Global::selectedTagHover             = 0;
void*        Global::timeMarkerAdded              = 0;
QPair<qreal,qreal>  Global::selectedTagHoverSnapped = qMakePair(-1., -1.);
QMap<QString,void*> Global::renders;
TagSelection Global::selectedTagMode              = TagSelectionNone;
qreal        Global::inertie                      = 5;
Udp*         Global::udp                          = 0;
QFont        Global::font;
QFont        Global::fontSmall;
GlVideo*     Global::video                        = 0;
QTreeWidget*   Global::chutier                    = 0;
ProjectBase*   Global::currentProject             = 0;
UserInfosBase* Global::userInfos                  = 0;

bool         Global::timelineSortChanged          = true;
bool         Global::metaChanged                  = true;
bool         Global::viewerSortChanged            = true;
bool         Global::eventsSortChanged            = true;
bool         Global::ticksChanged                 = true;
QMap<QString, QPair<QColor, qreal> > Global::colorForMeta;
Sorting*     Global::tagSortCriteria       = 0;
Sorting*     Global::tagColorCriteria      = 0;
Sorting*     Global::tagTextCriteria       = 0;
Sorting*     Global::tagClusterCriteria    = 0;
Sorting*     Global::tagFilterCriteria     = 0;
Sorting*     Global::tagHorizontalCriteria = 0;
Sorting*     Global::groupes               = 0;
WatcherBase* Global::watcher            = 0;
RekallBase*  Global::mainWindow         = 0;
TaskListBase*  Global::taskList         = 0;
FeedListBase*  Global::feedList         = 0;


QColor       Global::colorTagCaptation            = QColor(255, 255, 255);
QColor       Global::colorCluster                 = QColor(255, 255, 255, 100);
QColor       Global::colorAlternateMore           = QColor(255, 255, 255,  30);
QColor       Global::colorAlternate               = QColor(255, 255, 255,  12);
QColor       Global::colorAlternateLight          = QColor(255, 255, 255,   8);
QColor       Global::colorText                    = QColor(245, 248, 250);
QColor       Global::colorTagDisabled             = QColor(126, 126, 126);
QColor       Global::colorTimeline                = QColor( 45, 202, 225);      //QColor(50, 221, 255);
QColor       Global::colorAlternateStrong         = QColor(  0,   0,   0, 128);
QColor       Global::colorTextBlack               = QColor( 45,  50,  53);      //QColor( 43,  46,  47)
QColor       Global::colorTicks                   = QColor( 43,  46,  47);
QColor       Global::colorBackground              = QColor( 71,  77,  79);
QColor Global::getColorScale(qreal val) {
    QList<QColor> colors;
    if(val >= 100) {
        colors << QColor(74,201,159) << QColor(255,84,79) << QColor(229,149,205) << QColor(255,147,102) << QColor(166,204,91) << QColor(181,134,118) << QColor(255,234,136);
        val = (val/100)-1;
        return QColor(colors.at(val).red(),
                      colors.at(val).green(),
                      colors.at(val).blue(),
                      colors.at(val).alpha());
    }
    else {
        colors << QColor(229,149,205) << QColor(123,144,206) << QColor(74,201,159) << QColor(166,204,91) << QColor(255,234,136) << QColor(255,147,102) << QColor(181,134,118) << QColor(255,84,79);
        qreal indexFloat = val * (colors.count()-1);
        quint16 indexLow = qFloor(indexFloat), indexSup = qMin(indexLow+1, colors.count()-1); indexFloat = indexFloat - indexLow;
        return QColor(colors.at(indexLow).red()   * (1.-indexFloat) + colors.at(indexSup).red()   * (indexFloat),
                      colors.at(indexLow).green() * (1.-indexFloat) + colors.at(indexSup).green() * (indexFloat),
                      colors.at(indexLow).blue()  * (1.-indexFloat) + colors.at(indexSup).blue()  * (indexFloat),
                      colors.at(indexLow).alpha() * (1.-indexFloat) + colors.at(indexSup).alpha() * (indexFloat));
    }
}


void Global::seek(qreal _time) {
    time = _time;
    if(Global::video)
        Global::video->seek(time);
}
void Global::play(bool state) {
    if(Global::video)
        Global::video->play(state);
    Global::timer.restart();
    Global::timerPlay = state;
}




const QString Global::dateToString(const QDateTime &date, bool addExactTime) {
    quint16 daysTo   = date.daysTo(QDateTime::currentDateTime());
    quint16 secsTo   = date.secsTo(QDateTime::currentDateTime());
    quint16 minsTo   = secsTo / 60;
    quint16 hoursTo  = secsTo / 3600;
    quint16 weeksTo  = daysTo / 7;
    quint16 monthsTo = daysTo / 30;

    if(addExactTime) {
        if(monthsTo > 12)     return QString("on %1").arg(date.toString("dddd dd MM yyyy, hh:mm"));
        else if(monthsTo > 1) return QString("%1 ago, on %2").arg(plurial(monthsTo, "month" )).arg(date.toString("dddd dd MM hh:mm"));
        else if(weeksTo > 1)  return QString("%1 ago, on %2").arg(plurial(weeksTo,  "week"  )).arg(date.toString("dddd dd MM hh:mm"));
        else if(daysTo > 1)   return QString("%1 ago, on %2").arg(plurial(daysTo,   "day"   )).arg(date.toString("dddd, hh:mm"));
        else if(hoursTo > 1)  return QString("%1 ago, on %2").arg(plurial(hoursTo,  "hour"  )).arg(date.toString("hh:mm"));
        else if(minsTo  > 1)  return QString("%1 ago, on %2").arg(plurial(minsTo,   "minute")).arg(date.toString("hh:mm"));
        else if(secsTo  > 10) return QString("%1 ago, on %2").arg(plurial(secsTo,   "second")).arg(date.toString("hh:mm:ss"));
        else                  return QString("A few seconds ago, on %1").arg(date.toString("hh:mm:ss"));
    }
    else {
        if(monthsTo > 12)     return QString("on %1").arg(date.toString("dddd dd MM yyyy, hh:mm"));
        else if(monthsTo > 1) return QString("%1 ago").arg(plurial(monthsTo, "month" ));
        else if(weeksTo > 1)  return QString("%1 ago").arg(plurial(weeksTo,  "week"  ));
        else if(daysTo > 1)   return QString("%1 ago").arg(plurial(daysTo,   "day"   ));
        else if(hoursTo > 1)  return QString("%1 ago").arg(plurial(hoursTo,  "hour"  ));
        else if(minsTo  > 1)  return QString("%1 ago").arg(plurial(minsTo,   "minute"));
        else if(secsTo  > 10) return QString("%1 ago").arg(plurial(secsTo,   "second"));
        else                  return QString("A few seconds ago");
    }
}
const QString Global::plurial(qint16 value, const QString &text) {
    if(qAbs(value) > 1) return QString("%1 %2s") .arg(value).arg(text);
    else                return QString("%1 %2").arg(value).arg(text);
}

QString Global::getBetween(const QString &data, const QString &start, const QString &end, bool trim) {
    qint16 startIndex = data.indexOf(start)+start.length()+1;
    qint16 endIndex   = data.indexOf(end, startIndex);
    if(trim)    return data.mid(startIndex, endIndex-startIndex).trimmed();
    else        return data.mid(startIndex, endIndex-startIndex);
}
qreal Global::getDurationFromString(QString timeStr) {
    qreal duration = 0;
    if(timeStr.length() > 0) {
        timeStr.remove("seconds").remove("second").remove("secs").remove("sec").remove("s");
        QStringList parts = timeStr.split(" ").first().split(":");
        if(parts.count() == 3) {
            duration += parts.at(0).toDouble()*3600;
            duration += parts.at(1).toDouble()*60;
            QStringList decimals = Global::splits(parts.at(2), QStringList() << "." << ",");
            if(decimals.count())
                duration += decimals.at(0).toDouble();
            if(decimals.count() > 1)
                duration += decimals.at(1).toDouble() / 1000.;
        }
        else {
            QStringList decimals = Global::splits(parts.last(), QStringList() << "." << ",");
            if(decimals.count())
                duration += decimals.at(0).toDouble();
            if(decimals.count() > 1)
                duration += decimals.at(1).toDouble() / 1000.;
        }
    }
    return duration;
}
QStringList Global::splits(const QString &str, QStringList separators, QString::SplitBehavior behavior) {
    QStringList retour = QStringList() << str;
    foreach(const QString &separator, separators) {
        QStringList retourTmp;
        foreach(const QString &strToSplit, retour)
            retourTmp << strToSplit.split(separator, behavior);
        retour = retourTmp;
    }
    return retour;
}

QPair<QString,QString> Global::seperateMetadata(const QString &metaline, const QString &separator) {
    QPair<QString,QString> retour;
    qint16 index = metaline.indexOf(separator);
    if(index > 0) {
        retour.first = metaline.left(index).trimmed();
        retour.second = metaline.right(metaline.length() - index - 1).trimmed();
    }
    return retour;
}
QPair<QString, QPair<QString,QString> > Global::seperateMetadataAndGroup(const QString &metaline, const QString &separator) {
    QPair<QString, QPair<QString,QString> > retour;
    QPair<QString,QString> firstRetour = seperateMetadata(metaline, "]");
    retour.first = firstRetour.first.remove("[").trimmed();
    retour.second = seperateMetadata(firstRetour.second, separator);
    return retour;
}



QString Global::getFileHash(const QFileInfo &file) {
    QCryptographicHash fileHasher(QCryptographicHash::Sha1);
    QFile fileToHash(file.absoluteFilePath());
    fileToHash.open(QFile::ReadOnly);
    while(!fileToHash.atEnd())
        fileHasher.addData(fileToHash.read(8192));
    return QString(fileHasher.result().toHex()).toUpper();
}


FeedItemBase::FeedItemBase(FeedItemBaseType _action, const QString &_author, const QString &_object, const QDateTime &_date) {
    action = _action;
    author = _author;
    object = _object;
    date   = _date;
    if     (action == FeedItemBaseTypeCreation) {
        icon = QIcon(":/icons/events/download.gif");
        actionStr = "added";
    }
    else if(action == FeedItemBaseTypeUpdate) {
        icon = QIcon(":/icons/events/pencil.gif");
        actionStr = "updated";
    }
    else if(action == FeedItemBaseTypeDelete) {
        icon = QIcon(":/icons/events/x.gif");
        actionStr = "removed";
    }
    else if(action == FeedItemBaseTypeProcessingStart) {
        icon = QIcon(":/icons/events/bookmark.gif");
        actionStr = "started metadata extraction on";
    }
    else if(action == FeedItemBaseTypeProcessingEnd) {
        icon = QIcon(":/icons/events/bookmark.gif");
        actionStr = "finished metadata extraction on";
    }
}
void FeedItemBase::update() {
    setIcon(0, icon);
    setText(0, QString("<span style='font-family: Calibri, Arial; font-size: 11px; color: #F5F8FA'>%1 <span style='color: #A1A5A7'>%2</span> %3<span style='color: #A1A5A7'>, %4</span></span>").arg(author).arg(actionStr).arg(object).arg(Global::dateToString(date, false).toLower()));
    setToolTip(0, Global::dateToString(date));
}




void GlWidget::ensureVisible(const QPointF &point, qreal ratio) {
    if(point.x() && point.y()) {
        QRectF rect(QPointF(0, 0), size());
        if(!rect.translated(scrollDest).contains(point)) {
            QPointF pseudoCenter = rect.topLeft() + QPointF(rect.width() * ratio, rect.height() * ratio);
            QPointF scrollDestDest(scrollDest);
            if(point.x() >= 0)  scrollDestDest.setX(point.x() - pseudoCenter.x());
            if(point.y() >= 0)  scrollDestDest.setY(point.y() - pseudoCenter.y());
            scrollTo(scrollDestDest);
        }
    }
}
void GlWidget::scrollTo(const QPointF &point) {
    scrollDest.setX(qBound(0., point.x(), qMax(0., drawingBoundingRect.right()  + 100 - width())));
    scrollDest.setY(qBound(0., point.y(), qMax(0., drawingBoundingRect.bottom() + 100 - height())));
}


void GlText::setStyle(const QSize &_size, int _alignement, const QFont &_font) {
    size       = _size;
    font       = _font;
    init       = false;
    alignement = _alignement;
    image      = QImage(size, QImage::Format_ARGB32_Premultiplied);
    text       = "...";
}

void GlText::drawText(const QString &newtext, const QPoint &pos, qreal maxWidth) {
    setText(newtext, maxWidth);
    drawText(pos);
}
void GlText::setText(const QString &newText, qreal maxWidth) {
    if(newText != text) {
        if(maxWidth > 0) {
            QFontMetrics fm(font);
            text = fm.elidedText(newText, Qt::ElideRight, maxWidth, alignement);
        }
        else
            text = newText;
        init = false;
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::TextAntialiasing);
        painter.setPen(Qt::white);
        painter.setFont(font);
        painter.drawText(QRect(QPoint(0, 0), size), alignement, text);
        painter.end();
    }
}
void GlText::drawText(const QPoint &pos) {
    if(!init) {
        glEnable(GL_TEXTURE_2D);
        if(!texture) {
            glGenTextures(1, &texture);
            qDebug("%s", qPrintable(QString("[OPENGL] Création de la texture #%1 => |%2| (%3 x %4)").arg(texture).arg(text).arg(size.width()).arg(size.height())));
        }
        glBindTexture  (GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width(), size.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, QGLWidget::convertToGLFormat(image).bits());
        glDisable(GL_TEXTURE_2D);
        init = true;
    }
    if(init) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        GlRect::drawRect(QRect(pos, size));
        glDisable(GL_TEXTURE_2D);
    }
}



void GlRect::setTexture(const QString &filename, const QSize &desiredSize) {
    if(filename != currentFilename) {
        init = false;
        image.load(filename);
        if((desiredSize.width() > 0) && (desiredSize.height() > 0))
            image = image.scaled(desiredSize, Qt::KeepAspectRatio);
        size = image.size();
        currentFilename = filename;
    }
}
void GlRect::drawTexture(const QString &filename, const QRectF &rect, qreal croppingMode) {
    setTexture(filename);
    drawTexture(rect, croppingMode);
}
void GlRect::drawTexture(const QRectF &rect, qreal croppingMode) {
    if(!init) {
        glEnable(GL_TEXTURE_2D);
        if(!texture) {
            glGenTextures(1, &texture);
            qDebug("%s", qPrintable(QString("[OPENGL] Création de la texture #%1 => %2 (%3 x %4)").arg(texture).arg(currentFilename).arg(size.width()).arg(size.height())));
        }
        glBindTexture  (GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width(), size.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, QGLWidget::convertToGLFormat(image).bits());
        glDisable(GL_TEXTURE_2D);
        init = true;
    }
    if(init) {
        QRectF targetRect = rect;
        QRectF textureRect = QRectF(0, 0, 1, 1);
        QRectF partialTextureRect = QRectF(0,0,-1,-1);

        if(croppingMode <= -3)
            textureRect = QRectF(QPointF(0, 0), targetRect.size());
        else if(croppingMode == -2) {
            textureRect = QRectF(QPointF(0, 0), size);
            targetRect.setSize(textureRect.size() / qMax(textureRect.width(), textureRect.height()) * qMax(rect.width(), rect.height()));
            qreal scaleDepassement = qMax(targetRect.width() / rect.width(), targetRect.height() / rect.height());
            if(scaleDepassement > 1)
                targetRect.setSize(targetRect.size() / scaleDepassement);
            targetRect = QRectF(QPointF(rect.x() + rect.width() / 2 - targetRect.width() / 2, rect.y() + rect.height() / 2 - targetRect.height() / 2), QSizeF(targetRect.width(), targetRect.height()));
        }
        else if(croppingMode == -1) {
            textureRect = QRectF(QPointF(0, 0), size);
            if((partialTextureRect.width() > 0) && (partialTextureRect.height() > 0))
                textureRect = QRectF(QPointF(partialTextureRect.x()     * size.width(), partialTextureRect.y()      * size.height()),
                                     QSizeF (partialTextureRect.width() * size.width(), partialTextureRect.height() * size.height()));
        }
        else if(croppingMode >= 0) {
            textureRect.setSize(targetRect.size() / qMax(targetRect.width(), targetRect.height()) * qMax(size.width(), size.height()));
            qreal scaleDepassement = qMax(textureRect.width() / size.width(), textureRect.height() / size.height());
            if(scaleDepassement > 1)
                textureRect.setSize(textureRect.size() / scaleDepassement);
            qreal panXMax = textureRect.width() - size.width(), panYMax = textureRect.height() - size.height();
            qreal panX = ((2*croppingMode)-1) * panXMax/2, panY = ((2*croppingMode)-1) * panYMax/2;
            textureRect = QRectF(QPointF(panX + size.width() / 2 - textureRect.width() / 2, panY + size.height() / 2 - textureRect.height() / 2), QSizeF(textureRect.width(), textureRect.height()));
        }
        textureRect = QRectF(QPointF(textureRect.x()     / size.width(), textureRect.y()      / size.height()),
                             QSizeF (textureRect.width() / size.width(), textureRect.height() / size.height()));

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        if(croppingMode == -4) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            croppingMode = -3;
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        GlRect::drawRect(targetRect, 0, textureRect);
        glDisable(GL_TEXTURE_2D);
    }
}

void GlRect::drawRoundedRect(const QRectF &rect, bool border, qreal precision) {
    if(border)  GlRect::drawRect(rect, -precision);
    else        GlRect::drawRect(rect,  precision);
}
void GlRect::drawRect(const QRectF &rect, qreal roundPrecision, const QRectF &texCoord) {
    if(roundPrecision != 0) {
        if(roundPrecision > 0) {
            qreal borderRadius = rect.height() / 2;
            glBegin(GL_POLYGON);
            glVertex2f    (rect.topLeft()    .x() + borderRadius, rect.topLeft()    .y());
            glVertex2f    (rect.topRight()   .x() - borderRadius, rect.topRight()   .y());
            for(qreal angle = 0 ; angle < M_PI ; angle += qAbs(roundPrecision))
                glVertex2f(rect.topRight()   .x() - borderRadius + borderRadius * qCos(M_PI/2 - angle),   rect.center().y() - borderRadius * qSin(M_PI/2 - angle));
            glVertex2f    (rect.bottomRight().x() - borderRadius, rect.bottomRight().y());
            glVertex2f    (rect.bottomLeft() .x() + borderRadius, rect.bottomLeft() .y());
            for(qreal angle = 0 ; angle < M_PI ; angle += qAbs(roundPrecision))
                glVertex2f(rect.bottomLeft() .x() + borderRadius + borderRadius * qCos(3*M_PI/2 - angle), rect.center().y() - borderRadius * qSin(3*M_PI/2 - angle));
            glEnd();
        }
        if(roundPrecision < 0) {
            qreal borderRadius = rect.height() / 2;
            glBegin(GL_LINE_LOOP);
            glVertex2f    (rect.topLeft()    .x() + borderRadius, rect.topLeft()    .y());
            glVertex2f    (rect.topRight()   .x() - borderRadius, rect.topRight()   .y());
            for(qreal angle = 0 ; angle < M_PI ; angle += qAbs(roundPrecision))
                glVertex2f(rect.topRight()   .x() - borderRadius + borderRadius * qCos(M_PI/2 - angle),   rect.center().y() - borderRadius * qSin(M_PI/2 - angle));
            glVertex2f    (rect.bottomRight().x() - borderRadius, rect.bottomRight().y());
            glVertex2f    (rect.bottomLeft() .x() + borderRadius, rect.bottomLeft() .y());
            for(qreal angle = 0 ; angle < M_PI ; angle += qAbs(roundPrecision))
                glVertex2f(rect.bottomLeft() .x() + borderRadius + borderRadius * qCos(3*M_PI/2 - angle), rect.center().y() - borderRadius * qSin(3*M_PI/2 - angle));
            glEnd();
        }
    }
    else {
        glBegin(GL_QUADS);
        glTexCoord2f(texCoord.bottomLeft() .x(), texCoord.bottomLeft() .y()); glVertex2f(rect.topLeft()    .x(), rect.topLeft()    .y());
        glTexCoord2f(texCoord.bottomRight().x(), texCoord.bottomRight().y()); glVertex2f(rect.topRight()   .x(), rect.topRight()   .y());
        glTexCoord2f(texCoord.topRight()   .x(), texCoord.topRight()   .y()); glVertex2f(rect.bottomRight().x(), rect.bottomRight().y());
        glTexCoord2f(texCoord.topLeft()    .x(), texCoord.topLeft()    .y()); glVertex2f(rect.bottomLeft() .x(), rect.bottomLeft() .y());
        glEnd();
    }
}



HtmlDelegate::HtmlDelegate(bool _editable, QObject *parent) :
    QStyledItemDelegate(parent) {
    editable = _editable;
}
void HtmlDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItemV4 optionV4 = option;
    initStyleOption(&optionV4, index);

    QStyle *style = optionV4.widget? optionV4.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml(optionV4.text);

    /// Painting item without text
    optionV4.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;

    // Highlighting text if item is selected
    if (optionV4.state & QStyle::State_Selected)
        ctx.palette.setColor(QPalette::Text, optionV4.palette.color(QPalette::Active, QPalette::HighlightedText));

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4);
    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

QSize HtmlDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItemV4 optionV4 = option;
    initStyleOption(&optionV4, index);

    QTextDocument doc;
    doc.setHtml(optionV4.text);
    doc.setTextWidth(optionV4.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}
QWidget *HtmlDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
    if(!editable)
        return 0;
    QLineEdit *editor = new QLineEdit(parent);
    //editor->setStyleSheet();
    return editor;
}

QString HtmlDelegate::removeHtml(QString html) {
    return html.remove(QRegExp("<[^>]*>")).trimmed();
}
void HtmlDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QString data   = removeHtml(index.model()->data(index, Qt::EditRole).toString());
    QString prefix = index.model()->data(index, Qt::EditRole).toString();
    prefix = prefix.left(prefix.indexOf(data));
    (static_cast<QLineEdit*>(editor))->setWindowTitle(prefix);
    (static_cast<QLineEdit*>(editor))->setText(data);
}

void HtmlDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    model->setData(index, (static_cast<QLineEdit*>(editor))->windowTitle() + (static_cast<QLineEdit*>(editor))->text() + "</span>", Qt::EditRole);
}

void HtmlDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const {
    if(editor)
        editor->setGeometry(option.rect);
}

bool HtmlDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
