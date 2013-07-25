#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QMenu>
#include "document.h"
#include "cluster.h"
#include "person.h"

class Project : public ProjectBase {
    Q_OBJECT

public:
    explicit Project(QWidget *parent = 0);
    void open(const QDir &dir);

public:
    QList<Document*> documents;
    QList<Person*> persons;
public:
    void addDocument(void *document) { documents.append((Document*)document); }

private:
    qreal categoryColorOpacity, categoryColorOpacityDest;
public:
    QList<Tag*> viewerTags, eventsTags;
    QMap<QString, QMap<QString, QMap<QString, QList<Tag*> > > > timelineSortTags;
    QList< QPair<QRectF,qreal> > timelineCategoriesRectCache;

public:
    qreal nbTagsPerCategories;
    QMap< QPair<QString, QString>, Cluster*> timelineClusters;
    QList<GlText> timelineCategories;
    QList< QPair<QRectF, UiBool*> > guiToggles;
    QList< QPair<QRectF, QPair<QString, QString> > > guiCategories;
    GlRect textureStrips;
    QMenu *timelineFilesMenu;
public:
    Document* getDocument(const QString &name);
    Document* getDocumentAndSelect(const QString &name);

public:
    //QMap<QString, Tag*>;

public:
    void fireEvents();
    qreal totalTime() const;
public:
    const QRectF paintTimeline(bool before = false);
    const QRectF paintViewer();
    bool mouseTimeline(const QPointF &, QMouseEvent *, bool, bool, bool);
    bool mouseViewer  (const QPointF &, QMouseEvent *, bool, bool, bool);

public:
    QPointF getTimelineCursorPos(qreal);
    QPointF getViewerCursorPos  (qreal);
    qreal   getTimelineCursorTime(const QPointF &);
    qreal   getViewerCursorTime  (const QPointF &);
    void askDisplayMetaData() { emit(displayMetaData()); }

public:
    QDomElement serialize(QDomDocument &xmlDoc);
    void deserialize(const QDomElement &xmlElement);
    void save();

signals:
    void displayMetaData();
    void refreshMetadata();
};

#endif // PROJECT_H
