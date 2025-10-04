#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>
#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include "EXColorSelectorDock.h"
#include "EXColorSelectorPlugin.h"

K_PLUGIN_FACTORY_WITH_JSON(EXColorSelectorPluginFactory,
                           "extendedcolorselector.json",
                           registerPlugin<EXColorSelectorPlugin>();)

class EXColorSelectorFactory : public KoDockFactoryBase
{
public:
    EXColorSelectorFactory()
    {
    }

    QString id() const override
    {
        return QString("ExtendedColorSelector");
    }

    QDockWidget *createDockWidget() override
    {
        EXColorSelectorDock *dockWidget = new EXColorSelectorDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockRight;
    }
};

EXColorSelectorPlugin::EXColorSelectorPlugin(QObject *parent, const QVariantList &)
{
    KoDockRegistry::instance()->add(new EXColorSelectorFactory());
}

extern "C" {
Q_DECL_EXPORT void load_extended_color_selector_plugin()
{
    qDebug() << "Hello Krita, from Extended Color Selector plugin, in C++20!";
    EXColorSelectorPlugin plugin(nullptr, {});
}
}

#include "EXColorSelectorPlugin.moc"
