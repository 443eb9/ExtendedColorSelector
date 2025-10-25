#include <QButtonGroup>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QVector4D>
#include <qmath.h>

#include <kis_display_color_converter.h>

#include "EXChannelSlider.h"
#include "EXKoColorConverter.h"
#include "EXUtils.h"

EXChannelSlidersGroup::EXChannelSlidersGroup(QVector<ColorModelId> colorModels, QWidget *parent)
    : QWidget(parent)
    , m_canvas(nullptr)
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
        disconnect(item->widget());
        item->widget()->deleteLater();
    }

    m_sliders.clear();
    for (auto modelId : colorModels) {
        auto sliders = new EXChannelSliders(ColorModelFactory::fromId(modelId), this);
        m_layout->addWidget(sliders);
        m_sliders.append(sliders);
        setCanvas(m_canvas);
    }
}

const QVector<EXChannelSliders *> &EXChannelSlidersGroup::sliders() const
{
    return m_sliders;
}

EXChannelSliders::EXChannelSliders(ColorModelSP colorModel, QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    auto group = new QButtonGroup(this);
    group->setExclusive(true);
    for (quint32 i = 0; i < colorModel->channelCount(); ++i) {
        m_channelWidgets.append(new EXChannelSlider(i, colorModel, group, this));
        layout->addWidget(m_channelWidgets.last());
    }
    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void EXChannelSliders::setCanvas(KisCanvas2 *canvas)
{
    for (auto slider : m_channelWidgets) {
        slider->setCanvas(canvas);
        slider->update();
    }
}

void EXChannelSliders::setActive(bool active)
{
    for (auto slider : m_channelWidgets) {
        slider->setActive(active);
    }
}

const QVector<EXChannelSlider *> &EXChannelSliders::sliders() const
{
    return m_channelWidgets;
}

EXChannelSlider::EXChannelSlider(int channelIndex, ColorModelSP colorModel, QButtonGroup *group, QWidget *parent)
    : QWidget(parent)
    , m_channelIndex(channelIndex)
    , m_colorModel(colorModel)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 1, 0, 1);
    setFixedHeight(24);
    auto channelNames = colorModel->channelNames();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_radioButton = new QRadioButton(channelNames[m_channelIndex], this);
    m_label = new QLabel(channelNames[m_channelIndex], this);
    if (group) {
        group->addButton(m_radioButton);
    }
    m_spinBox = new QDoubleSpinBox(this);
    m_bar = new EXChannelSliderBar(channelIndex, colorModel, this);

    layout->addWidget(m_bar);
    layout->addWidget(m_spinBox);
    layout->addWidget(m_radioButton);
    layout->addWidget(m_label);
    setLayout(layout);

    connect(m_spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) mutable {
        auto [chmn, chmx] = m_colorModel->channelRanges();
        auto channel = (value - chmn[m_channelIndex]) / (chmx[m_channelIndex] - chmn[m_channelIndex]);
        m_bar->m_colorAtCurrentModel[m_channelIndex] = channel;
        Q_EMIT m_bar->sigValueChanging();
        Q_EMIT m_bar->sigValueFinalized();
    });

    connect(m_bar, &EXChannelSliderBar::sigValueChanging, this, [this]() {
        updateSpinBoxRangeAndValue();
    });

    connect(m_radioButton, &QRadioButton::clicked, this, [this](bool checked) mutable {
        if (checked) {
            Q_EMIT sigSelected();
        }
    });
}

void EXChannelSlider::setCanvas(KisCanvas2 *canvas)
{
    m_bar->setCanvas(canvas);
}

void EXChannelSlider::setActive(bool active)
{
    m_radioButton->setVisible(active);
    m_label->setVisible(!active);
}

void EXChannelSlider::setColor(QVector3D color, ColorModelSP colorModel)
{
    m_bar->m_colorAtCurrentModel = colorModel->transferTo(m_colorModel.data(), color, m_bar->m_colorAtCurrentModel);
    m_bar->updateImage();
    updateSpinBoxRangeAndValue();
}

void EXChannelSlider::updateSpinBoxRangeAndValue()
{
    auto [chmn, chmx] = m_colorModel->channelRanges();
    m_spinBox->blockSignals(true);
    m_spinBox->setRange(chmn[m_channelIndex], chmx[m_channelIndex]);
    m_spinBox->setValue(m_bar->m_colorAtCurrentModel[m_channelIndex] * (chmx[m_channelIndex] - chmn[m_channelIndex])
                        + chmn[m_channelIndex]);
    m_spinBox->blockSignals(false);
}

