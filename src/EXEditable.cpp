#include "EXEditable.h"

#include <KisDisplayConfig.h>
#include <KisSurfaceColorSpaceWrapper.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <kis_display_color_converter.h>

EXEditableGLImage::EXEditableGLImage(QWidget *parent)
    : KisGLImageWidget(KisSurfaceColorSpaceWrapper::DefaultColorSpace, parent)
{
}

void EXEditableGLImage::mousePressEvent(QMouseEvent *event)
{
    m_editStart = event->pos();
    startEdit(event, event->modifiers().testFlag(Qt::ShiftModifier) || event->modifiers().testFlag(Qt::AltModifier));

    Q_EMIT sigValueChangeStarted();
}

void EXEditableGLImage::mouseMoveEvent(QMouseEvent *event)
{
    auto modifiers = event->modifiers();
    float factor = 1.0;
    if (modifiers.testFlag(Qt::ShiftModifier)) {
        factor = 0.1;
    } else if (modifiers.testFlag(Qt::AltModifier)) {
        factor = 0.01;
    }

    if (factor == 1.0f) {
        edit(event);
    } else {
        shift(event, QVector2D(event->pos() - m_editStart) * factor);
    }

    Q_EMIT sigValueChanging();
}

void EXEditableGLImage::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    Q_EMIT sigValueFinalized();
}

void EXEditableGLImage::setDisplayColorConverter(KisDisplayColorConverter *converter)
{
    if (m_displayColorConverter == converter) {
        return;
    }

    if (m_displayColorConverter) {
        disconnect(m_displayColorConverter, nullptr, this, nullptr);
    }

    m_displayColorConverter = converter;
    m_displayRenderer = m_displayColorConverter ? m_displayColorConverter->displayRendererInterface() : nullptr;
}

KisDisplayColorConverter *EXEditableGLImage::displayColorConverter() const
{
    return m_displayColorConverter;
}

KoColorDisplayRendererInterface *EXEditableGLImage::displayRenderer() const
{
    return m_displayRenderer;
}

const KoColorSpace *EXEditableGLImage::generationColorSpace(const KoColorSpace *preferredColorSpace) const
{
    KoColorSpaceRegistry *registry = KoColorSpaceRegistry::instance();
    const KoColorProfile *outputProfile =
        m_displayColorConverter ? m_displayColorConverter->openGLCanvasSurfaceDisplayConfig().profile : nullptr;
    const KoColorSpace *outputColorSpace = outputProfile
        ? registry->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), outputProfile)
        : nullptr;

    const KoColorSpace *result = preferredColorSpace;

    if (!result && m_displayColorConverter) {
        result = m_displayColorConverter->paintingColorSpace();
    }

    if (!result) {
        result = outputColorSpace;
    }

    if (result && result->colorModelId() != RGBAColorModelID) {
        result = outputColorSpace;
    } else if (result && result->colorDepthId() != Float32BitsColorDepthID) {
        result = registry->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), result->profile());
    }

    if (!result) {
        const KoColorProfile *fallbackProfile = registry->p709SRGBProfile();
        if (fallbackProfile) {
            result = registry->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), fallbackProfile);
        }
    }

    if (result && result->profile() && registry->p2020PQProfile()
        && result->profile()->uniqueId() == registry->p2020PQProfile()->uniqueId()) {
        const KoColorProfile *g10Profile = registry->p2020G10Profile();
        if (g10Profile) {
            const KoColorSpace *g10Space =
                registry->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), g10Profile);
            if (g10Space) {
                result = g10Space;
            }
        }
    }

    return result;
}
