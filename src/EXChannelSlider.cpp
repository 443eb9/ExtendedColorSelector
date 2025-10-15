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

EXChannelSlidersGroup::EXChannelSlidersGroup(QVector<ColorModelId> colorModels,
                                             EXColorStateSP colorState,
                                             EXSettingsStateSP settingsState,
                                             EXColorPatchPopup *colorPatchPopup,
                                             QWidget *parent)
    : QWidget(parent)
    , m_canvas(nullptr)
    , m_colorState(colorState)
    , m_settingsState(settingsState)
    , m_colorPatchPopup(colorPatchPopup)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    resetColorModels(colorModels);
    setLayout(m_layout);
}

void EXChannelSlidersGroup::setCanvas(KisCanvas2 *canvas)
{
    m_canvas = canvas;
    for (auto sliders : m_sliders) {
        sliders->setCanvas(canvas);
        sliders->update();
    }
}

void EXChannelSlidersGroup::resetColorModels(QVector<ColorModelId> colorModels)
{
    for (int i = m_layout->count() - 1; i >= 0; --i) {
        auto item = m_layout->takeAt(i);
        item->widget()->deleteLater();
    }

    m_sliders.clear();
    for (auto modelId : colorModels) {
        auto sliders = new EXChannelSliders(ColorModelFactory::fromId(modelId),
                                            m_colorState,
                                            m_settingsState,
                                            m_colorPatchPopup,
                                            this);
        m_layout->addWidget(sliders);
        m_sliders.append(sliders);
        setCanvas(m_canvas);
    }
}

EXChannelSliders::EXChannelSliders(ColorModelSP colorModel,
                                   EXColorStateSP colorState,
                                   EXSettingsStateSP settingsState,
                                   EXColorPatchPopup *colorPatchPopup,
                                   QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    auto group = new QButtonGroup(this);
    group->setExclusive(true);
    for (int i = 0; i < 3; ++i) {
        m_channelWidgets[i] =
            new ChannelValueWidget(i, group, colorModel, colorState, settingsState, colorPatchPopup, this);
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
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void EXChannelSliders::setCanvas(KisCanvas2 *canvas)
{
    for (int i = 0; i < 3; ++i) {
        m_channelWidgets[i]->setCanvas(canvas);
        m_channelWidgets[i]->update();
    }
}

void EXChannelSliders::resetColorModel(ColorModelSP colorModel)
{
    for (int i = 0; i < 3; ++i) {
        m_channelWidgets[i]->resetColorModel(colorModel);
    }
}

ChannelValueWidget::ChannelValueWidget(int channelIndex,
                                       QButtonGroup *group,
                                       ColorModelSP colorModel,
                                       EXColorStateSP colorState,
                                       EXSettingsStateSP settingsState,
                                       EXColorPatchPopup *colorPatchPopup,
                                       QWidget *parent)
    : QWidget(parent)
    , m_channelIndex(channelIndex)
    , m_colorModel(colorModel)
    , m_colorState(colorState)
    , m_settingsState(settingsState)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 1, 0, 1);
    setFixedHeight(24);
    auto channelNames = colorModel->channelNames();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_radioButton = new QRadioButton(channelNames[m_channelIndex], this);
    m_radioButton->setChecked(colorState->primaryChannelIndex() == m_channelIndex);
    m_label = new QLabel(channelNames[m_channelIndex], this);
    group->addButton(m_radioButton);
    m_spinBox = new QDoubleSpinBox(this);
    m_bar = new ChannelValueBar(channelIndex, colorModel, colorState, settingsState, colorPatchPopup, this);

    layout->addWidget(m_bar);
    layout->addWidget(m_spinBox);
    layout->addWidget(m_radioButton);
    layout->addWidget(m_label);
    setLayout(layout);

    connect(m_spinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            [this, colorState](double value) mutable {
                auto [chmn, chmx] = m_colorModel->channelRanges();
                auto channel = (value - chmn[m_channelIndex]) / (chmx[m_channelIndex] - chmn[m_channelIndex]);
                auto color = m_colorAtCurrentModel;
                color[m_channelIndex] = channel;
                colorState->setColor(m_colorModel->transferTo(colorState->colorModel().data(), color));
                colorState->sendToKrita();
            });

    connect(colorState, &EXColorState::sigPrimaryChannelIndexChanged, this, [this, colorState]() {
        m_radioButton->setChecked(colorState->primaryChannelIndex() == m_channelIndex);
    });

    connect(colorState, &EXColorState::sigColorChanged, this, &ChannelValueWidget::updateSpinBoxRangeAndValue);

    connect(m_radioButton, &QRadioButton::clicked, this, [this, colorState](bool checked) mutable {
        if (checked) {
            colorState->setPrimaryChannelIndex(m_channelIndex);
        }
    });

    connect(colorState, &EXColorState::sigColorModelChanged, this, [this, colorState]() {
        m_radioButton->setText(colorState->colorModel()->channelNames()[m_channelIndex]);
    });

    resetColorModel(colorModel);
    settingsChanged();
}

void ChannelValueWidget::setCanvas(KisCanvas2 *canvas)
{
    m_bar->setCanvas(canvas);
}

