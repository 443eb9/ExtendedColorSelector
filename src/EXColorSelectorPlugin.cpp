#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>
#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include "EXColorSelectorDock.h"
#include "EXColorSelectorPlugin.h"

K_PLUGIN_FACTORY_WITH_JSON(ExtendedColorSelectorPluginFactory,
                           "extendedcolorselector.json",
                           registerPlugin<ExtendedColorSelectorPlugin>();)

class ExtendedColorSelectorFactory : public KoDockFactoryBase
{
public:
    ExtendedColorSelectorFactory()
    {
    }

    QString id() const override
    {
        return QString("ExtendedColorSelector");
    }

    QDockWidget *createDockWidget() override
    {
        ExtendedColorSelectorDock *dockWidget = new ExtendedColorSelectorDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockRight;
    }
};

ExtendedColorSelectorPlugin::ExtendedColorSelectorPlugin(QObject *parent, const QVariantList &)
{
    KoDockRegistry::instance()->add(new ExtendedColorSelectorFactory());
}

extern "C" {
Q_DECL_EXPORT void load_extended_color_selector_plugin()
{
    qDebug() << "Start loading extended color selector plugin...";
    ExtendedColorSelectorPlugin plugin(nullptr, {});
}
}

#include "EXColorSelectorPlugin.moc"
