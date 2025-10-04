#ifndef EXSETTINGSDIALOG_H
#define EXSETTINGSDIALOG_H

#include <QCloseEvent>
#include <QDialog>
#include <QListWidget>
#include <QStackedLayout>

#include "EXSettings.h"
#include "EXSettingsState.h"

class EXPerColorModelSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    EXPerColorModelSettingsDialog(EXSettingsStateSP settingsState, QWidget *parent = nullptr);

private:
    QListWidget *m_pageSwitchers;
    EXSettingsStateSP m_settingsState;

    void updateOrder();
    void handleColorModelEnabledChange(QListWidgetItem *item);

    void closeEvent(QCloseEvent *event) override;
};

class EXGlobalSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    EXGlobalSettingsDialog(EXSettingsStateSP settingsState, QWidget *parent = nullptr);

private:
    EXSettingsStateSP m_settingsState;
    void closeEvent(QCloseEvent *event) override;
};

#endif
