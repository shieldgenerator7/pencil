/*

Pencil - Traditional Animation Software
Copyright (C) 2012-2018 Matthew Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/
#include "bitmapcoloring.h"
#include "ui_bitmapcoloringwidget.h"
#include "layermanager.h"
#include "qdebug.h"


BitmapColoring::BitmapColoring(Editor* editor, QWidget *parent) :
    BaseDockWidget(parent)

{
    QWidget* innerWidget = new QWidget;
    setWindowTitle(tr("Bitmap Coloring"));

    ui = new Ui::BitmapColoringWidget;
    ui->setupUi(innerWidget);
    setWidget(innerWidget);

    mEditor = editor;
    if (mEditor->layers()->currentLayer()->type() == Layer::BITMAP)
        mLayerBitmap = static_cast<LayerBitmap*>(mEditor->layers()->currentLayer());

    connect(ui->btn1Select, &QPushButton::clicked, mEditor, &Editor::copyFromScan);
    connect(ui->btn1Next, &QPushButton::clicked, mEditor, &Editor::scrubNextKeyFrame);
    connect(ui->sb1_Threshold, QOverload<int>::of(&QSpinBox::valueChanged), mLayerBitmap, &LayerBitmap::setThreshold);
    connect(ui->btnx_blackLine, &QPushButton::clicked, mEditor, &Editor::toBlackLine);

    connect(ui->sb2_fillArea, QOverload<int>::of(&QSpinBox::valueChanged), mLayerBitmap, &LayerBitmap::setWhiteArea);
    connect(ui->btn2_fillRest, &QPushButton::clicked, mEditor, &Editor::fillWhiteAreasRest);
    connect(ui->btn2_repairs, &QPushButton::clicked, mEditor, &Editor::fillWhiteAreas);
    connect(ui->btn3_thinRest, &QPushButton::clicked, mEditor, &Editor::toThinBlackLineRest);
    connect(ui->btn3_repairs, &QPushButton::clicked, mEditor, &Editor::toThinBlackLine);
    connect(ui->btn4_replaceThinLines, &QPushButton::clicked, mEditor, &Editor::replaceThinLines);
}

BitmapColoring::~BitmapColoring()
{
    delete ui;
}

void BitmapColoring::initUI()
{
//    if (!isVisible()) { return; }

    updateUI();
}

void BitmapColoring::updateUI()
{
    if (!isVisible()) { return; }

    Layer* layer = mEditor->layers()->currentLayer();
    setEnabled(true);
    if (layer->type() == Layer::BITMAP && layer->parentId() == -1)
    {
        ui->tabWidgetColor->setEnabled(false);
        ui->tabWidgetScans->setEnabled(true);
        bool colLayerExists = false;
        for (int i = 0; i < mEditor->layers()->count(); i++)
        {
            if (mEditor->layers()->getLayer(i)->parentId() == layer->id())
            {
                ui->labx_3->setText(tr("To Layer: %1").arg(mEditor->layers()->getLayer(i)->name()));
                colLayerExists = true;
            }
        }
        colLayerExists == true ? ui->btnx_blackLine->setEnabled(true) : ui->btnx_blackLine->setEnabled(false);
    }
    else if (layer->type() == Layer::BITMAP && layer->parentId() > -1)
    {
        ui->tabWidgetColor->setEnabled(true);
        ui->tabWidgetScans->setEnabled(false);
    }
    else
    {
        setEnabled(false);
    }
}
