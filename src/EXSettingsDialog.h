#ifndef EXSETTINGSDIALOG_H
#define EXSETTINGSDIALOG_H

#include <QCloseEvent>
#include <QDialog>
#include <QListWidget>
#include <QStackedLayout>

#include "EXColorModel.h"
#include "EXSettings.h"
#include "EXSettingsState.h"

class EXPerColorModelSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    EXPerColorModelSettingsDialog(EXSettingsStateSP settingsState, QWidget *parent = nullptr);
    ~EXPerColorModelSettingsDialog() override = default;

private:
    QListWidget *m_pageSwitchers;
    EXSettingsStateSP m_settingsState;
    QVector<QListWidget *> m_extraSlidersLists;

    void updateColorModelsOrder();
    void updateExtraSlidersOrder(ColorModelId colorModelId);

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
