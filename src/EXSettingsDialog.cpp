#include <QButtonGroup>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QRadioButton>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <qmath.h>

#include "EXSettings.h"
#include "EXSettingsDialog.h"
#include "EXSettingsState.h"
#include "OptionalColorPicker.h"

EXPerColorModelSettingsDialog::EXPerColorModelSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Extended Color Selector Settings");
    auto settingsState = EXSettingsState::instance();
    auto mainLayout = new QHBoxLayout();

    auto pageSwitchers = new QListWidget();
    pageSwitchers->setDropIndicatorShown(true);
    pageSwitchers->setDragDropMode(QListWidget::InternalMove);
    auto pages = new QStackedLayout();

    for (const auto &colorModelId : settingsState->globalSettings.displayOrder) {
        auto &settings = settingsState->settings[colorModelId];
        auto colorModel = ColorModelFactory::fromId(colorModelId);

        pageSwitchers->addItem(colorModel->displayName());
        auto button = pageSwitchers->item(pageSwitchers->count() - 1);
        Q_ASSERT(button);
        button->setFlags(button->flags() | Qt::ItemFlag::ItemIsUserCheckable);
        button->setCheckState(settings.enabled ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

        auto page = new QWidget();
        auto pageLayout = new QVBoxLayout();
        page->setLayout(pageLayout);

        auto barEnabled = new QCheckBox(QString("Enable %1 Bar").arg(colorModel->displayName()));
        barEnabled->setChecked(settings.barEnabled);
        connect(barEnabled, &QCheckBox::clicked, [&settings](bool checked) {
            settings.barEnabled = checked;
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });

        auto colorfulPrimaryChannel = new QCheckBox("Colorful Primary Channel");
        colorfulPrimaryChannel->setChecked(settings.colorfulPrimaryChannel);
        connect(colorfulPrimaryChannel, &QCheckBox::clicked, [&settings](bool checked) {
            settings.colorfulPrimaryChannel = checked;
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });

        auto channelsSpinBoxEnabled = new QCheckBox("Display Channel Values");
        channelsSpinBoxEnabled->setChecked(settings.displayChannels);
        connect(channelsSpinBoxEnabled, &QCheckBox::clicked, [&settings](bool checked) {
            settings.displayChannels = checked;
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });

        auto clipGamutBox = new QCheckBox("Clip Gamut To SRGB Range");
        clipGamutBox->setChecked(settings.clipToSrgbGamut);
        connect(clipGamutBox, &QCheckBox::clicked, [&settings](bool checked) {
            settings.clipToSrgbGamut = checked;
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });

        auto shapeButtonsAndRotLayout = new QHBoxLayout();
        auto shapesGroup = new QButtonGroup();
        for (const auto shapeId : EXShapeFactory::AllShapes) {
            auto shape = EXShapeFactory::fromId(shapeId);

            auto button = new QRadioButton(shape->displayName());
            button->setChecked(shapeId == settings.shape);
            connect(button, &QRadioButton::clicked, [shapeId, &settings]() {
                settings.shape = shapeId;
                Q_EMIT EXSettingsState::instance()->settingsChanged();
            });
            shapeButtonsAndRotLayout->addWidget(button);
            shapesGroup->addButton(button);
        }

        auto wheelRotationBox = new QDoubleSpinBox();
        wheelRotationBox->setMaximum(360);
        wheelRotationBox->setValue(qRadiansToDegrees(settings.rotation));
        connect(wheelRotationBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&settings](double val) {
            settings.rotation = qDegreesToRadians(val);
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });
        shapeButtonsAndRotLayout->addWidget(new QLabel("Rotation"));
        shapeButtonsAndRotLayout->addWidget(wheelRotationBox);

        auto axesSettingsLayout = new QHBoxLayout();
        auto swapAxesButton = new QCheckBox("Swap Axes");
        swapAxesButton->setChecked(settings.swapAxes);
        connect(swapAxesButton, &QCheckBox::clicked, [&settings](bool checked) {
            settings.swapAxes = checked;
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });
        auto reverseXAxisButton = new QCheckBox("Revert X Axis");
        reverseXAxisButton->setChecked(settings.reverseX);
        connect(reverseXAxisButton, &QCheckBox::clicked, [&settings](bool checked) {
            settings.reverseX = checked;
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });
        auto reverseYAxisButton = new QCheckBox("Revert Y Axis");
        reverseYAxisButton->setChecked(settings.reverseY);
        connect(reverseYAxisButton, &QCheckBox::clicked, [&settings](bool checked) {
            settings.reverseY = checked;
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });
        axesSettingsLayout->addWidget(swapAxesButton);
        axesSettingsLayout->addWidget(reverseXAxisButton);
        axesSettingsLayout->addWidget(reverseYAxisButton);

        auto ringSettingsLayouts = new QVBoxLayout();
        auto ringSettingsLayout1 = new QHBoxLayout();
        auto ringThicknessBox = new QDoubleSpinBox();
        ringThicknessBox->setValue(settings.ringThickness);
        connect(ringThicknessBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&settings](double val) {
            settings.ringThickness = val;
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });
        auto ringMarginBox = new QDoubleSpinBox();
        ringMarginBox->setValue(settings.ringMargin);
        connect(ringMarginBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&settings](double val) {
            settings.ringMargin = val;
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });
        ringSettingsLayout1->addWidget(new QLabel("Ring Thickness"));
        ringSettingsLayout1->addWidget(ringThicknessBox);
        ringSettingsLayout1->addWidget(new QLabel("Ring Margin"));
        ringSettingsLayout1->addWidget(ringMarginBox);
        auto ringSettingsLayout2 = new QHBoxLayout();
        auto ringReversed = new QCheckBox("Ring Reversed");
        ringReversed->setChecked(settings.ringReversed);
        connect(ringReversed, &QCheckBox::clicked, [&settings](bool checked) {
            settings.ringReversed = checked;
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });
        auto ringRotation = new QDoubleSpinBox();
        ringRotation->setMaximum(360);
        ringRotation->setValue(qRadiansToDegrees(settings.ringRotation));
        connect(ringRotation, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&settings](double val) {
            settings.ringRotation = qDegreesToRadians(val);
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });
        ringSettingsLayout2->addWidget(ringReversed);
        ringSettingsLayout2->addWidget(new QLabel("Ring Rotation"));
        ringSettingsLayout2->addWidget(ringRotation);
        auto wheelRotateWithRingBox = new QCheckBox("Wheel Rotate With Ring");
        wheelRotateWithRingBox->setChecked(settings.wheelRotateWithRing);
        connect(wheelRotateWithRingBox, &QCheckBox::clicked, [&settings](bool checked) {
            settings.wheelRotateWithRing = checked;
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });
        ringSettingsLayouts->addLayout(ringSettingsLayout1);
        ringSettingsLayouts->addLayout(ringSettingsLayout2);
        ringSettingsLayouts->addWidget(wheelRotateWithRingBox);

        auto ringEnabled = new QCheckBox("Enable Ring");
        ringEnabled->setChecked(settings.ringEnabled);
        connect(ringEnabled, &QCheckBox::clicked, [&settings](bool checked) {
            settings.ringEnabled = checked;
            Q_EMIT EXSettingsState::instance()->settingsChanged();
        });

        pageLayout->addWidget(barEnabled);
        pageLayout->addWidget(colorfulPrimaryChannel);
        pageLayout->addWidget(channelsSpinBoxEnabled);
        if (colorModel->isSrgbBased()) {
            clipGamutBox->deleteLater();
        } else {
            pageLayout->addWidget(clipGamutBox);
        }
        pageLayout->addLayout(shapeButtonsAndRotLayout);
        pageLayout->addLayout(axesSettingsLayout);
        pageLayout->addWidget(ringEnabled);
        pageLayout->addLayout(ringSettingsLayouts);
        pageLayout->addStretch(1);
        pages->addWidget(page);
    }

    connect(pageSwitchers,
            &QListWidget::itemChanged,
            this,
            &EXPerColorModelSettingsDialog::handleColorModelEnabledChange);
    connect(pageSwitchers, &QListWidget::currentItemChanged, this, [pages](QListWidgetItem *current) {
        if (!current) {
            return;
        }
        pages->setCurrentIndex(current->listWidget()->row(current));
    });
    m_pageSwitchers = pageSwitchers;
    auto model = pageSwitchers->model();
    connect(model, &QAbstractItemModel::rowsMoved, this, &EXPerColorModelSettingsDialog::updateOrder);

    mainLayout->addWidget(pageSwitchers);
    mainLayout->addLayout(pages);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

