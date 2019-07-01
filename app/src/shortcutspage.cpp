/*

Pencil - Traditional Animation Software
Copyright (C) 2005-2007 Patrick Corrieri & Pascal Naidon
Copyright (C) 2013-2018 Matt Chiawen Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/
#include "shortcutspage.h"
#include "ui_shortcutspage.h"

#include <QDebug>
#include <QStringRef>
#include <QSettings>
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMessageBox>
#include "pencilsettings.h"


static const int ACT_NAME_COLUMN = 0;
static const int KEY_SEQ_COLUMN  = 1;

ShortcutsPage::ShortcutsPage( QWidget* parent )
    : QWidget(parent),
    ui( new Ui::ShortcutsPage )
{
    ui->setupUi(this);
    m_treeModel = new QStandardItemModel(this);
    m_treeModel->setColumnCount(2);
    m_treeModel->setHorizontalHeaderLabels({ "Action", "Shortcut" });
    treeModelLoadShortcutsSetting();

    ui->treeView->setModel(m_treeModel);
    ui->treeView->resizeColumnToContents(0);

    connect( ui->treeView, &QTreeView::clicked, this, &ShortcutsPage::tableItemClicked );
    connect( ui->keySequenceEdit, &QKeySequenceEdit::editingFinished, this, &ShortcutsPage::keyCapLineEditTextChanged );
    connect( ui->restoreShortcutsButton, &QPushButton::clicked, this, &ShortcutsPage::restoreShortcutsButtonClicked );
    connect( ui->clearButton, &QPushButton::clicked, this, &ShortcutsPage::clearButtonClicked );

    ui->treeView->selectionModel()->select(QItemSelection(m_treeModel->index(0, 0), m_treeModel->index(0, m_treeModel->columnCount() - 1)), QItemSelectionModel::Select);
    tableItemClicked(m_treeModel->index(0, 0));
}

void ShortcutsPage::tableItemClicked( const QModelIndex& modelIndex )
{
    int row = modelIndex.row();

    // extract action name
    QStandardItem* actionItem = m_treeModel->item(row, ACT_NAME_COLUMN);
    ui->actionNameLabel->setText(actionItem->text());

    // extract key sequence
    QStandardItem* keyseqItem = m_treeModel->item(row, KEY_SEQ_COLUMN);
    ui->keySequenceEdit->setKeySequence(keyseqItem->text());

    qDebug() << "Command Selected:" << actionItem->text();

    m_currentItemIndex = modelIndex;

    ui->keySequenceEdit->setFocus();
}

void ShortcutsPage::keyCapLineEditTextChanged()
{
    QKeySequence keySeqence = ui->keySequenceEdit->keySequence();
    if (!m_currentItemIndex.isValid())
    {
        return;
    }

    int row = m_currentItemIndex.row();
    QStandardItem* actionItem = m_treeModel->item(row, ACT_NAME_COLUMN);
    QStandardItem* keyseqItem = m_treeModel->item(row, KEY_SEQ_COLUMN);

    QString strCmdName = QString("Cmd%1").arg( actionItem->text() );
    QString strKeySeq  = keySeqence.toString( QKeySequence::PortableText );

    QSettings setting( PENCIL2D, PENCIL2D );
    setting.beginGroup("shortcuts");

    if (isKeySequenceExist(setting, strCmdName, keySeqence))
    {
        QMessageBox msgBox;
        msgBox.setText( tr("Shortcut Conflict!"));
        msgBox.setInformativeText( tr("%1 is already used, overwrite?").arg(keySeqence.toString(QKeySequence::NativeText)) );
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon( QMessageBox::Warning );

        int result = msgBox.exec();

        if ( result != QMessageBox::Yes )
        {
            ui->keySequenceEdit->setKeySequence( keyseqItem->text() );
            return;
        }
        removeDuplicateKeySequence(&setting, keySeqence);
    }

    setting.setValue(strCmdName, strKeySeq);
    setting.endGroup();
    setting.sync();

    treeModelLoadShortcutsSetting();

    qDebug() << "Shortcut " << strCmdName << " = " << strKeySeq;
}

void ShortcutsPage::restoreShortcutsButtonClicked()
{
    restoreShortcutsToDefault();
    treeModelLoadShortcutsSetting();
}

bool ShortcutsPage::isKeySequenceExist(const QSettings& settings, QString strTargetCmdName, QKeySequence targetkeySeq)
{
    foreach (QString strCmdName, settings.allKeys())
    {
        if (strTargetCmdName == strCmdName)
        {
            continue;
        }

        QString strCmdKeySeq = settings.value(strCmdName).toString();
        if (QKeySequence(strCmdKeySeq) == targetkeySeq)
        {
            return true;
        }
    }
    return false;
}

void ShortcutsPage::removeDuplicateKeySequence(QSettings* settings, QKeySequence keySeq)
{
    foreach(QString strCmdName, settings->allKeys())
    {
        QString strCmdKeySeq = settings->value(strCmdName).toString();

        if ( strCmdKeySeq == keySeq.toString(QKeySequence::PortableText))
        {
            settings->setValue(strCmdName, "");
        }
    }
}

void ShortcutsPage::treeModelLoadShortcutsSetting()
{
    // Load shortcuts from settings
    QSettings settings( PENCIL2D, PENCIL2D );
    settings.beginGroup("shortcuts");

    m_treeModel->setRowCount(settings.allKeys().size());
    Q_ASSERT(m_treeModel->columnCount() == 2);

    int row = 0;
    foreach (QString strCmdName, settings.allKeys())
    {
        QString strKeySequence = settings.value(strCmdName).toString();

        //convert to native format
        strKeySequence = QKeySequence(strKeySequence).toString(QKeySequence::NativeText);

        // strip the first 3 chars "Cmd"
        QStringRef strHumanReadCmdName (&strCmdName, 3, strCmdName.size() - 3);

        if (m_treeModel->item(row , ACT_NAME_COLUMN) == nullptr)
            m_treeModel->setItem(row, ACT_NAME_COLUMN, new QStandardItem());
        if (m_treeModel->item(row, KEY_SEQ_COLUMN) == nullptr)
            m_treeModel->setItem(row, KEY_SEQ_COLUMN, new QStandardItem());

        m_treeModel->item(row, ACT_NAME_COLUMN)->setText(strHumanReadCmdName.toString());
        m_treeModel->item(row, ACT_NAME_COLUMN)->setEditable(false);
        m_treeModel->item(row, KEY_SEQ_COLUMN)->setText(strKeySequence);
        m_treeModel->item(row, KEY_SEQ_COLUMN)->setEditable(false);

        row++;
    }
    settings.endGroup();

    ui->treeView->resizeColumnToContents( 0 );
}

void ShortcutsPage::clearButtonClicked()
{
    if ( !m_currentItemIndex.isValid() )
    {
        return;
    }

    int row = m_currentItemIndex.row();
    QStandardItem* actionItem = m_treeModel->item(row, ACT_NAME_COLUMN);

    QString strCmdName = QString("shortcuts/Cmd%1").arg( actionItem->text() );

    QSettings setting( PENCIL2D, PENCIL2D );
    setting.setValue( strCmdName, "" );
    setting.sync();

    ui->keySequenceEdit->clear();

    treeModelLoadShortcutsSetting();
}
