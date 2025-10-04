#include <QButtonGroup>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QVector4D>
#include <qmath.h>

#include <kis_display_color_converter.h>

#include "EXChannelSlider.h"
#include "EXColorState.h"
#include "EXKoColorConverter.h"
#include "EXSettingsState.h"
#include "EXUtils.h"

EXChannelSliders::EXChannelSliders(EXColorStateSP colorState,
                                   EXSettingsStateSP settingsState,
                                   EXColorPatchPopup *colorPatchPopup,
                                   QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto layout = new QVBoxLayout(this);
    auto group = new QButtonGroup(this);
    group->setExclusive(true);
    for (int i = 0; i < 3; ++i) {
        m_channelWidgets[i] = new ChannelValueWidget(i, group, colorState, settingsState, colorPatchPopup, this);
        layout->addWidget(m_channelWidgets[i]);
    }
    setLayout(layout);

    connect(settingsState.data(), &EXSettingsState::sigSettingsChanged, this, [this, colorState, settingsState]() {
        setVisible(settingsState->settings[colorState->colorModel()->id()].slidersEnabled);
    });

    connect(colorState.data(),
            &EXColorState::sigColorModelChanged,
            this,
            [this, colorState, settingsState](ColorModelId) {
                setVisible(settingsState->settings[colorState->colorModel()->id()].slidersEnabled);
                if (colorState->colorModel()->isOneDimensional()) {
                    m_channelWidgets[1]->hide();
                    m_channelWidgets[2]->hide();
                } else {
                    m_channelWidgets[1]->show();
                    m_channelWidgets[2]->show();
                }
            });
}

void EXChannelSliders::setCanvas(KisCanvas2 *canvas)
{
    for (int i = 0; i < 3; ++i) {
        m_channelWidgets[i]->setCanvas(canvas);
        m_channelWidgets[i]->update();
    }
}

ChannelValueWidget::ChannelValueWidget(int channelIndex,
                                       QButtonGroup *group,
                                       EXColorStateSP colorState,
                                       EXSettingsStateSP settingsState,
                                       EXColorPatchPopup *colorPatchPopup,
                                       QWidget *parent)
    : QWidget(parent)
    , m_channelIndex(channelIndex)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 2, 0, 2);
    auto channelNames = colorState->colorModel()->channelNames();

    m_radioButton = new QRadioButton(channelNames[m_channelIndex], this);
    group->addButton(m_radioButton);
    m_spinBox = new QDoubleSpinBox(this);
    m_bar = new ChannelValueBar(channelIndex, colorState, settingsState, colorPatchPopup, this);

    layout->addWidget(m_bar);
    layout->addWidget(m_spinBox);
    layout->addWidget(m_radioButton);
    setLayout(layout);

    connect(m_spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, colorState](double value) mutable {
        auto [chmn, chmx] = colorState->colorModel()->channelRanges();
        colorState->setChannel(m_channelIndex,
                               (value - chmn[m_channelIndex]) / (chmx[m_channelIndex] - chmn[m_channelIndex]));
    });

    connect(colorState, &EXColorState::sigPrimaryChannelIndexChanged, this, [this, colorState]() {
        m_radioButton->setChecked(colorState->primaryChannelIndex() == m_channelIndex);
    });

    connect(colorState, &EXColorState::sigColorChanged, this, [this, colorState]() {
        auto [chmn, chmx] = colorState->colorModel()->channelRanges();
        m_spinBox->blockSignals(true);
        m_spinBox->setRange(chmn[m_channelIndex], chmx[m_channelIndex]);
        m_spinBox->setValue(colorState->color()[m_channelIndex] * (chmx[m_channelIndex] - chmn[m_channelIndex])
                            + chmn[m_channelIndex]);
        m_spinBox->blockSignals(false);
    });

    connect(m_radioButton, &QRadioButton::clicked, this, [this, colorState](bool checked) mutable {
        if (checked) {
            colorState->setPrimaryChannelIndex(m_channelIndex);
        }
    });

    connect(colorState, &EXColorState::sigColorModelChanged, this, [this, colorState]() {
        m_radioButton->setText(colorState->colorModel()->channelNames()[m_channelIndex]);
    });
}

