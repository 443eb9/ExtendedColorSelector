#include "ColorState.h"

static ColorStateSP s_instance = nullptr;

ColorStateSP ColorState::instance()
{
    if (!s_instance) {
        s_instance = ColorStateSP(new ColorState());
    }
    return s_instance;
}

ColorConverterSP ColorState::converter() const
{
    return m_converter;
}

void ColorState::sendToKrita()
{
}

void ColorState::syncFromKrita()
{
}

void ColorState::setCanvas(KisCanvas2 *canvas)
{
    if (canvas) {
        m_currentColorSpace = canvas->image()->colorSpace();
    }
}

qreal ColorState::primaryChannelValue() const
{
    return m_color[m_primaryChannelIndex];
}

quint32 ColorState::primaryChannelIndex() const
{
    return m_primaryChannelIndex;
}

QVector2D ColorState::secondaryChannelValues() const
{
    Q_ASSERT(m_primaryChannelIndex < 3);

    switch (m_primaryChannelIndex) {
    case 0:
        return QVector2D(m_color[1], m_color[2]);
    case 1:
        return QVector2D(m_color[0], m_color[2]);
    case 2:
        return QVector2D(m_color[0], m_color[1]);
    default:
        return QVector2D();
    }
}

QVector3D ColorState::color() const
{
    return m_color;
}

const KoColorSpace *ColorState::colorSpace() const
{
    return m_currentColorSpace;
}
