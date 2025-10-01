#include <QButtonGroup>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <qmath.h>

#include <kis_display_color_converter.h>

#include "EXChannelSlider.h"
#include "EXColorState.h"
#include "EXKoColorConverter.h"
#include "EXUtils.h"

EXChannelSliders::EXChannelSliders(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto layout = new QVBoxLayout(this);
    auto group = new QButtonGroup(this);
    group->setExclusive(true);
    for (int i = 0; i < 3; ++i) {
        m_channelWidgets[i] = new ChannelValueWidget(i, group, this);
        layout->addWidget(m_channelWidgets[i]);
    }
    setLayout(layout);
}

void EXChannelSliders::setCanvas(KisCanvas2 *canvas)
{
    for (int i = 0; i < 3; ++i) {
        m_channelWidgets[i]->setCanvas(canvas);
        m_channelWidgets[i]->update();
    }
}

ChannelValueWidget::ChannelValueWidget(int channelIndex, QButtonGroup *group, QWidget *parent)
    : QWidget(parent)
    , m_channelIndex(channelIndex)
{
    auto colorState = EXColorState::instance();
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 2, 0, 2);
    auto channelNames = colorState->colorModel()->channelNames();

    m_radioButton = new QRadioButton(channelNames[m_channelIndex], this);
    group->addButton(m_radioButton);
    m_spinBox = new QDoubleSpinBox(this);
    m_bar = new ChannelValueBar(channelIndex, this);

    layout->addWidget(m_bar);
    layout->addWidget(m_spinBox);
    layout->addWidget(m_radioButton);
    setLayout(layout);

    connect(m_spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, colorState](double value) {
        auto [chmn, chmx] = colorState->colorModel()->channelRanges();
        colorState->setChannel(m_channelIndex,
                               (value - chmn[m_channelIndex]) / (chmx[m_channelIndex] - chmn[m_channelIndex]));
    });

    connect(colorState, &EXColorState::sigPrimaryChannelIndexChanged, [this, colorState]() {
        m_radioButton->setChecked(colorState->primaryChannelIndex() == m_channelIndex);
    });

    connect(colorState, &EXColorState::sigColorChanged, [this, colorState]() {
        auto [chmn, chmx] = colorState->colorModel()->channelRanges();
        m_spinBox->setRange(chmn[m_channelIndex], chmx[m_channelIndex]);
        m_spinBox->blockSignals(true);
        m_spinBox->setValue(colorState->color()[m_channelIndex] * (chmx[m_channelIndex] - chmn[m_channelIndex])
                            + chmn[m_channelIndex]);
        m_spinBox->blockSignals(false);
    });

    connect(m_radioButton, &QRadioButton::clicked, this, [this, colorState](bool checked) {
        if (checked) {
            colorState->setPrimaryChannelIndex(m_channelIndex);
        }
    });

    connect(colorState, &EXColorState::sigColorModelChanged, [this, colorState]() {
        m_radioButton->setText(colorState->colorModel()->channelNames()[m_channelIndex]);
    });
}

void ChannelValueWidget::setCanvas(KisCanvas2 *canvas)
{
    m_bar->setCanvas(canvas);
}

ChannelValueBar::ChannelValueBar(int channelIndex, QWidget *parent)
    : QWidget(parent)
    , m_channelIndex(channelIndex)
    , m_dri(nullptr)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(EXColorState::instance(), &EXColorState::sigColorChanged, [this]() {
        updateImage();
        update();
    });

    connect(EXColorState::instance(), &EXColorState::sigColorModelChanged, [this]() {
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

    auto colorState = EXColorState::instance();
    auto converter = EXColorConverter(colorState->colorSpace());
    auto mapper = converter.displayToMemoryPositionMapper();

    auto pixelGet = [this, colorState, mapper](float x, float y, QVector<float> &channels) {
        QVector3D color = colorState->color();
        color[m_channelIndex] = x;
        color = colorState->kritaColorModel()->fromXyz(colorState->colorModel()->toXyz(color));
        if (!colorState->colorModel()->isSrgbBased()) {
            ExtendedUtils::sanitizeOutOfGamutColor(color, QVector3D(0.5, 0.5, 0.5));
        }
        channels[mapper[0]] = color.x(), channels[mapper[1]] = color.y(), channels[mapper[2]] = color.z();
        channels[mapper[3]] = 1;
    };
    m_image = ExtendedUtils::generateGradient(width(), 1, colorState->colorSpace(), m_dri, pixelGet);
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

    int x = EXColorState::instance()->color()[m_channelIndex] * width();
    painter.drawRect(x - 1, 0, 2, height());
}

void ChannelValueBar::mousePressEvent(QMouseEvent *event)
{
}

void ChannelValueBar::mouseMoveEvent(QMouseEvent *event)
{
    qreal value = qBound((qreal)0, (qreal)event->pos().x() / width(), (qreal)1);
    EXColorState::instance()->setChannel(m_channelIndex, value);
}

void ChannelValueBar::mouseReleaseEvent(QMouseEvent *event)
{
    EXColorState::instance()->sendToKrita();
}