void ChannelValueWidget::resetColorModel(ColorModelSP colorModel)
{
    m_colorModel = colorModel;
    m_bar->resetColorModel(colorModel);
    updateSpinBoxRangeAndValue();

    if (colorModel->id() == m_colorState->colorModel()->id()) {
        m_radioButton->show();
        m_label->hide();
    } else {
        m_radioButton->hide();
        m_label->show();
    }
}

void ChannelValueWidget::settingsChanged()
{
    bool showSpinBoxes = m_settingsState->globalSettings.showChannelSpinBoxes;
    m_spinBox->setVisible(showSpinBoxes);
}

void ChannelValueWidget::updateSpinBoxRangeAndValue()
{
    auto [chmn, chmx] = m_colorModel->channelRanges();
    m_colorAtCurrentModel = m_colorState->colorModel()->transferTo(m_colorModel.data(), m_colorState->color());
    m_spinBox->blockSignals(true);
    m_spinBox->setRange(chmn[m_channelIndex], chmx[m_channelIndex]);
    m_spinBox->setValue(m_colorAtCurrentModel[m_channelIndex] * (chmx[m_channelIndex] - chmn[m_channelIndex])
                        + chmn[m_channelIndex]);
    m_spinBox->blockSignals(false);
}

ChannelValueBar::ChannelValueBar(int channelIndex,
                                 ColorModelSP colorModel,
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
    , m_colorModel(colorModel)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_colorAtCurrentModel = m_colorState->colorModel()->transferTo(m_colorModel.data(), m_colorState->color());
    connect(m_colorState, &EXColorState::sigColorChanged, this, [this]() {
        m_colorAtCurrentModel = m_colorState->colorModel()->transferTo(m_colorModel.data(), m_colorState->color());
        updateImage();
        update();
    });

    connect(m_colorState, &EXColorState::sigColorModelChanged, this, [this]() {
        updateImage();
        update();
    });

    connect(m_colorState, &EXColorState::sigColorSpaceChanged, this, [this]() {
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

void ChannelValueBar::resetColorModel(ColorModelSP colorModel)
{
    m_colorModel = colorModel;
    m_colorAtCurrentModel = m_colorState->colorModel()->transferTo(m_colorModel.data(), m_colorState->color());
    updateImage();
    update();
}

void ChannelValueBar::updateImage()
{
    if (!m_dri || !m_colorState->colorSpace()) {
        m_image = QImage();
        return;
    }

    auto makeColorful = m_settingsState->settings[m_colorModel->id()].colorfulHueRing;
    int alphaPos = m_colorState->colorSpace()->alphaPos();
    auto sanitizeOutOfGamut = m_colorState->kritaColorModel()->isSrgbBased() && !m_colorModel->isSrgbBased()
        && m_settingsState->globalSettings.outOfGamutColorEnabled;

    auto pixelGet = [this, makeColorful, sanitizeOutOfGamut, alphaPos](float x, float y) -> QVector4D {
        Q_UNUSED(y);

        QVector3D color = m_colorAtCurrentModel;
        color[m_channelIndex] = x;
        if (makeColorful) {
            m_colorModel->makeColorful(color, m_channelIndex);
        }
        color = m_colorModel->transferTo(m_colorState->kritaColorModel(), color);
        if (sanitizeOutOfGamut) {
            ExtendedUtils::sanitizeOutOfGamutColor(color, m_settingsState->globalSettings.outOfGamutColor);
        }
        auto colorWithAlpha = color.toVector4D();
        colorWithAlpha[alphaPos] = 1.0f;
        return colorWithAlpha;
    };
    m_image = ExtendedUtils::generateGradient(width(),
                                              1,
                                              false,
                                              new EXColorConverter(m_colorState->colorSpace(), m_colorModel),
                                              m_dri,
                                              pixelGet);
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

    if (m_image.isNull()) {
        updateImage();
    }
    painter.drawImage(QRect(0, 0, width(), height()), m_image);

    auto contrastColor = ExtendedUtils::getContrastingColor(m_colorState->qColor());
    painter.setPen(QPen(contrastColor, 1));
    int x = m_colorAtCurrentModel[m_channelIndex] * width();
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
    Q_UNUSED(event);

    float value = qBound(0.f, (float)event->pos().x() / width(), 1.f);
    auto color = m_colorAtCurrentModel;
    color[m_channelIndex] = value;
    m_colorState->setColor(m_colorModel->transferTo(m_colorState->colorModel().data(), color));
}

void ChannelValueBar::shift(QMouseEvent *event, QVector2D delta)
{
    Q_UNUSED(event);

    qreal value = (m_editStart + delta.x()) / width();
    if (ExtendedUtils::testFlag(m_colorState->colorModel()->wrappableChannelIndexBits(), m_channelIndex)) {
        value = value - qFloor(value);
    } else {
        value = qBound(0.0, value, 1.0);
    }

    auto color = m_colorAtCurrentModel;
    color[m_channelIndex] = value;
    m_colorState->setColor(m_colorModel->transferTo(m_colorState->colorModel().data(), color));
}

float ChannelValueBar::currentWidgetCoord()
{
    return (float)(m_colorAtCurrentModel[m_channelIndex] * width());
}
