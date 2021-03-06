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

#include "tag.h"

Tag::Tag(DocumentBase *_document, qint16 _documentVersion) :
    QObject(_document) {
    document          = _document;
    version   = _documentVersion;
    timelineWasInside = false;
    player       = 0;
    progression       = progressionDest = decounter = 0;
    isInProgress      = false;
    breathing         = false;
    viewerFirstPos = timelineFirstPos = true;
    viewerFirstPosVisible = timelineFirstPosVisible = false;
    blinkTime         = 0;
    timelineFilesAction = 0;
    timeStart = timeEnd = timeMediaOffset = 0;
    tagScale     = 0;
    tagDestScale = 1;
    displayText  = "";
    linkMove = linkMoveDest = 0.66;

    viewerTimeText          .setStyle(QSize( 70, Global::viewerTagHeight), Qt::AlignCenter,    Global::font);
    viewerDocumentText      .setStyle(QSize(300, Global::viewerTagHeight), Qt::AlignVCenter,   Global::font);
    timelineTimeStartText   .setStyle(QSize( 70, Global::timelineTagHeightDest), Qt::AlignRight | Qt::AlignVCenter, Global::fontSmall);
    timelineTimeEndText     .setStyle(QSize( 70, Global::timelineTagHeightDest), Qt::AlignLeft  | Qt::AlignVCenter, Global::fontSmall);
    timelineTimeDurationText.setStyle(QSize(100, Global::timelineTagHeightDest), Qt::AlignCenter, Global::fontSmall);
    timelineDocumentText    .setStyle(QSize(300, Global::timelineTagHeightDest), Qt::AlignCenter, Global::fontSmall);
}

void Tag::init() {
    if(document->getFunction() == DocumentFunctionRender) {
        Global::renders.insert(document->getName(version), this);
        player = new PlayerVideo();
        player->load(this, ((document->getType() == DocumentTypeVideo) || (document->getType() == DocumentTypeImage)), document->file.absoluteFilePath());
    }
}

void Tag::init(TagType _type, qreal _timeStart, qreal _duration) {
    if(document->getFunction() == DocumentFunctionRender) {
        setType(_type, _timeStart);
        Global::renders.insert(document->getName(version), this);
        player = new PlayerVideo();
        player->load(this, ((document->getType() == DocumentTypeVideo) || (document->getType() == DocumentTypeImage)), document->file.absoluteFilePath());
        if(Global::falseProject)
            _duration = Global::alea(50, 180);
    }
    else {
        if(Global::falseProject) {
            qreal val = Global::alea(0, 100);
            if(document->getType(version) == DocumentTypeMarker)
                val = 0;
            if(val < 50)      setType(TagTypeContextualTime,      _timeStart);
            else if(val < 80) setType(TagTypeContextualMilestone, _timeStart);
            else              setType(TagTypeGlobal, 0);
        }
        else
            setType(_type, _timeStart);

        if(Global::falseProject) {
            qreal tirage = Global::aleaF(0, 100);
            if(tirage > 50)       _duration = Global::alea( 2,  5);
            else if(tirage > 10)  _duration = Global::alea( 5, 12);
            else                  _duration = Global::alea(12, 25);
        }
    }
    if(getType() == TagTypeContextualTime)
        setTimeEnd(timeStart + _duration);
}
void Tag::setType(TagType _type, qreal time) {
    type = _type;
    if(time < 0)    time = timeStart;
    if(getType() == TagTypeContextualTime) {
        if((timeStart == 0) && (timeEnd == 0))  timeStart = time;
        else                                    timeStart = time - 5;
        timeEnd = timeStart + 5;
    }
    else
        timeStart = timeEnd = time;
    //Global::timelineSortChanged = Global::viewerSortChanged = Global::eventsSortChanged = true;
}

void Tag::setTimeStart(qreal _timeStart) {
    qreal mediaDuration = document->getMediaDuration(version);

    if(mediaDuration <= 0)
        mediaDuration = timeEnd;
    timeStart = qBound(timeEnd - mediaDuration + getTimeMediaOffset(), _timeStart, timeEnd);
    //Global::timelineSortChanged = Global::viewerSortChanged = Global::eventsSortChanged = true;
}
void Tag::setTimeEnd(qreal _timeEnd) {
    if(getType() == TagTypeContextualTime) {
        qreal mediaDuration = document->getMediaDuration(version);
        if(mediaDuration <= 0)
            mediaDuration = 9999999;
        timeEnd = qBound(timeStart, _timeEnd, timeStart + mediaDuration - getTimeMediaOffset());
        //Global::timelineSortChanged = Global::viewerSortChanged = Global::eventsSortChanged = true;
    }
    else
        timeEnd = timeStart;
}
void Tag::setTimeMediaOffset(qreal _timeMediaOffset) {
    timeMediaOffset = qBound(0., _timeMediaOffset, document->getMediaDuration(version) - getDuration());
    setTimeEnd(timeEnd);
}
void Tag::moveTimeStart(qreal _timeStart) {
    qreal duration = getDuration();
    timeStart = qMax(0., _timeStart);
    timeEnd   = timeStart + duration;
    //Global::timelineSortChanged = Global::viewerSortChanged = Global::eventsSortChanged = true;
}
void Tag::moveTimeEnd(qreal _timeEnd) {
    qreal duration = getDuration();
    timeEnd   = _timeEnd;
    timeStart = qMax(0., timeEnd - duration);
    //Global::timelineSortChanged = Global::viewerSortChanged = Global::eventsSortChanged = true;
}
void Tag::addTimeMediaOffset(qreal offset) {
    setTimeMediaOffset(getTimeMediaOffset() + offset);
}






bool Tag::fireEvents() {
    //Progression
    progressionDest = progress(Global::time);
    Global::inert(&progression, progressionDest);
    decounter = qMin(0., Global::time - getTimeStart());

    //Enter / Leave
    qreal progressionAbs = progressAbs(Global::time);
    qint16 oscValue = -1;
    if(((0. <= progressionAbs) && (progressionAbs <= 1.)) && (!timelineWasInside)) {
        timelineWasInside = true;
        blinkTime = Global::tagBlinkTime;
        if(document->getFunction() == DocumentFunctionRender) {
            if(!player) {
                player = new PlayerVideo();
                player->load(this, ((document->getType() == DocumentTypeVideo) || (document->getType() == DocumentTypeImage)), document->file.absoluteFilePath());
            }
            Global::video->load(this);
        }
        if(getType() != TagTypeGlobal)
            oscValue = 1;
    }
    else if(((0. > progressionAbs) || (progressionAbs > 1.)) && (timelineWasInside)) {
        timelineWasInside = false;
        if(document->getFunction() == DocumentFunctionRender)
            Global::video->unload(this);
        if(getType() != TagTypeGlobal)
            oscValue = 0;
    }
    if(oscValue >= 0)
        Global::udp->send("127.0.0.1", 57120, "/rekall", QList<QVariant>() << document->getTypeStr(version) << document->getAuthor(version) << document->getName(version) << getTimeStart() << getTimeEnd() << oscValue << document->baseColor.redF() << document->baseColor.greenF() << document->baseColor.blueF() << document->baseColor.alphaF());

    if((oscValue == 1) && (document->getType() == DocumentTypeMarker)) {
        Global::mainWindow->changeAnnotation(this);
        return true;
    }
    return ((0. <= progressionAbs) && (progressionAbs <= 1.)) && (document->getType() == DocumentTypeMarker);
}

