#ifndef EXSETTINGSDIALOG_H
#define EXSETTINGSDIALOG_H

#include <QCloseEvent>
#include <QDialog>
#include <QListWidget>
#include <QStackedLayout>

#include "EXSettings.h"

class EXPerColorModelSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    EXPerColorModelSettingsDialog(QWidget *parent = nullptr);

private:
    QListWidget *m_pageSwitchers;

    void updateOrder();
    void handleColorModelEnabledChange(QListWidgetItem *item);

    void closeEvent(QCloseEvent *event) override;
};

class EXGlobalSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    EXGlobalSettingsDialog(QWidget *parent = nullptr);

private:
    void closeEvent(QCloseEvent *event) override;
};

#endif
