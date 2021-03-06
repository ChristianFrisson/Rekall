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

#include "phases.h"
#include "ui_phases.h"

Phase::Phase(QTreeWidget *parent, const QDateTime &date, const QString &name)
    : QTreeWidgetItem(parent) {
    setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
    setCheckState(0, Qt::Checked);
    setValues(date, name);
}
void Phase::setValues(const QDateTime &date, const QString &name) {
    if(!name.isEmpty())
        setText(0, name);
    setText(1, date.toString("yyyy/MM/dd hh:mm:ss"));
}

Phases::Phases(QWidget *parent) :
    QWidget(parent, Qt::Tool | Qt::FramelessWindowHint),
    ui(new Ui::Phases) {
    ui->setupUi(this);
    needCalulation = false;
    ui->filter->addItem("document date",  "Rekall->Date/Time");
    ui->filter->addItem("import date",    "Rekall->Import Date/Time");
    ui->filter->setCurrentIndex(0);

    phasesByDaysTo.setAction(ui->daysTo);
    phasesByMinsTo.setAction(ui->minsTo);
}

Phases::~Phases() {
    delete ui;
}

void Phases::showEvent(QShowEvent *) {
    emit(displayed(true));
}
void Phases::closeEvent(QCloseEvent *) {
    emit(displayed(false));
}
void Phases::hideEvent(QHideEvent *) {
    emit(displayed(false));
}
const QString Phases::styleSheet2() const {
    return styleSheet() + ui->globalFrameRed->styleSheet();
}
void Phases::setStyleSheet2(const QString &str) {
    ui->globalFrameRed->setStyleSheet(str);
}



void Phases::addCheckEnd() {
    quint16 index = 0;

    if(ui->allow->isChecked()) {
        QDateTime oldDate, startingDate, currentDate;
        bool firstElement = true;
        QMapIterator<QString, MetadataElement> metaElementIterator(elementsToProcess);
        while(metaElementIterator.hasNext()) {
            metaElementIterator.next();
            currentDate = metaElementIterator.value().toDateTime();
            if(firstElement)
                oldDate = startingDate = currentDate;

            if((oldDate.daysTo(currentDate) >= phasesByDaysTo) && ((oldDate.msecsTo(currentDate) / 1000. / 60.) >= phasesByMinsTo)) {
                if(index < ui->checks->topLevelItemCount())  ((Phase*)(ui->checks->topLevelItem(index)))->setValues(currentDate, "");
                else                                         new Phase(ui->checks, currentDate, QString("%2-%1").arg(index+1).arg(tagName));
                index++;
                startingDate = currentDate;
            }
            oldDate = currentDate;
            firstElement = false;
        }
        currentDate = currentDate.addSecs(1);
        if(index < ui->checks->topLevelItemCount())  ((Phase*)(ui->checks->topLevelItem(index)))->setValues(currentDate, "");
        else                                         new Phase(ui->checks, currentDate, QString("%2-%1").arg(index+1).arg(tagName));

        QString text2;
        for(quint16 i = 0 ; i < ui->checks->topLevelItemCount() ; i++)
            if((!ui->checks->topLevelItem(i)->isHidden()) && (ui->checks->topLevelItem(i)->checkState(0) == Qt::Unchecked))
                text2 = ui->filter->currentText();
        emit(actionned(QString("%1 days").arg(ui->daysTo->value()), text2));
    }
    else
        emit(actionned("", ""));

    while(ui->checks->topLevelItemCount() > index)
        delete ui->checks->topLevelItem(ui->checks->topLevelItemCount()-1);

    ui->checks->sortByColumn(1, Qt::AscendingOrder);
    needCalulation = false;
}

bool Phases::isAcceptable(bool, const MetadataElement &value) const {
    if(value.isDate()) {
        for(quint16 i = 0 ; i < ui->checks->topLevelItemCount() ; i++) {
            Phase *phase = (Phase*)ui->checks->topLevelItem(i);
            if(phase->getDate() > value.toDateTime())
                return phase->checkState(0) == Qt::Checked;
        }
    }
    return true;
}
const QString Phases::getPhaseFor(const MetadataElement &value) const {
    if(value.isDate()) {
        quint16 i;
        for(i = 0 ; i < ui->checks->topLevelItemCount() ; i++) {
            Phase *phase = (Phase*)ui->checks->topLevelItem(i);
            if(value.toDateTime() < phase->getDate())
                return QString("%1").arg(i, 4, 10, QChar('0'));
        }
        return QString("%1").arg(i-1, 4, 10, QChar('0'));
    }
    return QString();
}
const QString Phases::getVerbosePhaseFor(const QString &_phaseId) const {
    quint16 phaseId = _phaseId.toUInt();
    if(phaseId < ui->checks->topLevelItemCount())
        return ((Phase*)ui->checks->topLevelItem(phaseId))->getName();
    return QString();
}