void EXChannelSlider::setSanitizeOutOfGamut(bool sanitize, QVector3D outOfGamutColor)
{
    m_bar->m_sanitizeOutOfGamut = sanitize;
    m_bar->m_outOfGamutColor = outOfGamutColor;
    m_bar->updateImage();
}

void EXChannelSlider::setShowChannelSpinBoxes(bool show)
{
    m_spinBox->setVisible(show);
}

void EXChannelSlider::setSelected(bool selected)
{
    m_radioButton->setChecked(selected);
}

void EXChannelSlider::setColorConverter(EXColorConverterSP converter)
{
    m_bar->m_converter = converter;
    m_bar->updateImage();
}

void EXChannelSlider::setColorful(bool colorful)
{
    m_bar->m_colorful = colorful;
    m_bar->updateImage();
}

QPair<ColorModelSP, quint32> EXChannelSlider::colorModelAndChannelIndex() const
{
    return {m_colorModel, m_channelIndex};
}

QVector3D EXChannelSlider::colorAtCurrentModel() const
{
    return m_bar->m_colorAtCurrentModel;
}

EXChannelSliderBar *EXChannelSlider::bar() const
{
    return m_bar;
}

EXChannelSliderBar::EXChannelSliderBar(int channelIndex, ColorModelSP colorModel, QWidget *parent)
    : EXEditable(parent)
    , m_channelIndex(channelIndex)
    , m_dri(nullptr)
    , m_colorModel(colorModel)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void EXChannelSliderBar::setCanvas(KisCanvas2 *canvas)
{
    if (canvas) {
        m_dri = canvas->displayColorConverter()->displayRendererInterface();
    }
}

void EXChannelSliderBar::updateImage()
{
    if (!m_dri || !m_converter) {
        m_image = QImage();
        return;
    }

    int alphaPos = m_converter->colorSpace()->alphaPos();
    auto sanitizeOutOfGamut =
        m_converter->colorModel()->isSrgbBased() && !m_colorModel->isSrgbBased() && m_sanitizeOutOfGamut;

    auto pixelGet = [this, sanitizeOutOfGamut, alphaPos](float x, float y) -> QVector4D {
        Q_UNUSED(y);

        QVector3D color = m_colorAtCurrentModel;
        color[m_channelIndex] = x;
        if (m_colorful) {
            m_colorModel->makeColorful(color, m_channelIndex);
        }
        color = m_colorModel->transferTo(m_converter->colorModel(), color);
        if (sanitizeOutOfGamut) {
            ExtendedUtils::sanitizeOutOfGamutColor(color, m_outOfGamutColor);
        }
        auto colorWithAlpha = color.toVector4D();
        colorWithAlpha[alphaPos] = 1.0f;
        return colorWithAlpha;
    };
    m_image = ExtendedUtils::generateGradient(width(), 1, false, m_converter, m_dri, pixelGet);

    update();
}

void EXChannelSliderBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateImage();
}

void EXChannelSliderBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    if (m_image.isNull()) {
        updateImage();
    }
    painter.drawImage(QRect(0, 0, width(), height()), m_image);

    // auto contrastColor = ExtendedUtils::getContrastingColor(m_colorState->qColor());
    auto contrastColor = QColor(0, 0, 0); // TODO
    painter.setPen(QPen(contrastColor, 1));
    int x = m_colorAtCurrentModel[m_channelIndex] * width();
    painter.drawRect(x - 1, 0, 2, height());
}

void EXChannelSliderBar::startEdit(QMouseEvent *event, bool isShift)
{
    m_editStart = currentWidgetCoord();

    if (!isShift) {
        edit(event);
    }

    Q_EMIT sigValueChangeStarted();
}

void EXChannelSliderBar::mouseReleaseEvent(QMouseEvent *event)
{
    EXEditable::mouseReleaseEvent(event);
    Q_EMIT sigValueFinalized();
}

void EXChannelSliderBar::edit(QMouseEvent *event)
{
    Q_UNUSED(event);

    float value = qBound(0.f, (float)event->pos().x() / width(), 1.f);
    m_colorAtCurrentModel[m_channelIndex] = value;
    Q_EMIT sigValueChanging();
}

void EXChannelSliderBar::shift(QMouseEvent *event, QVector2D delta)
{
    Q_UNUSED(event);

    float value = (m_editStart + delta.x()) / width();
    if (ExtendedUtils::testFlag(m_colorModel->wrappableChannelIndexBits(), m_channelIndex)) {
        value = value - qFloor(value);
    } else {
        value = qBound(0.0f, value, 1.0f);
    }

    m_colorAtCurrentModel[m_channelIndex] = value;
    Q_EMIT sigValueChanging();
}

float EXChannelSliderBar::currentWidgetCoord()
{
    return (float)(m_colorAtCurrentModel[m_channelIndex] * width());
}
