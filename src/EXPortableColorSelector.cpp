#include <QVBoxLayout>

#include <kis_action_manager.h>

#include "EXPortableColorSelector.h"
#include "EXSettingsState.h"

EXPortableColorSelector::EXPortableColorSelector(QWidget *parent)
    : QDialog(parent)
    , m_toggleAction(nullptr)
{
    setWindowFlag(Qt::WindowType::FramelessWindowHint, true);
    auto mainLayout = new QVBoxLayout(this);

    m_colorPatchPopup = new EXColorPatchPopup(this);
    m_plane = new EXChannelPlane(m_colorPatchPopup, this);
    m_colorModelSwitchers = new EXColorModelSwitchers(this);
    m_sliders = new EXChannelSliders(m_colorPatchPopup, this);
    mainLayout->addWidget(m_plane);
    mainLayout->addWidget(m_colorModelSwitchers);
    mainLayout->addWidget(m_sliders);

    setLayout(mainLayout);

    connect(EXSettingsState::instance(),
            &EXSettingsState::sigSettingsChanged,
            this,
            &EXPortableColorSelector::settingsChanged);
}

void EXPortableColorSelector::settingsChanged()
{
    auto &settings = EXSettingsState::instance()->globalSettings;
    m_plane->setMinimumSize(settings.pWidth, settings.pWidth);
    if (settings.pEnableChannelPlane) {
        m_plane->show();
    } else {
        m_plane->hide();
    }

    if (settings.pEnableSliders) {
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
        m_colorPatchPopup->recordColor();
        m_colorPatchPopup->shutdown();
    } else {
        move(QCursor::pos() - QPoint(width() / 2, height() / 2));
        activateWindow();
        show();
        setFocus();
    }
}

void EXPortableColorSelector::leaveEvent(QEvent *event)
{
    QDialog::leaveEvent(event);
    m_colorPatchPopup->recordColor();
    m_colorPatchPopup->shutdown();
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
