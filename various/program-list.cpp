#
/*
 *    Copyright (C) 2026
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the drm module for sdrconnect
 *
 *    drm module is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    drm module is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with drm module; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	<QFile>
#include	<QDomDocument>
#include	"program-list.h"
#include	"radio.h"

	programList::programList (RadioInterface *mr, QString saveName) {
	this	-> saveName	= saveName;
	myWidget	= new QScrollArea (NULL);
	myWidget	-> resize (240, 200);
	myWidget	-> setWidgetResizable(true);

	tableWidget 	= new QTableWidget (0, 2);
	myWidget	-> setWidget(tableWidget);
	tableWidget 	-> setHorizontalHeaderLabels (
	            QStringList () << tr ("station") << tr ("frequency"));
	connect (tableWidget, SIGNAL (cellClicked (int, int)),
	         this, SLOT (tableSelect (int, int)));
	connect (tableWidget, SIGNAL (cellDoubleClicked (int, int)),
	         this, SLOT (removeRow (int, int)));
	connect (this, SIGNAL (newFrequency (int32_t)),
	         mr, SLOT (setFrequency (int32_t)));
	loadTable ();
}

	programList::~programList () {
int16_t	rows	= tableWidget -> rowCount ();
	saveTable ();
	for (int i = rows; i > 0; i --)
	   tableWidget -> removeRow (i);
	delete	tableWidget;
	delete	myWidget;
}

void	programList::show	() {
	myWidget	-> show ();
}

void	programList::hide	() {
	myWidget	-> hide ();
}

bool	programList::isVisible	() {
	return myWidget	-> isVisible ();
}

void	programList::addRow (const QString &name, const QString &freq) {
int16_t	row	= tableWidget -> rowCount ();

	tableWidget	-> insertRow (row);
	QTableWidgetItem *item0	= new QTableWidgetItem;
	item0		-> setTextAlignment (Qt::AlignRight |Qt::AlignVCenter);
	tableWidget	-> setItem (row, 0, item0);

	QTableWidgetItem *item1 = new QTableWidgetItem;
	item1		-> setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	tableWidget	-> setItem (row, 1, item1);

	tableWidget	-> setCurrentItem (item0);
	tableWidget	-> item (row, 0) -> setText (name);
	tableWidget	-> item (row, 1) -> setText (freq);
}
//
//	Locally we dispatch the "click" and "translate"
//	it into a frequency and a call to the main gui to change
//	the frequency

void	programList::tableSelect (int row, int column) {
QTableWidgetItem* theItem = tableWidget  -> item (row, 1);

	(void)column;
	QString theFreq	= theItem -> text ();
	int32_t	freq	= theFreq. toInt ();
	emit newFrequency (freq);
}

void	programList::removeRow (int row, int column) {
	tableWidget	-> removeRow (row);
	(void)column;
}

void	programList::saveTable () {
QDomDocument theProgramList;
QDomElement root	= theProgramList. createElement ("ProgramList");
	theProgramList. appendChild (root);
	for (int i = 0; i < tableWidget -> rowCount (); i ++) {
	   QDomElement elem = theProgramList.
		                     createElement ("Entry");
	   elem. setAttribute ("name", tableWidget -> item (i, 0) -> text ());
	   elem. setAttribute ("freq", tableWidget -> item (i, 1) -> text ());
	   root. appendChild (elem);
	}
	QFile file (this -> saveName);
        if (!file. open (QIODevice::WriteOnly | QIODevice::Text))
           return;

        QTextStream stream (&file);
	stream << theProgramList. toString ();
	file. close ();
}

void	programList::loadTable () {
QDomDocument xmlBOM;
QFile f (this -> saveName);

	if (!f. open (QIODevice::ReadOnly))
           return;

        xmlBOM. setContent (&f);
        f. close ();
	QDomElement root	= xmlBOM. documentElement ();
	QDomElement component	= root. firstChild (). toElement ();
	if (component. isNull ())
	   fprintf (stderr, "The component is er niet\n");
	while (!component. isNull ()) {
	   fprintf (stderr, "Finding component with tag %s\n",
	                                       component. tagName (). toLatin1 (). data ());
	   if (!(component. tagName () == "Entry"))
	      continue;
	   QString name	= component. attribute ("name", "???");
	   QString freq	= component. attribute ("freq", "7880");
	   addRow (name, freq);
	   component = component. nextSibling (). toElement ();
	}
}