void ChannelValueWidget::setCanvas(KisCanvas2 *canvas)
{
    m_bar->setCanvas(canvas);
}

ChannelValueBar::ChannelValueBar(int channelIndex,
                                 EXColorStateSP colorState,
                                 EXSettingsStateSP settingsState,
                                 EXColorPatchPopup *colorPatchPopup,
                                 QWidget *parent)
    : EXEditable(parent)
    , m_channelIndex(channelIndex)
    , m_dri(nullptr)
    , m_colorPatchPopup(colorPatchPopup)
    , m_colorState(colorState)
    , m_settingsState(settingsState)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(m_colorState, &EXColorState::sigColorChanged, this, [this]() {
        updateImage();
        update();
    });

    connect(m_colorState, &EXColorState::sigColorModelChanged, this, [this]() {
        updateImage();
        update();
    });
}

void ChannelValueBar::setCanvas(KisCanvas2 *canvas)
{
    if (canvas) {
        m_dri = canvas->displayColorConverter()->displayRendererInterface();
    }
}

void ChannelValueBar::updateImage()
{
    if (m_dri == nullptr) {
        m_image = QImage();
        return;
    }

    auto makeColorful = m_settingsState->settings[m_colorState->colorModel()->id()].colorfulHueRing;

    auto pixelGet = [this, makeColorful](float x, float y) -> QVector4D {
        QVector3D color = m_colorState->color();
        color[m_channelIndex] = x;
        if (makeColorful) {
            m_colorState->colorModel()->makeColorful(color, m_channelIndex);
        }
        color = m_colorState->colorModel()->transferTo(m_colorState->kritaColorModel(), color, nullptr);
        auto settings = m_settingsState->globalSettings;
        if (m_colorState->possibleOutOfSrgb() && settings.outOfGamutColorEnabled) {
            ExtendedUtils::sanitizeOutOfGamutColor(color, settings.outOfGamutColor);
        }
        return QVector4D(color, 1.0f);
    };
    m_image = ExtendedUtils::generateGradient(width(), 1, false, m_colorState->koColorConverter(), m_dri, pixelGet);
}

void ChannelValueBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateImage();
}

void ChannelValueBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.drawImage(QRect(0, 0, width(), height()), m_image);

    auto contrastColor = ExtendedUtils::getContrastingColor(m_colorState->qColor());
    painter.setPen(QPen(contrastColor, 1));
    int x = m_colorState->color()[m_channelIndex] * width();
    painter.drawRect(x - 1, 0, 2, height());
}

void ChannelValueBar::startEdit(QMouseEvent *event, bool isShift)
{
    m_editStart = currentWidgetCoord();

    if (!isShift) {
        edit(event);
    }

    if (m_colorPatchPopup) {
        m_colorPatchPopup->popupAt(mapToGlobal(QPoint()) - QPoint(m_colorPatchPopup->width(), 0));
    }
}

void ChannelValueBar::mouseReleaseEvent(QMouseEvent *event)
{
    EXEditable::mouseReleaseEvent(event);
    m_colorState->sendToKrita();

    if (m_colorPatchPopup && m_settingsState->globalSettings.recordLastColorWhenMouseRelease) {
        m_colorPatchPopup->recordColor();
    }
}

void ChannelValueBar::edit(QMouseEvent *event)
{
    float value = qBound(0.f, (float)event->pos().x() / width(), 1.f);
    m_colorState->setChannel(m_channelIndex, value);
}

void ChannelValueBar::shift(QMouseEvent *event, QVector2D delta)
{
    qreal value = (m_editStart + delta.x()) / width();
    value = value - qFloor(value);
    m_colorState->setChannel(m_channelIndex, value);
}

float ChannelValueBar::currentWidgetCoord()
{
    return (float)(m_colorState->color()[m_channelIndex] * width());
}