const QRectF Tag::paintTimeline(bool before) {
    if(before) {
        Global::inert(&tagScale, tagDestScale);
        Global::inert(&timelinePos, timelineDestPos);

        if((Global::tagHorizontalCriteria->isTimeline()) && (getType() == TagTypeGlobal))
            timelineBoundingRect = QRectF(QPointF(Global::timelineGL->scroll.x()-Global::timelineGlobalDocsWidth, 0), QSizeF(qMax(Global::timelineTagHeight, getDuration(true) * Global::timeUnit), Global::timelineTagHeight));
            //timelineBoundingRect = QRectF(QPointF(Global::timelineGL->scroll.x()-Global::timelineGlobalDocsWidth, 0), QSizeF(getDuration(true) * Global::timeUnit, Global::timelineTagHeight));
        else {
            qreal pos   = Global::tagHorizontalCriteria->getCriteriaFormatedReal(getCriteriaHorizontal(this), getTimeStart());
            qreal width = Global::tagHorizontalCriteria->getCriteriaFormatedRealDuration(getDuration(true));
            timelineBoundingRect = QRectF(QPointF(pos * Global::timeUnit, 0), QSizeF(qMax(Global::timelineTagHeight, width * Global::timeUnit), Global::timelineTagHeight));
            //timelineBoundingRect = QRectF(QPointF(pos * Global::timeUnit, 0), QSizeF(width * Global::timeUnit, Global::timelineTagHeight));
            if(getType() == TagTypeContextualMilestone)
                timelineBoundingRect.translate(-timelineBoundingRect.width() / 2, 0);
        }

        bool isLargeTag = (document->getFunction() == DocumentFunctionRender) && (Global::tagHorizontalCriteria->isTimeline());
        if(isLargeTag)
            timelineBoundingRect.setHeight(Global::timelineTagThumbHeight);
        //if(timelineBoundingRect.width() < timelineBoundingRect.height())
        //  timelineBoundingRect.adjust(-(timelineBoundingRect.height() - timelineBoundingRect.width())/2, 0, (timelineBoundingRect.height() - timelineBoundingRect.width())/2, 0);

        //QColor colorDestTmp = (Global::selectedTags.contains(this) == true)?(Global::colorTimeline):(document->baseColor);
        QColor colorDestTmp = document->baseColor;
        if(document->status == DocumentStatusWaiting)
            colorDestTmp.setAlphaF(0.1);
        if((document->status == DocumentStatusProcessing) || (Global::selectedTags.contains(this) == true))
            colorDestTmp.setAlphaF(Global::breathingFast);
        if(!Global::tagHorizontalCriteria->isTimeline()) {
            if((Global::timerPlay) && !((0.001 < progression) && (progression < 0.999))) colorDestTmp.setAlphaF(0.2);
            //else                                                                         colorDestTmp.setAlphaF(1.0);
        }

        if(!((colorDestTmp.red() == 0) && (colorDestTmp.green() == 0) && (colorDestTmp.blue() == 0)))
            colorDest = colorDestTmp;
        realTimeColor.setRedF  (realTimeColor.redF()   + (colorDest.redF()   - realTimeColor.redF()  ) / Global::inertie);
        realTimeColor.setGreenF(realTimeColor.greenF() + (colorDest.greenF() - realTimeColor.greenF()) / Global::inertie);
        realTimeColor.setBlueF (realTimeColor.blueF()  + (colorDest.blueF()  - realTimeColor.blueF() ) / Global::inertie);
        realTimeColor.setAlphaF(realTimeColor.alphaF() + (colorDest.alphaF() - realTimeColor.alphaF()) / Global::inertie);
        if(breathing)
            realTimeColor = realTimeColor.lighter(100 + (1-Global::breathing)*15);

        glPushMatrix();
        glTranslatef(qRound(timelinePos.x()), qRound(timelinePos.y()), 0);

        glTranslatef(qRound(timelineBoundingRect.center().x()), qRound(timelineBoundingRect.center().y()), 0);
        glScalef(tagScale, tagScale, 1);
        glTranslatef(-qRound(timelineBoundingRect.center().x()), -qRound(timelineBoundingRect.center().y()), 0);

        //Drawing
        if(Global::timelineGL->visibleRect.intersects(timelineBoundingRect.translated(timelinePos + QPointF(0, Global::timelineHeaderSize.height())))) {
            //Thumbnail strip
            if(isLargeTag) {
                qreal mediaDuration = document->getMediaDuration(version);

                //Thumb adapt
                if((document->getType() == DocumentTypeVideo) && (document->thumbnails.count()))
                    timelineBoundingRect.setHeight((Global::thumbsEach * Global::timeUnit) * document->thumbnails.first().size.height() / document->thumbnails.first().size.width());

                //Strip
                if((document->getType() == DocumentTypeVideo) && (document->thumbnails.count())) {
                    //Media offset
                    qreal mediaOffset    = getTimeMediaOffset();
                    qreal timeThumbStart = (mediaOffset / Global::thumbsEach)                   * Global::thumbsEach;
                    qreal timeThumbEnd   = ((mediaOffset + getDuration()) / Global::thumbsEach) * Global::thumbsEach;

                    Global::timelineGL->qglColor(Qt::white);
                    for(qreal timeThumbX = timeThumbStart ; timeThumbX < timeThumbEnd ; timeThumbX += Global::thumbsEach) {
                        QRectF thumbRect = QRectF(QPointF((timeThumbX-timeThumbStart) * Global::timeUnit, 0), QSizeF(Global::thumbsEach * Global::timeUnit, timelineBoundingRect.height())).translated(timelineBoundingRect.topLeft());
                        thumbRect.setRight(qMin(thumbRect.right(), timelineBoundingRect.right()));
                        document->thumbnails[qMin(qFloor(timeThumbX / Global::thumbsEach)+1, document->thumbnails.count()-1)].drawTexture(thumbRect, 0);
                    }
                }
                else if((document->waveform.count()) && (mediaDuration > 0)) {
                    qreal mediaOffset    = getTimeMediaOffset() / mediaDuration;
                    qreal sampleMax      = getDuration()        / mediaDuration;

                    glBegin(GL_LINES);
                    Global::timelineGL->qglColor(realTimeColor);
                    for(qreal timeX = 0 ; timeX < timelineBoundingRect.width() ; timeX++) {
                        quint16 waveformIndex = qMin((int)((mediaOffset + timeX/timelineBoundingRect.width()*sampleMax) * document->waveform.count()), document->waveform.count()-1);
                        glVertex2f(timelineBoundingRect.left() + timeX, timelineBoundingRect.center().y() - document->waveform.at(waveformIndex).first  * document->waveform.normalisation * timelineBoundingRect.height()/2);
                        glVertex2f(timelineBoundingRect.left() + timeX, timelineBoundingRect.center().y() - document->waveform.at(waveformIndex).second * document->waveform.normalisation * timelineBoundingRect.height()/2 + 1);
                    }
                    glEnd();
                }
                else if(document->thumbnails.count()) {
                    Global::timelineGL->qglColor(Qt::white);
                    document->thumbnails.first().drawTexture(timelineBoundingRect, Global::thumbnailSlider);
                }
                else {
                    Global::timelineGL->qglColor(realTimeColor);
                    GlRect::drawRect(timelineBoundingRect);
                }
            }
            else if(document->getType() == DocumentTypeMarker) {
                //Bar
                Global::timelineGL->qglColor(realTimeColor);
                if((getType() == TagTypeContextualMilestone) || (getType() == TagTypeGlobal)) {
                    timelineBoundingRect.setWidth(3);
                    GlRect::drawRect(timelineBoundingRect);
                }
                else {
                    GlRect::drawRect(QRectF(timelineBoundingRect.topLeft(),  QSizeF( 1, timelineBoundingRect.height())));
                    GlRect::drawRect(QRectF(timelineBoundingRect.topRight(), QSizeF(-1, timelineBoundingRect.height())));
                    GlRect::drawRect(QRectF(timelineBoundingRect.topLeft() + QPointF(0, timelineBoundingRect.height() / 2 - 1), QSizeF(timelineBoundingRect.width(), 2)));
                }
            }
            else {
                //Bar
                Global::timelineGL->qglColor(realTimeColor);
                if(isTagLastVersion(this))
                    GlRect::drawRoundedRect(timelineBoundingRect.adjusted(1, 1, -1, -1), false, M_PI/4);
                GlRect::drawRoundedRect(timelineBoundingRect.adjusted(1, 1, -1, -1), true, M_PI/4);
            }

            //Text
            QPoint textPos;
            if(getType() != TagTypeGlobal) {
                Global::timelineGL->qglColor(realTimeColor);

                if(Global::selectedTags.contains(this)) {
                    textPos = QPoint(timelineBoundingRect.left() - 2 - timelineTimeStartText.size.width(), 1 + timelineBoundingRect.center().y() - timelineTimeStartText.size.height()/2);
                    timelineTimeStartText.drawText(Sorting::timeToString(getTimeStart()), textPos);

                    if(getType() == TagTypeContextualTime) {
                        textPos = QPoint(timelineBoundingRect.right() + 2, 1 + timelineBoundingRect.center().y() - timelineTimeEndText.size.height()/2);
                        if(Global::tagHorizontalCriteria->isTimeline())
                            timelineTimeEndText.drawText(Sorting::timeToString(getTimeEnd()), textPos);
                        else
                            timelineTimeEndText.drawText(Sorting::timeToString(getTimeEnd()) + " (" + Sorting::timeToString(getDuration()) + ")", textPos);

                        if(isTagLastVersion(this))
                            Global::timelineGL->qglColor(Qt::white);
                        textPos = QPoint(timelineBoundingRect.center().x() - timelineTimeDurationText.size.width()/2, 1 + timelineBoundingRect.center().y() - timelineTimeDurationText.size.height()/2);
                        if(document->getFunction(version) == DocumentFunctionRender) {
                            if(getTimeMediaOffset() > 0)    timelineTimeDurationText.drawText(Sorting::timeToString(getDuration()) + " / " + Sorting::timeToString(document->getMediaDuration()) + tr(" (-") + Sorting::timeToString(getTimeMediaOffset()) + ")", textPos);
                            else                            timelineTimeDurationText.drawText(Sorting::timeToString(getDuration()) + " / " + Sorting::timeToString(document->getMediaDuration()), textPos);
                        }
                        else if(Global::tagHorizontalCriteria->isTimeline())
                            timelineTimeDurationText.drawText(Sorting::timeToString(getDuration()), textPos);
                    }
                }
                if((!displayText.isEmpty()) && (document->getFunction(version) == DocumentFunctionContextual)) {
                    textPos = QPoint(timelineBoundingRect.center().x() - timelineDocumentText.size.width()/2, timelineBoundingRect.top() - timelineDocumentText.size.height() - 1);
                    timelineDocumentText.drawText(displayText, textPos, qMax(20., timelineBoundingRect.width()));
                }
            }

            //Selection anchors
            if((Global::selectedTags.contains(this)) && (getType() == TagTypeContextualTime) && (document->getFunction() == DocumentFunctionContextual)) {
                Global::timelineGL->qglColor(Global::colorBackground);
                glBegin(GL_LINES);
                glVertex2f(timelineBoundingRect.topLeft()    .x() + 10, timelineBoundingRect.topLeft()    .y());
                glVertex2f(timelineBoundingRect.bottomLeft() .x() + 10, timelineBoundingRect.bottomLeft() .y());
                glVertex2f(timelineBoundingRect.topRight()   .x() - 10, timelineBoundingRect.topRight()   .y());
                glVertex2f(timelineBoundingRect.bottomRight().x() - 10, timelineBoundingRect.bottomRight().y());
                glEnd();
            }
        }


        //History tags
        if(historyTags.count()) {
            QColor colorAlpha = realTimeColor;

            //Anchors
            QPointF historyChordBegCtr = timelineBoundingRect.center();
            QPointF historyChordBegTop(timelineBoundingRect.center().x(), timelineBoundingRect.top()    + 1);
            QPointF historyChordBegBtm(timelineBoundingRect.center().x(), timelineBoundingRect.bottom() - 1);
            GLfloat historyChordPts[4][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
            glLineWidth(2);
            foreach(Tag *historyTag, historyTags) {
                if((historyTag == this) || (!historyTag) || (!historyTag->isAcceptableWithSortFilters(true)))
                    continue;

                //Color
                if(((historyTag->getType() == TagTypeGlobal) && (getType() != TagTypeGlobal)) || ((getType() == TagTypeGlobal) && (historyTag->getType() != TagTypeGlobal)))    colorAlpha.setAlphaF(0.1);
                else                                                                                                                                                            colorAlpha.setAlphaF(0.4);
                Global::timelineGL->qglColor(colorAlpha);

                //Anchors
                QPointF historyChordEndCtr = historyTag->timelineBoundingRect.center() - timelinePos + historyTag->timelinePos;
                QPointF historyChordEndTop = QPointF(historyChordEndCtr.x(), historyTag->timelineBoundingRect.top()    + 1 - timelinePos.y() + historyTag->timelinePos.y());
                QPointF historyChordEndBtm = QPointF(historyChordEndCtr.x(), historyTag->timelineBoundingRect.bottom() - 1 - timelinePos.y() + historyTag->timelinePos.y());

                QPointF historyChordBeg = historyChordBegCtr, historyChordEnd = historyChordEndCtr;
                if(     historyChordEndCtr.y() < historyChordBegCtr.y())   { historyChordBeg = historyChordBegTop;   historyChordEnd = historyChordEndBtm;   }
                else if(historyChordEndCtr.y() > historyChordBegCtr.y())   { historyChordBeg = historyChordBegBtm;   historyChordEnd = historyChordEndTop;   }
                historyChordPts[0][0] = historyChordBeg.x(); historyChordPts[0][1] = historyChordBeg.y();
                historyChordPts[1][0] = historyChordBeg.x(); historyChordPts[1][1] = historyChordBeg.y() + (historyChordEnd.y() - historyChordBeg.y()) * linkMoveDest;
                historyChordPts[2][0] = historyChordEnd.x(); historyChordPts[2][1] = historyChordEnd.y() + (historyChordBeg.y() - historyChordEnd.y()) * linkMoveDest;
                historyChordPts[3][0] = historyChordEnd.x(); historyChordPts[3][1] = historyChordEnd.y();
                glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &historyChordPts[0][0]);
                glEnable(GL_MAP1_VERTEX_3);
                glBegin(GL_LINE_STRIP);
                for(qreal t = 0 ; t <= 1.05; t += 0.05) {
                    glColor4f(realTimeColor.redF()   * (1-t) + historyTag->realTimeColor.redF() * t,
                              realTimeColor.greenF() * (1-t) + historyTag->realTimeColor.greenF() * t,
                              realTimeColor.blueF()  * (1-t) + historyTag->realTimeColor.blueF() * t,
                              colorAlpha.alphaF());
                    glEvalCoord1f(t);
                }
                glEnd();
                glDisable(GL_MAP1_VERTEX_3);
            }
            glLineWidth(1);
        }

        //Linked tags
        Global::inert(&linkMove, linkMoveDest, 5);
        if((Global::timelineGL->showLinkedTags > 0.01) && (linkedTags.count())) {
            //Anchors
            QPointF linkedChordBegCtr = timelineBoundingRect.center();
            QPointF linkedChordBegTop(timelineBoundingRect.center().x(), timelineBoundingRect.top()    + 1);
            QPointF linkedChordBegBtm(timelineBoundingRect.center().x(), timelineBoundingRect.bottom() - 1);
            GLfloat linkedChordPts[4][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
            glLineWidth(2);
            foreach(Tag *linkedTag, linkedTags) {
                if((linkedTag == this) || (!linkedTag) || (!linkedTag->isAcceptableWithSortFilters(true)))
                    continue;

                //Anchors
                QPointF linkedChordEndCtr = linkedTag->timelineBoundingRect.center() - timelinePos + linkedTag->timelinePos;
                QPointF linkedChordEndTop = QPointF(linkedChordEndCtr.x(), linkedTag->timelineBoundingRect.top()    + 1 - timelinePos.y() + linkedTag->timelinePos.y());
                QPointF linkedChordEndBtm = QPointF(linkedChordEndCtr.x(), linkedTag->timelineBoundingRect.bottom() - 1 - timelinePos.y() + linkedTag->timelinePos.y());

                QPointF linkedChordBeg = linkedChordBegCtr, historyChordEnd = linkedChordEndCtr;
                if(     linkedChordEndCtr.y() < linkedChordBegCtr.y())   { linkedChordBeg = linkedChordBegTop;   historyChordEnd = linkedChordEndBtm;   }
                else if(linkedChordEndCtr.y() > linkedChordBegCtr.y())   { linkedChordBeg = linkedChordBegBtm;   historyChordEnd = linkedChordEndTop;   }

                linkedChordPts[0][0] = linkedChordBegCtr.x(); linkedChordPts[0][1] = linkedChordBegCtr.y();
                linkedChordPts[1][0] = linkedChordBegCtr.x(); linkedChordPts[1][1] = linkedChordBegCtr.y() + (linkedChordEndCtr.y() - linkedChordBegCtr.y()) * linkMove;
                linkedChordPts[2][0] = linkedChordEndCtr.x(); linkedChordPts[2][1] = linkedChordEndCtr.y() + (linkedChordBegCtr.y() - linkedChordEndCtr.y()) * linkMove;
                linkedChordPts[3][0] = linkedChordEndCtr.x(); linkedChordPts[3][1] = linkedChordEndCtr.y();
                glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &linkedChordPts[0][0]);
                glEnable(GL_MAP1_VERTEX_3);
                glBegin(GL_LINE_STRIP);
                for(qreal t = 0 ; t <= (1.05 * Global::timelineGL->showLinkedTags); t += 0.05) {
                    glColor4f(realTimeColor.redF()   * (1-t) + linkedTag->realTimeColor.redF() * t,
                              realTimeColor.greenF() * (1-t) + linkedTag->realTimeColor.greenF() * t,
                              realTimeColor.blueF()  * (1-t) + linkedTag->realTimeColor.blueF() * t,
                              0.4);
                    glEvalCoord1f(t);
                }
                glEnd();
                glDisable(GL_MAP1_VERTEX_3);
            }
            glLineWidth(1);
        }

        //Linking
        if((Global::selectedTagsInAction.contains(this)) && (Global::selectedTagMode == TagSelectionLink)) {
            QColor colorAlpha = realTimeColor;
            QColor destColor  = realTimeColor;
            Tag *hoverTag = (Tag*)Global::selectedTagHover;
            if(hoverTag) {
                if((linkedTags.contains(hoverTag)) || (historyTags.contains(hoverTag)) || (hoverTag->linkedTags.contains(this)) || (hoverTag->historyTags.contains(this)))
                    colorAlpha = destColor = Qt::red;
                else
                    destColor = ((Tag*)hoverTag)->realTimeColor;
            }
            colorAlpha.setAlphaF(0.4);
            destColor .setAlphaF(0.4);
            Global::timelineGL->qglColor(colorAlpha);

            //Anchors
            QPointF linkedChordBegCtr = timelineBoundingRect.center();
            GLfloat linkedChordPts[4][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
            glLineWidth(2);

            //Anchors
            QPointF linkedChordEndCtr = selectedTagMousePos - timelinePos;
            linkedChordEndCtr.setY(linkedChordEndCtr.y() - Global::timelineHeaderSize.height());
            if(hoverTag)
                linkedChordEndCtr = hoverTag->getTimelineBoundingRect().center() + hoverTag->timelinePos - timelinePos;
            linkedChordPts[0][0] = linkedChordBegCtr.x(); linkedChordPts[0][1] = linkedChordBegCtr.y();
            linkedChordPts[1][0] = linkedChordBegCtr.x(); linkedChordPts[1][1] = linkedChordBegCtr.y() + (linkedChordEndCtr.y() - linkedChordBegCtr.y()) * linkMoveDest;
            linkedChordPts[2][0] = linkedChordEndCtr.x(); linkedChordPts[2][1] = linkedChordEndCtr.y() + (linkedChordBegCtr.y() - linkedChordEndCtr.y()) * linkMoveDest;
            linkedChordPts[3][0] = linkedChordEndCtr.x(); linkedChordPts[3][1] = linkedChordEndCtr.y();
            glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &linkedChordPts[0][0]);
            glEnable(GL_MAP1_VERTEX_3);
            glBegin(GL_LINE_STRIP);
            for(qreal t = 0 ; t <= (1.05 * 1); t += 0.05) {
                glColor4f(colorAlpha.redF()   * (1-t) + destColor.redF() * t,
                          colorAlpha.greenF() * (1-t) + destColor.greenF() * t,
                          colorAlpha.blueF()  * (1-t) + destColor.blueF() * t,
                          colorAlpha.alphaF());
                glEvalCoord1f(t);
            }
            glEnd();
            glDisable(GL_MAP1_VERTEX_3);
            glLineWidth(1);
        }

        //Hash tags
        if((Global::timelineGL->showHashedTags > 0.01) && (hashTags.count())) {
            QColor colorAlpha = realTimeColor;

            //Anchors
            QPointF hashChordBegCtr = timelineBoundingRect.center();
            QPointF hashChordBegTop(timelineBoundingRect.center().x(), timelineBoundingRect.top()    + 1);
            QPointF hashChordBegBtm(timelineBoundingRect.center().x(), timelineBoundingRect.bottom() - 1);
            GLfloat hashChordPts[4][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
            glLineWidth(1);
            glLineStipple(5, 0xAAAA);
            glEnable(GL_LINE_STIPPLE);
            foreach(Tag *hashTag, hashTags) {
                if((hashTag == this) || (!hashTag) || (!hashTag->isAcceptableWithSortFilters(true)))
                    continue;

                //Color
                if(((hashTag->getType() == TagTypeGlobal) && (getType() != TagTypeGlobal)) || ((getType() == TagTypeGlobal) && (hashTag->getType() != TagTypeGlobal)))
                    colorAlpha.setAlphaF(0.2);
                else
                    colorAlpha.setAlphaF(0.4);
                Global::timelineGL->qglColor(colorAlpha);

                //Anchors
                QPointF hashChordEndCtr = hashTag->timelineBoundingRect.center() - timelinePos + hashTag->timelinePos;
                QPointF hashChordBeg = hashChordBegCtr, hashChordEnd = hashChordEndCtr;
                hashChordPts[0][0] = hashChordBeg.x(); hashChordPts[0][1] = hashChordBeg.y();
                hashChordPts[1][0] = hashChordBeg.x(); hashChordPts[1][1] = hashChordBeg.y() + (hashChordEnd.y() - hashChordBeg.y()) * linkMoveDest;
                hashChordPts[2][0] = hashChordEnd.x(); hashChordPts[2][1] = hashChordEnd.y() + (hashChordBeg.y() - hashChordEnd.y()) * linkMoveDest;
                hashChordPts[3][0] = hashChordEnd.x(); hashChordPts[3][1] = hashChordEnd.y();
                glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &hashChordPts[0][0]);
                glEnable(GL_MAP1_VERTEX_3);
                glBegin(GL_LINE_STRIP);
                for(qreal t = 0 ; t <= (1.05 * Global::timelineGL->showHashedTags); t += 0.05) {
                    glColor4f(realTimeColor.redF()   * (1-t) + hashTag->realTimeColor.redF() * t,
                              realTimeColor.greenF() * (1-t) + hashTag->realTimeColor.greenF() * t,
                              realTimeColor.blueF()  * (1-t) + hashTag->realTimeColor.blueF() * t,
                              colorAlpha.alphaF());
                    glEvalCoord1f(t);
                }
                glEnd();
                glDisable(GL_MAP1_VERTEX_3);
            }
            glLineWidth(1);
            glDisable(GL_LINE_STIPPLE);
        }



        //Snapping
        if((Global::selectedTagsInAction.count()) && (Global::selectedTagHover == this) && ((Global::selectedTagHoverSnapped.first >= 0) || (Global::selectedTagHoverSnapped.second >= 0))) {
            foreach(void *selectedTagInAction, Global::selectedTagsInAction) {
                Tag *snappedTag = (Tag*)selectedTagInAction;
                qint16 posStart = Global::timelineHeaderSize.width() + Global::timelineGlobalDocsWidth + Global::timeUnit * Global::selectedTagHoverSnapped.first  - timelinePos.x();
                qint16 posEnd   = Global::timelineHeaderSize.width() + Global::timelineGlobalDocsWidth + Global::timeUnit * Global::selectedTagHoverSnapped.second - timelinePos.x();

                QList< QPair<QPointF, QPointF> > pointsToDraw;
                bool progressive = true;

                if((Global::selectedTagMode == TagSelectionMove) && ((Global::selectedTagHoverSnapped.first >= 0) || (Global::selectedTagHoverSnapped.second >= 0))) {
                    if((Global::selectedTagHoverSnapped.first >= 0) && (Global::selectedTagHoverSnapped.second >= 0)) {
                        progressive = false;
                        pointsToDraw.append(qMakePair(QPointF(timelineBoundingRect.left(),  timelineBoundingRect.center().y()),
                                                      QPointF(posStart, snappedTag->timelineBoundingRect.translated(snappedTag->timelinePos).translated(-timelinePos).center().y())));
                        pointsToDraw.append(qMakePair(QPointF(timelineBoundingRect.left(),  timelineBoundingRect.center().y()),
                                                      QPointF(snappedTag->timelineBoundingRect.translated(snappedTag->timelinePos).translated(-timelinePos).left(), snappedTag->timelineBoundingRect.translated(snappedTag->timelinePos).translated(-timelinePos).center().y())));
                        pointsToDraw.append(qMakePair(QPointF(timelineBoundingRect.right(), timelineBoundingRect.center().y()),
                                                      QPointF(posEnd, snappedTag->timelineBoundingRect.translated(snappedTag->timelinePos).translated(-timelinePos).center().y())));
                        pointsToDraw.append(qMakePair(QPointF(timelineBoundingRect.right(), timelineBoundingRect.center().y()),
                                                      QPointF(snappedTag->timelineBoundingRect.translated(snappedTag->timelinePos).translated(-timelinePos).right(), snappedTag->timelineBoundingRect.translated(snappedTag->timelinePos).translated(-timelinePos).center().y())));
                    }
                    else if(Global::selectedTagHoverSnapped.first >= 0)
                        pointsToDraw.append(qMakePair(QPointF(timelineBoundingRect.left(), timelineBoundingRect.center().y()),
                                                      QPointF(snappedTag->timelineBoundingRect.translated(snappedTag->timelinePos).translated(-timelinePos).right(), snappedTag->timelineBoundingRect.translated(snappedTag->timelinePos).translated(-timelinePos).center().y())));
                    else if(Global::selectedTagHoverSnapped.second >= 0)
                        pointsToDraw.append(qMakePair(QPointF(timelineBoundingRect.right(), timelineBoundingRect.center().y()),
                                                      QPointF(snappedTag->timelineBoundingRect.translated(snappedTag->timelinePos).translated(-timelinePos).left(), snappedTag->timelineBoundingRect.translated(snappedTag->timelinePos).translated(-timelinePos).center().y())));
                }
                else if(Global::selectedTagHoverSnapped.first >= 0)
                    pointsToDraw.append(qMakePair(QPointF(posStart, timelineBoundingRect.center().y()),
                                                  QPointF(posStart, snappedTag->timelineBoundingRect.translated(snappedTag->timelinePos).translated(-timelinePos).center().y())));
                else if(Global::selectedTagHoverSnapped.second >= 0)
                    pointsToDraw.append(qMakePair(QPointF(posEnd, timelineBoundingRect.center().y()),
                                                  QPointF(posEnd, snappedTag->timelineBoundingRect.translated(snappedTag->timelinePos).translated(-timelinePos).center().y())));

                Global::timelineGL->qglColor(Global::colorCluster);
                glLineStipple(5, 0xAAAA);
                glEnable(GL_LINE_STIPPLE);
                GLfloat hashChordPts[4][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
                for(quint16 i = 0 ; i < pointsToDraw.count() ; i+=2) {
                    QPointF hashChordBeg, hashChordEnd;
                    hashChordBeg = (pointsToDraw.at(i).first  * (Global::timelineGL->tagSnapSlow)) + (pointsToDraw.at(qMin(pointsToDraw.count()-1, i+1)).first  * (1-Global::timelineGL->tagSnapSlow));
                    hashChordEnd = (pointsToDraw.at(i).second * (Global::timelineGL->tagSnapSlow)) + (pointsToDraw.at(qMin(pointsToDraw.count()-1, i+1)).second * (1-Global::timelineGL->tagSnapSlow));
                    hashChordPts[0][0] = hashChordBeg.x(); hashChordPts[0][1] = hashChordBeg.y();
                    hashChordPts[1][0] = hashChordBeg.x(); hashChordPts[1][1] = hashChordBeg.y() + (hashChordEnd.y() - hashChordBeg.y()) * linkMoveDest;
                    hashChordPts[2][0] = hashChordEnd.x(); hashChordPts[2][1] = hashChordEnd.y() + (hashChordBeg.y() - hashChordEnd.y()) * linkMoveDest;
                    hashChordPts[3][0] = hashChordEnd.x(); hashChordPts[3][1] = hashChordEnd.y();
                    glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &hashChordPts[0][0]);
                    glEnable(GL_MAP1_VERTEX_3);
                    glBegin(GL_LINE_STRIP);
                    qreal tMax = 1.05;  if(progressive) tMax *= Global::timelineGL->tagSnap;
                    for(qreal t = 0 ; t <= tMax; t += 0.05)
                        glEvalCoord1f(t);
                    glEnd();
                    glDisable(GL_MAP1_VERTEX_3);
                }
                glDisable(GL_LINE_STIPPLE);
            }
        }
        glPopMatrix();
    }
    linkMoveDest = 0.66;

    if(timelineFirstPosVisible) {
        timelineFirstPosVisible = false;
        Global::timelineGL->ensureVisible(timelineBoundingRect.translated(timelinePos).topLeft());
    }
    return timelineBoundingRect.translated(timelinePos);
}

