#include "InstallTypeViewStep.h"
#include "InstallTypePage.h"

#include "utils/Logger.h"

CALAMARES_PLUGIN_FACTORY_DEFINITION( InstallTypeViewStepFactory, registerPlugin< InstallTypeViewStep >(); )

InstallTypeViewStep::InstallTypeViewStep( QObject* parent )
    : Calamares::ViewStep( parent )
    , m_widget( new InstallTypePage() )
{
}

InstallTypeViewStep::~InstallTypeViewStep()
{
    if ( m_widget && m_widget->parent() == nullptr )
    {
        m_widget->deleteLater();
    }
}

QString
InstallTypeViewStep::prettyName() const
{
    return tr( "Install Type" );
}


QWidget*
InstallTypeViewStep::widget()
{
    return m_widget;
}


bool
InstallTypeViewStep::isNextEnabled() const
{
    return true;
}


bool
InstallTypeViewStep::isBackEnabled() const
{
    return true;
}


bool
InstallTypeViewStep::isAtBeginning() const
{
    return true;
}


bool
InstallTypeViewStep::isAtEnd() const
{
    return true;
}


QList< Calamares::job_ptr >
InstallTypeViewStep::jobs() const
{
    return m_widget->createJobs();
}

void
InstallTypeViewStep::onActivate()
{
    m_widget->onActivate();
}

void
InstallTypeViewStep::setConfigurationMap( const QVariantMap& configurationMap )
{
    m_widget->setConfigurationMap( configurationMap );
}