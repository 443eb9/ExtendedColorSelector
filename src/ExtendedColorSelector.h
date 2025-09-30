#ifndef EXTENDEDCOLORSELECTOR_H
#define EXTENDEDCOLORSELECTOR_H

#include <QObject>
#include <QVariantList>

class ExtendedColorSelectorPlugin : public QObject
{
public:
    ExtendedColorSelectorPlugin(QObject *parent, const QVariantList &);
};

#endif // EXTENDEDCOLORSELECTOR_H