const QRectF Tag::paintViewer(quint16 tagIndex) {
    Global::inert(&viewerPos, viewerDestPos);
    viewerBoundingRect = QRectF(QPointF(0, 0), QSizeF(Global::viewerGL->width(), Global::viewerTagHeight));
    QRectF thumbnailRect;
    bool hasThumbnail = false;
    if(document->thumbnails.count()) {
        thumbnailRect = QRectF(QPointF(75, 11), QSizeF(105, 70));
        hasThumbnail = true;
        viewerBoundingRect.setHeight(thumbnailRect.height() + 2*thumbnailRect.top());
    }


    if(Global::viewerGL->visibleRect.intersects(viewerBoundingRect.translated(viewerPos))) {
        glPushMatrix();
        glTranslatef(qRound(viewerPos.x()), qRound(viewerPos.y()), 0);

        glTranslatef(qRound(viewerBoundingRect.center().x()), qRound(viewerBoundingRect.center().y()), 0);
        glScalef(tagScale, tagScale, 1);
        glTranslatef(-qRound(viewerBoundingRect.center().x()), -qRound(viewerBoundingRect.center().y()), 0);

        //Selection
        if(Global::selectedTags.contains(this)) {
            QColor color = document->baseColor;
            if((document->status == DocumentStatusProcessing) || (Global::selectedTags.contains(this)))
                color.setAlphaF(Global::breathingFast);
            Global::viewerGL->qglColor(color);
            GlRect::drawRect(viewerBoundingRect);
        }

        //Alternative background
        if(tagIndex % 2) {
            Global::viewerGL->qglColor(Global::colorAlternate);
            GlRect::drawRect(viewerBoundingRect);
        }

        //Progression
        isInProgress = false;
        bool isBlinking = false;
        QRectF progressionRect(viewerBoundingRect.topLeft(), QSizeF(viewerBoundingRect.width(), 5));
        if((getType() == TagTypeContextualMilestone) && (blinkTime)) {
            isInProgress = true;
            blinkTime = qMax(0., blinkTime-20);
            if(qFloor(blinkTime / 250) % 2) {
                if(document->getType() == DocumentTypeMarker) {
                    isBlinking = true;
                    Global::viewerGL->qglColor(realTimeColor);
                    GlRect::drawRect(viewerBoundingRect);
                }
                else {
                    Global::viewerGL->qglColor(Global::colorTimeline);
                    GlRect::drawRect(progressionRect);
                }
            }
        }
        else if((0.001 < progression) && (progression < 0.999)) {
            isInProgress = true;
            if(document->getType() == DocumentTypeMarker) {
                Global::viewerGL->qglColor(realTimeColor);
                GlRect::drawRect(QRectF(progressionRect.topLeft(), QSizeF(progressionRect.width() * progression, viewerBoundingRect.height())));
            }
            else {
                Global::viewerGL->qglColor(Global::colorTimeline);
                GlRect::drawRect(QRectF(progressionRect.topLeft(), QSizeF(progressionRect.width() * progression, progressionRect.height())));
            }
        }

        //Bar
        QColor barColor = realTimeColor;
        if((decounter == 0) && (!isInProgress) && (Global::timerPlay))  barColor.setAlphaF(0.2);
        else                                                            barColor.setAlphaF(1.0);


        Global::viewerGL->qglColor(barColor);
        if(hasThumbnail)
            GlRect::drawRect(thumbnailRect.adjusted(-5, -5, 5, 5));
        else if(document->getType() == DocumentTypeMarker)
            GlRect::drawRect(QRectF(viewerBoundingRect.topLeft(), QSizeF(5, viewerBoundingRect.height())));
        else {
            if(isTagLastVersion(this))
                GlRect::drawRoundedRect(viewerBoundingRect.adjusted(7, 7, -75, -7).translated(QPointF(60, 0)), false, M_PI/4);
            GlRect::drawRoundedRect(viewerBoundingRect.adjusted(7, 7, -75, -7).translated(QPointF(60, 0)), true, M_PI/4);
        }


        //Décompte
        if((Global::timerPlay) && (-5 <= decounter) && (decounter < 0)) {
            Global::viewerGL->qglColor(Global::colorText);
            qreal angleMax = 2*M_PI * -decounter/5, cote = (Global::viewerTagHeight / 2) * 0.6;
            QRect rect(QPoint(0, 0), QSize(70, viewerBoundingRect.height()));
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(rect.center().x(), rect.center().y());
            for(qreal angle = 0 ; angle < angleMax ; angle += 0.1)
                glVertex2f(rect.center().x() + cote * qCos(-M_PI/2 + angle), rect.center().y() + cote * qSin(-M_PI/2 + angle));
            glEnd();
        }
        //Temps
        else if((decounter < 0) || (!Global::timerPlay)) {
            Global::viewerGL->qglColor(Global::colorText);
            viewerTimeText.drawText(Sorting::timeToString(getTimeStart()));
        }


        //Thumb
        QPoint textePos(75, 0);
        if(hasThumbnail) {
            Global::viewerGL->qglColor(QColor(255, 255, 255, barColor.alpha()));
            document->thumbnails.first().drawTexture(thumbnailRect, Global::breathingPics);
            textePos = thumbnailRect.topRight().toPoint() + QPoint(10, -10);
        }

        //Texte
        if((isBlinking) || (isInProgress))                                                                                                               Global::viewerGL->qglColor(Qt::white);
        else if(((Global::selectedTags.contains(this)) || ((isTagLastVersion(this) && (!hasThumbnail)))) && (document->getType() != DocumentTypeMarker)) Global::viewerGL->qglColor(Qt::black);
        else                                                                                                                                             Global::viewerGL->qglColor(barColor);
        QString texte = document->getName(version);
        if(getType() == TagTypeContextualTime)
            texte += QString(" (%1)").arg(Sorting::timeToString(getDuration()));
        viewerDocumentText.drawText(texte, textePos);

        glPopMatrix();
    }
    if(viewerFirstPosVisible) {
        viewerFirstPosVisible = false;
        Global::viewerGL->ensureVisible(viewerBoundingRect.translated(viewerDestPos).topLeft());
    }
    return viewerBoundingRect.translated(viewerPos);
}

