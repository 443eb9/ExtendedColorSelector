#include <QVBoxLayout>

#include <kis_action_manager.h>

#include "EXColorModel.h"
#include "EXPortableColorSelector.h"

EXPortableColorSelector::EXPortableColorSelector(QWidget *parent)
    : QDialog(parent)
    , m_toggleAction(nullptr)
    , m_colorState(EXColorState::instance())
    , m_settingsState(EXSettingsState::instance())
{
    setWindowFlag(Qt::WindowType::FramelessWindowHint, true);
    auto mainLayout = new QVBoxLayout(this);

    m_colorPatchPopup = new EXColorPatchPopup(this);
    m_plane = new EXChannelPlane(this);
    m_plane->setColorModel(ColorModelFactory::fromId((ColorModelId)m_settingsState->globalSettings.currentColorModel));
    m_colorState->connectChannelPlane(m_plane);
    m_settingsState->connectChannelPlane(m_plane);
    m_colorPatchPopup->connectToWidget(m_plane);

    connect(m_colorState.data(), &EXColorState::sigColorChanged, this, [this]() {
        m_colorPatchPopup->updateCurrentColor(m_colorState->qColor());
    });
    connect(m_colorState.data(), &EXColorState::sigFGColorUsed, this, [this]() {
        m_colorPatchPopup->updateLastUsedColor(m_colorState->qColor());
    });

    m_dynamicRangeSlider = new EXDynamicRangeSlider(this);
    m_colorState->connectDynamicRangeSlider(m_dynamicRangeSlider);
    mainLayout->addWidget(m_dynamicRangeSlider, 0);

    m_colorModelSwitchers = new EXColorModelSwitchers(m_colorState, m_settingsState, this);
    m_sliders = new EXChannelSlidersGroup(QVector<ColorModelId>(), this);
    mainLayout->addWidget(m_plane);
    mainLayout->addWidget(m_colorModelSwitchers);
    mainLayout->addWidget(m_sliders);

    setLayout(mainLayout);

    connect(m_settingsState, &EXSettingsState::sigSettingsChanged, this, &EXPortableColorSelector::settingsChanged);
    connect(m_colorState.data(), &EXColorState::sigColorModelChanged, this, &EXPortableColorSelector::updateSliders);
    connect(m_colorState.data(), &EXColorState::sigColorModelChanged, m_plane, [this]() {
        m_settingsState->applySettingsToPlane(m_plane);
    });
}

void EXPortableColorSelector::settingsChanged()
{
    auto &settings = m_settingsState->globalSettings;
    m_plane->setMinimumSize(settings.pWidth, settings.pWidth);
    if (settings.pEnableChannelPlane) {
        m_plane->show();
    } else {
        m_plane->hide();
    }

    if (settings.pEnableSliders) {
        updateSliders();
        m_sliders->show();
    } else {
        m_sliders->hide();
    }

    if (settings.pEnableColorModelSwitcher) {
        m_colorModelSwitchers->show();
    } else {
        m_colorModelSwitchers->hide();
    }
}

void EXPortableColorSelector::setViewManager(KisViewManager *kisview)
{
    m_toggleAction = kisview->actionManager()->createAction("toggle_portable_color_selector");
    connect(m_toggleAction, &KisAction::triggered, this, &EXPortableColorSelector::toggle);
}

void EXPortableColorSelector::setCanvas(KisCanvas2 *canvas)
{
    m_plane->setCanvas(canvas);
    m_sliders->setCanvas(canvas);
}

void EXPortableColorSelector::toggle()
{
    if (isVisible()) {
        hide();
    } else {
        move(QCursor::pos() - QPoint(width() / 2, height() / 2));
        activateWindow();
        show();
        setFocus();
    }
}

void EXPortableColorSelector::enterEvent(QEvent *event)
{
    QDialog::enterEvent(event);
    m_colorPatchPopup->updateLastConfirmedColor(m_colorState->qColor());
}

void EXPortableColorSelector::leaveEvent(QEvent *event)
{
    QDialog::leaveEvent(event);
    m_colorPatchPopup->hide();
    hide();
}

void EXPortableColorSelector::keyPressEvent(QKeyEvent *event)
{
    QDialog::keyPressEvent(event);

    if (event->key() == Qt::Key_Escape) {
        hide();
        return;
    }

    if (m_toggleAction && m_toggleAction->shortcut() == QKeySequence(event->key() + int(event->modifiers()))) {
        toggle();
    }
}

void EXPortableColorSelector::updateSliders()
{
    auto &settings = m_settingsState->settings[m_colorState->colorModel()->id()];
    if (settings.slidersEnabled) {
        auto sliders = QVector(settings.extraSliders);
        sliders.prepend(m_colorState->colorModel()->id());
        m_sliders->resetColorModels(sliders);
    } else {
        m_sliders->resetColorModels(settings.extraSliders);
    }

    for (auto sliders : m_sliders->sliders()) {
        for (auto slider : sliders->sliders()) {
            m_colorState->connectChannelSlider(slider);
            m_settingsState->connectChannelSlider(slider);
            m_colorPatchPopup->connectToWidget(slider->bar());
        }
    }
}