void EXPerColorModelSettingsDialog::updateOrder()
{
    auto widgets = QVector<QListWidgetItem *>();
    for (int i = 0; i < m_pageSwitchers->count(); ++i) {
        widgets.append(m_pageSwitchers->item(i));
    }

    auto &globalSettings = EXSettingsState::instance()->globalSettings;
    globalSettings.displayOrder.clear();

    for (const auto widget : widgets) {
        auto colorModel = ColorModelFactory::fromName(widget->text());
        if (colorModel) {
            globalSettings.displayOrder.append(colorModel->id());
        }
    }

    Q_EMIT EXSettingsState::instance()->settingsChanged();
}

void EXPerColorModelSettingsDialog::handleColorModelEnabledChange(QListWidgetItem *item)
{
    auto colorModel = ColorModelFactory::fromName(item->text());
    if (!colorModel) {
        return;
    }

    auto &settings = EXSettingsState::instance()->settings[colorModel->id()];
    settings.enabled = item->checkState() == Qt::CheckState::Checked;
    Q_EMIT EXSettingsState::instance()->settingsChanged();
}

void EXPerColorModelSettingsDialog::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);
    auto settingsState = EXSettingsState::instance();
    for (auto &settings : settingsState->settings) {
        settings.writeAll();
    }
}