void Phases::action() {
    if((sender() == ui->checkAll) || (sender() == ui->uncheckAll) || (sender() == ui->invertAll)) {
        QList<QTreeWidgetItem*> items;
        if(ui->checks->selectedItems().count() > 1)
            items = ui->checks->selectedItems();
        else {
            for(quint16 i = 0 ; i < ui->checks->topLevelItemCount() ; i++)
                if(!ui->checks->topLevelItem(i)->isHidden())
                    items << ui->checks->topLevelItem(i);
        }
        foreach(QTreeWidgetItem *item, items) {
            if(     (sender() == ui->checkAll)   && (item->checkState(0) == Qt::Unchecked))
                item->setCheckState(0, Qt::Checked);
            else if((sender() == ui->uncheckAll) && (item->checkState(0) == Qt::Checked))
                item->setCheckState(0, Qt::Unchecked);
            else if((sender() == ui->invertAll)  && (item->checkState(0) == Qt::Checked))
                item->setCheckState(0, Qt::Unchecked);
            else if((sender() == ui->invertAll)  && (item->checkState(0) == Qt::Unchecked))
                item->setCheckState(0, Qt::Checked);
        }
    }
    else {
        if(ui->filter->count()) {
            QString filterText = ui->filter->itemData(ui->filter->currentIndex()).toString();
            if(filterText.isEmpty())
                filterText = ui->filter->currentText();

            if(filterText.count()) {
                QStringList tagNames = filterText.split("->");
                if(tagNames.count()) {
                    tagName       = tagNames.last();
                    if(tagNames.count() > 1)
                        tagNameCategory = tagNames.first();
                }
            }
        }
        emit(actionned("nothing", ""));
    }

}
void Phases::actionNames() {
    ui->checks->sortByColumn(1, Qt::AscendingOrder);
    emit(actionned("nothing", ""));
}
void Phases::actionSelection() {
    QString suffix = "all";
    if(ui->checks->selectedItems().count() > 1)
        suffix = "selection";

    ui->checkAll  ->setText(tr("Check %1").arg(suffix));
    ui->uncheckAll->setText(tr("Uncheck %1").arg(suffix));
    ui->invertAll ->setText(tr("Invert %1").arg(suffix));
}

QDomElement Phases::serialize(QDomDocument &xmlDoc) const {
    QDomElement xmlData = xmlDoc.createElement("phases");
    for(quint16 i = 0 ; i < ui->checks->topLevelItemCount() ; i++) {
        Phase *phase = (Phase*)ui->checks->topLevelItem(i);
        QDomElement phaseXml = xmlDoc.createElement("phase");
        phaseXml.setAttribute("date" , phase->getDateStr());
        phaseXml.setAttribute("name" , phase->getName());
    }
    return xmlData;
}
void Phases::deserialize(const QDomElement &xmlElement) {
    QString a = xmlElement.attribute("attribut");
}


void Phases::reset(const QString &filterText, qint16 daysTo, qint16 minsTo, bool checked, QStringList checksOnly) {
    ui->allow->setChecked(checked);
    for(quint16 i = 0 ; i < ui->filter->count() ; i++)
        if(ui->filter->itemText(i) == filterText) {
            ui->filter->setCurrentIndex(i);
            break;
        }
    QApplication::processEvents();
    if(checksOnly.count()) {
        for(quint16 i = 0 ; i < ui->checks->topLevelItemCount() ; i++) {
            if(!ui->checks->topLevelItem(i)->isHidden()) {
                if(checksOnly.contains(ui->checks->topLevelItem(i)->text(0)))
                    ui->checks->topLevelItem(i)->setCheckState(0, Qt::Checked);
                else
                    ui->checks->topLevelItem(i)->setCheckState(0, Qt::Unchecked);
            }
        }
    }
    if(daysTo >= 0)
        ui->daysTo->setValue(daysTo);
    if(minsTo >= 0)
        ui->minsTo->setValue(minsTo);
}

void Phases::mouseReleaseEvent(QMouseEvent *) {
    close();
}
