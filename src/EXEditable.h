#ifndef EXEDITABLEGLIMAGE_H
#define EXEDITABLEGLIMAGE_H

#include <QImage>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPointF>
#include <QVector2D>

#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpace.h>
#include <kis_display_color_converter.h>

#include "KisGLImageF16.h"
#include "KisGLImageWidget.h"

class EXEditableImage : public KisGLImageWidget
{
    Q_OBJECT

public:
    explicit EXEditableImage(QWidget *parent = nullptr);
    ~EXEditableImage() override = default;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    virtual void startEdit(QMouseEvent *event, bool isShift) = 0;
    virtual void edit(QMouseEvent *event) = 0;
    virtual void shift(QMouseEvent *event, QVector2D delta) = 0;

    void loadQImage(const QImage &image);
    void loadGLImage(const KisGLImageF16 &image);
    void setStretch(bool stretch);
    void setUseGLImage(bool use)
    {
        m_useGLImage = use;
    }
    bool useGLImage() const
    {
        return m_useGLImage;
    }

Q_SIGNALS:
    void sigValueChangeStarted();
    void sigValueChanging();
    void sigValueFinalized();

protected:
    void setDisplayColorConverter(KisDisplayColorConverter *converter);
    KisDisplayColorConverter *displayColorConverter() const;
    KoColorDisplayRendererInterface *displayRenderer() const;
    const KoColorSpace *generationColorSpace(const KoColorSpace *preferredColorSpace = nullptr) const;

private:
    QPointF m_editStart;
    KisDisplayColorConverter *m_displayColorConverter;
    KoColorDisplayRendererInterface *m_displayRenderer;

    QImage m_cachedQImage;
    bool m_useGLImage;
    bool m_stretch;
};

#endif // EXEDITABLEGLIMAGE_H
