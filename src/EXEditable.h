#ifndef EXEDITABLEGLIMAGE_H
#define EXEDITABLEGLIMAGE_H

#include <QMouseEvent>
#include <QPointF>
#include <QVector2D>

#include "KisGLImageWidget.h"

class KisDisplayColorConverter;
class KoColorDisplayRendererInterface;
class KoColorSpace;

class EXEditableGLImage : public KisGLImageWidget
{
    Q_OBJECT

public:
    explicit EXEditableGLImage(QWidget *parent = nullptr);
    ~EXEditableGLImage() override = default;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void startEdit(QMouseEvent *event, bool isShift) = 0;
    virtual void edit(QMouseEvent *event) = 0;
    virtual void shift(QMouseEvent *event, QVector2D delta) = 0;

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
    KisDisplayColorConverter *m_displayColorConverter = nullptr;
    KoColorDisplayRendererInterface *m_displayRenderer = nullptr;
};

#endif // EXEDITABLEGLIMAGE_H
