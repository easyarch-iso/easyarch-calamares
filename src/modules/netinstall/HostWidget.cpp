#include "HostWidget.h"
#include "Config.h"
#include "NetInstallPage.h"
#include "NoNetInstallPage.h"

#include "GlobalStorage.h"
#include "JobQueue.h"
#include "utils/Logger.h"

#include <QHBoxLayout>
#include <QPaintEvent>

HostWidget::HostWidget( Config* config, QWidget* parent )
    : QWidget( parent )
    , m_netInstallPage( new NetInstallPage( config ) )
    , m_noNetInstallPage( new NoNetInstallPage() )
    , m_isOnlineInstall( false )
    , m_layout( new QHBoxLayout() )
{
    m_layout->addWidget( m_noNetInstallPage );

    setLayout( m_layout );  // start out as no net install mode
}

void
HostWidget::onActivate()
{
    auto* gs = Calamares::JobQueue::instance()->globalStorage();
    bool isOnlineInstall = gs->value( "isOnlineInstall" ).toBool();

    if ( isOnlineInstall && !m_isOnlineInstall )
    {
        layout()->replaceWidget( m_noNetInstallPage, m_netInstallPage );
    }
    else if ( !isOnlineInstall && m_isOnlineInstall )
    {
        layout()->replaceWidget( m_netInstallPage, m_noNetInstallPage );
    }


    m_isOnlineInstall = isOnlineInstall;
    m_netInstallPage->onActivate();
}

void
HostWidget::setPageTitle( CalamaresUtils::Locale::TranslatedString* title )
{
    m_netInstallPage->setPageTitle( title );
}