EXGlobalSettingsDialog::EXGlobalSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Extended Color Selector Global Settings");
    auto mainLayout = new QVBoxLayout(this);
    setWindowTitle("Extended Color Selector - Global Settings");
    auto &settings = EXSettingsState::instance()->globalSettings;

    auto outOfGamutColorPicker = new OptionalColorPicker(
        this,
        "Out Of Gamut Color",
        QColor::fromRgbF(settings.outOfGamutColor[0], settings.outOfGamutColor[1], settings.outOfGamutColor[2]));
    outOfGamutColorPicker->setPickingEnabled(settings.outOfGamutColorEnabled);
    connect(outOfGamutColorPicker->colorDialog, &QColorDialog::colorSelected, [&settings](const QColor &color) {
        settings.outOfGamutColor[0] = color.redF();
        settings.outOfGamutColor[1] = color.greenF();
        settings.outOfGamutColor[2] = color.blueF();
        Q_EMIT EXSettingsState::instance()->settingsChanged();
    });
    connect(outOfGamutColorPicker->enableBox, &QCheckBox::clicked, [&settings](bool checked) {
        settings.outOfGamutColorEnabled = checked;
        Q_EMIT EXSettingsState::instance()->settingsChanged();
    });

    auto barHeightLayout = new QHBoxLayout(this);
    auto barHeightBox = new QSpinBox();
    barHeightBox->setValue(settings.barHeight);
    connect(barHeightBox, QOverload<int>::of(&QSpinBox::valueChanged), [&settings](int val) {
        settings.barHeight = val;
        Q_EMIT EXSettingsState::instance()->settingsChanged();
    });
    barHeightLayout->addWidget(new QLabel("Bar Height"));
    barHeightLayout->addWidget(barHeightBox);

    auto dontSyncIfOutOfGamutBox = new QCheckBox("Don't Sync Color From Krita If Out Of Gamut");
    dontSyncIfOutOfGamutBox->setChecked(settings.dontSyncIfOutOfGamut);
    connect(dontSyncIfOutOfGamutBox, &QCheckBox::clicked, [&settings](bool checked) {
        settings.dontSyncIfOutOfGamut = checked;
        Q_EMIT EXSettingsState::instance()->settingsChanged();
    });

    auto portableSelectorSettingsGroup = new QGroupBox("Portable Color Selector");
    auto pSettingsLayouts = new QVBoxLayout();
    auto pSettingsLayout1 = new QHBoxLayout();
    auto pWidthBox = new QSpinBox();
    pWidthBox->setMaximum(1000);
    pWidthBox->setValue(settings.pWidth);
    connect(pWidthBox, QOverload<int>::of(&QSpinBox::valueChanged), [&settings](int val) {
        settings.pWidth = val;
        Q_EMIT EXSettingsState::instance()->settingsChanged();
    });
    auto pBarHeightBox = new QSpinBox();
    pBarHeightBox->setValue(settings.pBarHeight);
    connect(pBarHeightBox, QOverload<int>::of(&QSpinBox::valueChanged), [&settings](int val) {
        settings.pBarHeight = val;
        Q_EMIT EXSettingsState::instance()->settingsChanged();
    });
    pSettingsLayout1->addWidget(new QLabel("Width"));
    pSettingsLayout1->addWidget(pWidthBox);
    pSettingsLayout1->addWidget(new QLabel("Bar Height"));
    pSettingsLayout1->addWidget(pBarHeightBox);
    auto pSettingsLayout2 = new QHBoxLayout();
    auto pEnableColorModelSwitcher = new QCheckBox("Enable Color Model Switcher");
    pEnableColorModelSwitcher->setChecked(settings.pEnableColorModelSwitcher);
    connect(pEnableColorModelSwitcher, &QCheckBox::clicked, [&settings](bool checked) {
        settings.pEnableColorModelSwitcher = checked;
        Q_EMIT EXSettingsState::instance()->settingsChanged();
    });

    pSettingsLayout2->addWidget(pEnableColorModelSwitcher);
    pSettingsLayouts->addLayout(pSettingsLayout1);
    pSettingsLayouts->addLayout(pSettingsLayout2);
    portableSelectorSettingsGroup->setLayout(pSettingsLayouts);

    mainLayout->addWidget(outOfGamutColorPicker);
    mainLayout->addWidget(dontSyncIfOutOfGamutBox);
    mainLayout->addLayout(barHeightLayout);
    mainLayout->addWidget(portableSelectorSettingsGroup);
    mainLayout->addStretch(1);
}

void EXGlobalSettingsDialog::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);
    auto settingsState = EXSettingsState::instance();
    settingsState->globalSettings.writeAll();
}
