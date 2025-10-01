#include <QButtonGroup>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <qmath.h>

#include <kis_display_color_converter.h>

#include "ColorState.h"
#include "ExtendedChannelValues.h"

ExtendedChannelValues::ExtendedChannelValues(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto layout = new QVBoxLayout(this);
    auto group = new QButtonGroup(this);
    group->setExclusive(true);
    for (int i = 0; i < 3; ++i) {
        m_channelWidgets[i] = new ChannelValueWidget(i, this);
        group->addButton(m_channelWidgets[i]->m_radioButton);
        layout->addWidget(m_channelWidgets[i]);

        connect(m_channelWidgets[i]->m_radioButton, &QRadioButton::toggled, this, [this, i](bool checked) {
            if (checked) {
                ColorState::instance()->setPrimaryChannelIndex(i);
            }
        });
    }
    setLayout(layout);
}

void ExtendedChannelValues::setCanvas(KisCanvas2 *canvas)
{
    for (int i = 0; i < 3; ++i) {
        m_channelWidgets[i]->setCanvas(canvas);
        m_channelWidgets[i]->update();
    }
}

ChannelValueWidget::ChannelValueWidget(int channelIndex, QWidget *parent)
    : QWidget(parent)
    , m_channelIndex(channelIndex)
{
    auto colorState = ColorState::instance();
    auto layout = new QHBoxLayout(this);
    auto channelNames = colorState->colorModel()->channelNames();

    m_radioButton = new QRadioButton(channelNames[m_channelIndex], this);
    m_spinBox = new QDoubleSpinBox(this);
    m_bar = new ChannelValueBar(channelIndex, this);

    layout->addWidget(m_bar);
    layout->addWidget(m_spinBox);
    layout->addWidget(m_radioButton);
    setLayout(layout);

    connect(m_spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, &colorState](double value) {
        colorState->setChannel(m_channelIndex, value);
    });

    connect(colorState.data(), &ColorState::sigPrimaryChannelIndexChanged, [this, colorState]() {
        m_radioButton->setChecked(colorState->primaryChannelIndex() == m_channelIndex);
    });

    connect(colorState.data(), &ColorState::sigColorChanged, [this, colorState]() {
        m_spinBox->blockSignals(true);
        m_spinBox->setValue(colorState->color()[m_channelIndex]);
        m_spinBox->blockSignals(false);
    });

    connect(m_radioButton, &QRadioButton::clicked, this, [this, &colorState](bool checked) {
        if (checked) {
            colorState->setPrimaryChannelIndex(m_channelIndex);
        }
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

    connect(ColorState::instance().data(), &ColorState::sigColorChanged, this, [this]() {
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
        qDebug() << "No DRI available";
        m_image = QImage();
        return;
    }

    ColorStateSP colorState = ColorState::instance();

    const qreal deviceDivider = 1.0 / devicePixelRatioF();
    const int deviceSize = qCeil(width() * deviceDivider);
    const qsizetype pixelSize = colorState->colorSpace()->pixelSize();
    quint32 imageSize = deviceSize * deviceSize * pixelSize;
    QScopedArrayPointer<quint8> raw(new quint8[imageSize]{});
    quint8 *dataPtr = raw.data();
    RGBModel rgbConverter;

    for (int y = 0; y < deviceSize; y++) {
        for (int x = 0; x < deviceSize; x++) {
            qreal value = (qreal)x / (width() - 1);
            QVector3D color = colorState->color();
            color[m_channelIndex] = value;

            KoColor koColor(QColor::fromRgbF(color.x(), color.y(), color.z()), colorState->colorSpace());
            memcpy(dataPtr, koColor.data(), pixelSize);
            dataPtr += pixelSize;
        }
    }

    QImage image = m_dri->toQImage(colorState->colorSpace(), raw.data(), QSize(deviceSize, deviceSize), false);
    image.setDevicePixelRatio(devicePixelRatioF());

    m_image = image;
}

void ChannelValueBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.drawImage(0, 0, m_image, 0, 0, -1, height());

    int x = ColorState::instance()->color()[m_channelIndex] * width();
    painter.drawRect(x - 1, 0, 2, height());
}

void ChannelValueBar::mousePressEvent(QMouseEvent *event)
{
}

void ChannelValueBar::mouseMoveEvent(QMouseEvent *event)
{
    qreal value = qBound((qreal)0, (qreal)event->pos().x() / width(), (qreal)1);
    ColorState::instance()->setPrimaryChannelValue(value);
}

void ChannelValueBar::mouseReleaseEvent(QMouseEvent *event)
{
    ColorState::instance()->sendToKrita();
}