bool Tag::mouseTimeline(const QPointF &pos, QMouseEvent *e, bool dbl, bool, bool, bool press, bool) {
    if(timelineContains(pos)) {
        if(press) {
            if(!Global::selectedTags.contains(this)) {
                if(!((e) && (((e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier) || ((e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier))))
                    Global::selectedTags.clear();
                if(Global::selectedTags.contains(this)) Global::selectedTags.removeOne(this);
                else                                    Global::selectedTags.append(this);
                Global::selectedTagHover = this;
                Global::mainWindow->displayMetadataAndSelect(this);
            }
            else if(!Global::selectedTagsInAction.contains(this)) {
                Global::selectedTagsInAction.clear();
                Global::selectedTagsInAction = Global::selectedTags;
                if(Global::tagHorizontalCriteria->isTimeline()) {
                    foreach(void* _tag, Global::selectedTagsInAction) {
                        Tag *tag = (Tag*)_tag;
                        tag->selectedTagStartDrag = tag->timelineProgressRaw(pos) * tag->getDuration();
                        tag->selectedTagMousePos = pos;
                    }
                    if((e->button() & Qt::LeftButton) == Qt::LeftButton) {
                        qreal pixelPos      = qMax(getDuration(), Global::timelineTagHeight / Global::timeUnit) * Global::timeUnit * timelineProgressRaw(pos);
                        qreal pixelDuration = qMax(getDuration(), Global::timelineTagHeight / Global::timeUnit) * Global::timeUnit;
                        if(     (pixelPos < 10)                 && (getType() == TagTypeContextualTime))   Global::selectedTagMode = TagSelectionStart;
                        else if((pixelPos > (pixelDuration-10)) && (getType() == TagTypeContextualTime))   Global::selectedTagMode = TagSelectionEnd;
                        else if((e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)             Global::selectedTagMode = TagSelectionMediaOffset;
                        else if((e->modifiers() & Qt::AltModifier)   == Qt::AltModifier)                   Global::selectedTagMode = TagSelectionDuplicate;
                        else if((e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)                 Global::selectedTagMode = TagSelectionLink;
                        else                                                                               Global::selectedTagMode = TagSelectionMove;
                    }
                }
                else {
                    if((e->button() & Qt::LeftButton) == Qt::LeftButton) {
                        if((e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)                      Global::selectedTagMode = TagSelectionLink;
                    }
                }

            }


            //Menu
            if(((e->buttons() & Qt::RightButton) == Qt::RightButton) && (Global::tagHorizontalCriteria->isTimeline())) {
                QMenu *menu = new QMenu(Global::mainWindow);
                QAction *actionToContextualTime = 0, *actionToGlobal = 0, *actionToContextualMilestone = 0;
                if(getType() == TagTypeGlobal) {
                    actionToContextualTime      = menu->addAction(QIcon(":/icons/res_icon_toContextualDuration.png"),  tr("Convert to duration document…"));
                    actionToContextualMilestone = menu->addAction(QIcon(":/icons/res_icon_toContextualMilestone.png"), tr("Convert to milestone document…"));
                }
                else {
                    actionToGlobal = menu->addAction(QIcon(":/icons/res_icon_toGlobal.png"), tr("Convert to global…"));
                    if(     getType() == TagTypeContextualTime)       actionToContextualMilestone = menu->addAction(QIcon(":/icons/res_icon_toContextualMilestone.png"), tr("Convert to milestone…"));
                    else if(getType() == TagTypeContextualMilestone)  actionToContextualTime      = menu->addAction(QIcon(":/icons/res_icon_toContextualDuration.png"),  tr("Add duration…"));
                }
                QAction *ret = menu->exec(QCursor::pos());
                if(     ret == actionToGlobal)                setType(TagTypeGlobal);
                else if(ret == actionToContextualMilestone)   setType(TagTypeContextualMilestone);
                else if(ret == actionToContextualTime)        setType(TagTypeContextualTime);
                if(ret) {
                    Global::timelineGL->ensureVisible(getTimelineBoundingRect().translated(timelineDestPos).topLeft());
                    Global::viewerGL  ->ensureVisible(getViewerBoundingRect()  .translated(viewerDestPos)  .topLeft());
                }
            }

            //Opens
            if(dbl) {
                if(document->chutierItem)
                    document->chutierItem->fileShowInOS();
                else if(document->getType(version) == DocumentTypeWeb)
                    UiFileItem::fileShowInOS(document->getMetadata("Rekall", "URL", version).toString());
                tagScale     = 3;
                tagDestScale = 1;
            }
            return true;
        }
        Global::selectedTagHover = this;
        if(!Global::selectedTags.count())
            Global::mainWindow->displayMetadata();

        return true;
    }
    return (Global::selectedTagsInAction.contains(this));
}
bool Tag::mouseViewer(const QPointF &pos, QMouseEvent *e, bool dbl, bool, bool, bool press, bool) {
    if(viewerContains(pos)) {
        Global::selectedTagHover = this;

        if(press) {
            Global::timeline->seek(getTimeStart(), true, false);
            if(!((e) && (((e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier) || ((e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier))))
                Global::selectedTags.clear();
            if(Global::selectedTags.contains(this)) Global::selectedTags.removeOne(this);
            else                                    Global::selectedTags.append(this);
            Global::selectedTagsInAction.clear();
            if(document->chutierItem)
                Global::chutier->setCurrentItem(document->chutierItem);
        }
        if((dbl) && (document->chutierItem)) {
            /*
            document->chutierItem->fileShowInOS();
            tagScale     = 3;
            tagDestScale = 1;
            */
            Global::mainWindow->showPreviewTab();
        }
        return true;
    }
    return false;
}


bool Tag::snapTime(qreal *time) const {
    if(qAbs(*time-getTimeStart()) < 1) {
        *time = getTimeStart();
        Global::selectedTagHoverSnapped.first  = *time;
        Global::selectedTagHoverSnapped.second = -1;
        return true;
    }
    else if(qAbs(*time-getTimeEnd()) < 1) {
        *time = getTimeEnd();
        Global::selectedTagHoverSnapped.second = *time;
        Global::selectedTagHoverSnapped.first  = -1;
        return true;
    }
    else if(Global::selectedTagMode == TagSelectionMove) {
        Global::selectedTagHoverSnapped.first  = getTimeStart();
        Global::selectedTagHoverSnapped.second = getTimeEnd();
        return true;
    }
    else if(Global::selectedTagMode == TagSelectionLink) {
        Global::selectedTagHoverSnapped.first  = getTimeStart();
        Global::selectedTagHoverSnapped.second = getTimeEnd();
        return true;
    }
    return false;
}


bool Tag::tagHistoryFilters() const {
    return (   Global::showHistory) ||
            ((!Global::showHistory) && (isTagLastVersion(this)));
}
bool Tag::isAcceptableWithSortFilters(bool strongCheck) const {
    return (tagHistoryFilters()) && (document->isAcceptableWithSortFilters(strongCheck, version));
}
bool Tag::isAcceptableWithColorFilters(bool strongCheck) const {
    return (tagHistoryFilters()) && (document->isAcceptableWithColorFilters(strongCheck, version));
}
bool Tag::isAcceptableWithTextFilters(bool strongCheck) const {
    return (tagHistoryFilters()) && (document->isAcceptableWithTextFilters(strongCheck, version));
}
bool Tag::isAcceptableWithClusterFilters(bool strongCheck) const {
    return (tagHistoryFilters()) && (document->isAcceptableWithClusterFilters(strongCheck, version));
}
bool Tag::isAcceptableWithFilterFilters(bool strongCheck) const {
    return (tagHistoryFilters()) && (document->isAcceptableWithFilterFilters(strongCheck, version));
}
bool Tag::isAcceptableWithGroupeFilters(bool strongCheck) const {
    return (tagHistoryFilters()) && (document->isAcceptableWithGroupeFilters(strongCheck, version));
}
bool Tag::isAcceptableWithHorizontalFilters(bool strongCheck) const {
    return (tagHistoryFilters()) && (document->isAcceptableWithHorizontalFilters(strongCheck, version));
}

const QString Tag::getAcceptableWithClusterFilters() const {
    return document->getAcceptableWithClusterFilters(version);
}


bool Tag::sortColor(const Tag *first, const Tag *second) {
    if((!first) || (!second))
        return false;

    QString firstStr = Tag::getCriteriaColor(first), secondStr = Tag::getCriteriaColor(second);
    if(firstStr == secondStr) {
        firstStr = first->document->getName(first->getDocumentVersion()); secondStr = second->document->getName(second->getDocumentVersion());
        if(firstStr == secondStr)
            return first->getDocumentVersion() < second->getDocumentVersion();
        return firstStr < secondStr;
    }
    else
        return firstStr < secondStr;
}
bool Tag::sortViewer(const Tag *first, const Tag *second) {
    if((!first) || (!second))
        return false;
    if((first->progressionDest == second->progressionDest) || (((0. < first->progressionDest) && (first->progressionDest < 1.)) && ((0. < second->progressionDest) && (second->progressionDest < 1.))))
        return first->getTimeStart() < second->getTimeStart();
    else
        return first->progressionDest > second->progressionDest;
}
bool Tag::sortEvents(const Tag *first, const Tag *second) {
    if((!first) || (!second))
        return false;
    return first->getTimeStart() < second->getTimeStart();
}
bool Tag::sortAlpha(const Tag *first, const Tag *second) {
    if((!first) || (!second))
        return false;
    return first->document->getName(first->getDocumentVersion()) < second->document->getName(second->getDocumentVersion());
}


const QString Tag::getCriteriaSort(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaSort(tag->version);
}
const QString Tag::getCriteriaColor(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaColor(tag->version);
}
const QString Tag::getCriteriaText(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaText(tag->version);
}
const QString Tag::getCriteriaCluster(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaCluster(tag->version);
}
/*
const MetadataElement Tag::getCriteriaPhase(const Tag *tag) {
    return tag->document->getCriteriaPhase(tag->version);
}
*/
const QString Tag::getCriteriaFilter(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaFilter(tag->version);
}
const QString Tag::getCriteriaFilterFormated(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaFilterFormated(tag->version);
}
const QString Tag::getCriteriaHorizontal(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaHorizontal(tag->version);
}
const QString Tag::getCriteriaHorizontalFormated(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaHorizontalFormated(tag->version);
}

const QString Tag::getCriteriaGroupe(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaGroupe(tag->version);
}
const QString Tag::getCriteriaGroupeFormated(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaGroupeFormated(tag->version);
}


const QString Tag::getCriteriaSortFormated(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaSortFormated(tag->version);
}
const QString Tag::getCriteriaTextFormated(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaTextFormated(tag->version);
}
const QString Tag::getCriteriaColorFormated(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaColorFormated(tag->version);
}
const QString Tag::getCriteriaClusterFormated(const Tag *tag) {
    if(!tag)    return "";
    return tag->document->getCriteriaClusterFormated(tag->version);
}

bool Tag::isTagLastVersion(const Tag *tag) {
    if(tag) return (tag->version < 0) || (tag->version == tag->document->getMetadataCountM());
    else    return false;
}

QDomElement Tag::serialize(QDomDocument &xmlDoc) const {
    QDomElement xmlData = xmlDoc.createElement("tag");
    xmlData.setAttribute("timeStart",       getTimeStart());
    xmlData.setAttribute("timeEnd",         getTimeEnd());
    xmlData.setAttribute("type",            getType());
    xmlData.setAttribute("documentVersion", getDocumentVersion());
    /*
    if(linkedTags.count()) {
        QDomElement linkedTagsXml = xmlDoc.createElement("linkedTags");
        foreach(Tag *linkedTag, linkedTags) {
            if(linkedTag) {
                QDomElement linkedTagXml = xmlDoc.createElement("linkedTag");
                linkedTagXml.setAttribute("documentVersion", linkedTag->version);
                linkedTagsXml.appendChild(linkedTagXml);
            }
        }
        xmlData.appendChild(linkedTagsXml);
    }
    */
    return xmlData;
}
void Tag::deserialize(const QDomElement &xmlElement) {
    QString a = xmlElement.attribute("attribut");
}
