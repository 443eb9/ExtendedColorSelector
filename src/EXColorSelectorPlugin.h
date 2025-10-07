#ifndef EXTENDEDCOLORSELECTOR_H
#define EXTENDEDCOLORSELECTOR_H

#include <QObject>
#include <QVariantList>

class EXColorSelectorPlugin : public QObject
{
public:
    EXColorSelectorPlugin(QObject *parent, const QVariantList &);
    ~EXColorSelectorPlugin() override = default;
};

#endif // EXTENDEDCOLORSELECTOR_H
