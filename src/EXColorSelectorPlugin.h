#ifndef EXTENDEDCOLORSELECTOR_H
#define EXTENDEDCOLORSELECTOR_H

#include <QObject>
#include <QVariantList>

class EXColorSelectorPlugin : public QObject
{
public:
    EXColorSelectorPlugin(QObject *parent, const QVariantList &);
};

#endif // EXTENDEDCOLORSELECTOR_H
