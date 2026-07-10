#
/*
 *    Copyright (C)  2026
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of Qt-DAB
 *
 *    Qt-DAB is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    Qt-DAB is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Qt-DAB; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	<QSettings>
#include	<QPen>
#include	<QLabel>
#include	<QChart>
#include        <QColorDialog>
#include	<QGraphicsLayout>
#include	"basic-scope.h"
#include	"settings-handler.h"

	basicScope::basicScope (clickableChart *plotArea,
	                        QSettings *dabSettings,
	                        const QString &scopeName) {
QString	colorString;

	this	-> plotArea		= plotArea;
	this	-> scopeSettings	= dabSettings;
	this	-> displaySize		= 512;
	this	-> scopeName		= scopeName;

	colorString     = value_s (scopeSettings, scopeName,
                                               "displayColor", "black");

	theChart	= new QChart ();
	theChart	-> setBackgroundBrush (QBrush (QColor ("colorString")));
        theChart	 -> legend () -> hide ();
        theChart	 -> layout () -> setContentsMargins(0, 0, 0, 0);
        theChart	 -> setMargins (QMargins (2, 2, 2, 2));

	X_axis		= new QValueAxis();
	X_axis	-> 	setLabelsColor (Qt::lightGray);
        colorString     = value_s (scopeSettings, scopeName,
                                               "gridColor", "5e5c64");
	X_axis	->	setGridLineColor (QColor (colorString));
	X_axis	->	setGridLineVisible (true);
	X_axis	->	setMinorGridLineVisible (false);
	X_axis	->	setTickCount (6);
//	X_axis  ->      setMinorTickCount (1);

	Y_axis		= new QValueAxis();
	Y_axis	->	setLabelsColor (Qt::lightGray);
	Y_axis	->	setGridLineColor (QColor (colorString));
	Y_axis  ->      setGridLineVisible (true);
        Y_axis  ->      setMinorGridLineVisible (false);
        Y_axis  ->      setTickCount (4);
//	Y_axis  ->      setMinorTickCount (1);

	theChart	-> addAxis (X_axis, Qt::AlignBottom);
	theChart	-> addAxis (Y_axis, Qt::AlignLeft);
	plotArea	-> setChart (theChart);

	connect (plotArea, &clickableChart::clicked_right,
	         this, &basicScope::rightMouseClick);
	ValueLine_1	= new QLineSeries ();
	ValueLine_2	= new QLineSeries ();
	colorString     = value_s (scopeSettings, scopeName,
                                               "curveColor", "#f9f06b");
	ValueLine_1	-> setPen (QPen (QColor (colorString), 2.0));
	ValueLine_2	-> setPen (QPen (QColor ("red"), 2.0));
	theChart	-> addSeries (ValueLine_1);
	theChart	-> addSeries (ValueLine_2);
	ValueLine_1	-> attachAxis (X_axis);
	ValueLine_2	-> attachAxis (X_axis);
	ValueLine_1	-> attachAxis (Y_axis);
	ValueLine_2	-> attachAxis (Y_axis);
}

	basicScope::~basicScope	() {
}

void	basicScope::showSpectrum (double *data_1, double *data_2,
	                          int amount,
	                         float x_min, float x_max,
	                         float y_min, float y_max) {
	X_axis	-> setRange	(x_min, x_max);
	Y_axis	-> setRange	(y_min, y_max);

	QList<QPointF> Y_1_Values;
        Y_1_Values. reserve (amount);
	QList<QPointF> Y_2_Values;
        Y_2_Values. reserve (amount);
        for (uint16_t i = 0; i < amount; i++) {
	   Y_1_Values. append (QPointF (i * (x_max - x_min) / amount + x_min,
	                                                        data_1 [i]));
	   Y_2_Values. append (QPointF (i * (x_max - x_min) / amount + x_min,
	                                                        data_2 [i]));
	}
	ValueLine_1 -> replace (Y_1_Values);
	ValueLine_2 -> replace (Y_2_Values);
}

void	basicScope::rightMouseClick	() {
QColor	displayColor;
QColor	gridColor;
QColor	curveColor;

	displayColor =
	        QColorDialog::getColor (Qt::black, nullptr, "displayColor");
        if (!displayColor. isValid ())
           return;
        gridColor = QColorDialog::getColor (Qt::black, nullptr, "gridColor");
        if (!gridColor. isValid ())
           return;
        curveColor = QColorDialog::getColor (Qt::yellow, nullptr, "curveColor");
        if (!curveColor. isValid ())
           return;
	store (scopeSettings, scopeName,
	                    "displayColor", displayColor. name ());
	store (scopeSettings, scopeName, "gridColor", gridColor. name ());
	store (scopeSettings, scopeName, "curveColor", curveColor. name ());

	theChart	-> setBackgroundBrush (QBrush (displayColor));
	X_axis		-> setGridLineColor (gridColor);
	Y_axis		-> setGridLineColor (gridColor);
        ValueLine_1	-> setPen (QPen (curveColor, 2.0));
}
