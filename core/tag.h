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

#ifndef TAG_H
#define TAG_H

#include <QObject>
#include <QMenu>
#include <QFontMetrics>
#include "misc/global.h"
#include "metadata.h"


class DocumentBase : public Metadata {
public:
    explicit DocumentBase(QObject *parent = 0) : Metadata(parent) { }
    virtual void removeTag(void *tag) = 0;
};


class Tag : public QObject, public Nameable {
    Q_OBJECT

public:
    explicit Tag(DocumentBase *_document, qint16 _documentVersion = -1);

private:
    qreal   timeStart, timeEnd, timeMediaOffset;
    TagType type;
    qint16  version;
public:
    qreal   linkMove, linkMoveDest;
    QString        displayText;
    QList<Tag*>    linkedTags;
    qreal          selectedTagStartDrag;
    QPointF        selectedTagMousePos;
public:
    inline qreal   getTimeStart()       const { return timeStart; }
    inline qreal   getTimeEnd()         const { return timeEnd;   }
    inline qreal   getTimeMediaOffset() const { return timeMediaOffset; }
    inline TagType getType()            const { return type;      }
    void setTimeStart      (qreal _timeStart);
    void setTimeEnd        (qreal _timeEnd);
    void setTimeMediaOffset(qreal _timeMediaOffset);
    void setType           (TagType _type, qreal time = -1);
    void moveTimeStart(qreal _timeStart);
    void moveTimeEnd  (qreal _timeEnd);
    void addTimeMediaOffset(qreal offset);


private:
    qreal          progression;
    DocumentBase  *document;
public:
    QAction       *timelineFilesAction;
    PlayerVideo   *player;

public:
    QList<Tag*> historyTags, hashTags;

public:
    void init(TagType _type, qreal _timeStart, qreal _duration = 0);
    void init();
    const QString getTitle() const { return document->getName(version); }
    inline qint16 getDocumentVersion() const    {  return document->getMetadataIndexVersion(version); }
    inline qint16 getDocumentVersionRaw() const {  return version; }
    inline void   setDocumentVersion(qint16 versionShift = 0) {
        if(version < 0)
            version = getDocumentVersion() + versionShift;
    }

private:
    bool   timelineWasInside, isInProgress;
    qreal  progressionDest, decounter;
    qreal  blinkTime;
    QColor colorDest;
    QColor realTimeColor;
    bool   breathing;
private:
public:
    inline const QColor getRealTimeColor() const { return realTimeColor; }
    inline void  shouldBreathe(bool _breathing)  { breathing = _breathing; }
    inline bool  contains(qreal time)      const { return (getTimeStart() <= time) && (time <= (getTimeStart() + qMax(1., getDuration()))); }
    inline qreal progress(qreal time)      const { return qBound(0., progressAbs(time), 1.); }
    inline qreal progressAbs(qreal time)   const { return (time - getTimeStart()) / qMax(1., getDuration()); }
    inline qreal getDuration(bool drawable = false) const {
        if((drawable) && (getType() == TagTypeContextualMilestone))
            return 0;
        return timeEnd - timeStart;
    }
    inline DocumentBase* getDocument()     const { return document; }
    inline bool timelineContains (const QPointF &pos) const {    return (timelineBoundingRect.translated(timelinePos).translated(0, Global::timelineHeaderSize.height()).contains(pos)); }
    inline bool viewerContains   (const QPointF &pos) const {    return (viewerBoundingRect.translated(viewerPos).contains(pos));    }
    inline qreal timelineProgressRaw(const QPointF &pos) const {
        QRectF rect = timelineBoundingRect.translated(timelinePos);
        return (pos.x() - rect.x()) / rect.width();
    }
    inline qreal timelineProgress(const QPointF &pos) const {
        return qBound(0., timelineProgressRaw(pos), 1.);
    }
    inline qreal viewerProgress  (const QPointF &pos) const {
        QRectF rect = viewerBoundingRect.translated(viewerPos);
        return qBound(0., (pos.y() - rect.y()) / rect.height(), 1.);
    }
    bool snapTime(qreal *time) const;
    bool fireEvents();

private:
    GlText  viewerTimeText, viewerDocumentText, timelineTimeStartText, timelineTimeEndText, timelineTimeDurationText, timelineDocumentText;
    GlRect  viewerTimePastille;
    bool    timelineFirstPos, timelineFirstPosVisible;
    bool    viewerFirstPos, viewerFirstPosVisible;
    QRectF  timelineBoundingRect, viewerBoundingRect;
    qreal   tagDestScale;
public:
    qreal   tagScale;
    QPointF timelinePos, timelineDestPos;
    QPointF viewerPos,   viewerDestPos;
public:
    inline void setTimelinePos(const QPointF _timelineDestPos) {
        timelineDestPos = _timelineDestPos;
        if(timelineFirstPos) {
            timelinePos = timelineDestPos;
            timelineFirstPos = false;
            timelineFirstPosVisible = true;
        }
    }
    inline void setViewerPos(const QPointF _viewerDestPos) {
        viewerDestPos = _viewerDestPos;
        if(viewerFirstPos) {
            viewerPos = viewerDestPos;
            viewerFirstPos = false;
            viewerFirstPosVisible = true;
        }
    }
    inline const QRectF getTimelineBoundingRect() const { return timelineBoundingRect; }
    inline const QRectF getViewerBoundingRect()   const { return viewerBoundingRect;   }
    const QRectF paintTimeline(bool before = false);
    const QRectF paintViewer(quint16 tagIndex);
    bool  mouseTimeline(const QPointF &, QMouseEvent *, bool, bool, bool, bool, bool);
    bool  mouseViewer  (const QPointF &, QMouseEvent *, bool, bool, bool, bool, bool);

public:
    bool tagHistoryFilters() const;
    bool isAcceptableWithSortFilters      (bool strongCheck) const;
    bool isAcceptableWithColorFilters     (bool strongCheck) const;
    bool isAcceptableWithTextFilters      (bool strongCheck) const;
    bool isAcceptableWithClusterFilters   (bool strongCheck) const;
    bool isAcceptableWithFilterFilters    (bool strongCheck) const;
    bool isAcceptableWithGroupeFilters    (bool strongCheck) const;
    bool isAcceptableWithHorizontalFilters(bool strongCheck) const;
    const QString getAcceptableWithClusterFilters() const;
    static bool sortColor (const Tag *first, const Tag *second);
    static bool sortViewer(const Tag *first, const Tag *second);
    static bool sortEvents(const Tag *first, const Tag *second);
    static bool sortAlpha (const Tag *first, const Tag *second);
    static const QString getCriteriaSort              (const Tag *tag);
    static const QString getCriteriaColor             (const Tag *tag);
    static const QString getCriteriaText              (const Tag *tag);
    static const QString getCriteriaCluster           (const Tag *tag);
    static const QString getCriteriaFilter            (const Tag *tag);
    static const QString getCriteriaHorizontal        (const Tag *tag);
    static const QString getCriteriaGroupe            (const Tag *tag);
    //static const MetadataElement getCriteriaPhase     (const Tag *tag);
    static const QString getCriteriaGroupeFormated    (const Tag *tag);
    static const QString getCriteriaSortFormated      (const Tag *tag);
    static const QString getCriteriaColorFormated     (const Tag *tag);
    static const QString getCriteriaTextFormated      (const Tag *tag);
    static const QString getCriteriaClusterFormated   (const Tag *tag);
    static const QString getCriteriaFilterFormated    (const Tag *tag);
    static const QString getCriteriaHorizontalFormated(const Tag *tag);
    static       bool    isTagLastVersion             (const Tag *tag);


public:
    QDomElement serialize(QDomDocument &xmlDoc) const;
    void deserialize(const QDomElement &xmlElement);
};

#endif // TAG_H
