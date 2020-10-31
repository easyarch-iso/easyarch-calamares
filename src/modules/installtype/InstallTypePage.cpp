#include "InstallTypePage.h"
#include "MirrorList.h"
#include "UpdateMirrorListJob.h"

#include "ui_InstallTypePage.h"

#include "GlobalStorage.h"
#include "JobQueue.h"
#include "network/Manager.h"
#include "utils/Logger.h"

#include <QMessageBox>

const QString InstallTypePage::GSInstallTypeKey = "isOnlineInstall";

InstallTypePage::InstallTypePage( QWidget* parent )
    : QWidget( parent )
    , m_mirrorList( new MirrorList() )
    , ui( new Ui::Page_InstallType() )
{
    ui->setupUi( this );

    connect( m_mirrorList, &MirrorList::loaded, [ this ]() {
        // this should be called only once
        // to setup the ui selection
        QStringList regionNames = m_mirrorList->regionNames();
        QString region = m_mirrorList->selectedRegion();
        int index = m_mirrorList->serverIndex();
        QStringList urls = m_mirrorList->serverPrettyUrls( region );
        ui->regionComboBox->clear();
        ui->serverComboBox->clear();

        cDebug() << "Region:" << region;
        cDebug() << "Index:" << index;

        if ( region.isEmpty() || index < 0 )
        {
            return;
        }

        ui->regionComboBox->addItems( regionNames );
        ui->serverComboBox->addItems( urls );
        ui->regionComboBox->setCurrentText( region );
        ui->serverComboBox->setCurrentIndex( index );
    } );
    connect( ui->onlineRadioBtn, &QRadioButton::toggled, [ this ]( bool checked ) {
        auto* gs = Calamares::JobQueue::instance()->globalStorage();
        if ( checked )
        {
            bool hasInternet = CalamaresUtils::Network::Manager::instance().hasInternet();
            if ( hasInternet )
            {
                gs->insert( InstallTypePage::GSInstallTypeKey, checked );
            }
            else
            {
                // we need internet connectivity for online install type
                // also switch back to offline mode, cause installing in online
                // mode without internet makes no sense.
                ui->offlineRadioBtn->setChecked( true );
                QMessageBox::information( this,
                                          tr( "No Internet" ),
                                          tr( "Looks like you are not connected to the internet. But the\n"
                                              "online installation requires internet connectivity.\n"
                                              "Please connect to a network with internet access, "
                                              "then rerun the installer." ),
                                          QMessageBox::StandardButton::Ok );
            }
        }
    } );
    connect( ui->regionComboBox, QOverload< int >::of( &QComboBox::currentIndexChanged ), [ this ]( int index ) {
        regionChanged( index );
    } );
    connect( ui->serverComboBox, QOverload< int >::of( &QComboBox::currentIndexChanged ), [ this ]( int index ) {
        serverIndexChanged( index );
    } );
    connect( ui->updateMirrorCheckBox, &QCheckBox::toggled, [ this ]( bool checked ) {
        auto* gs = Calamares::JobQueue::instance()->globalStorage();
        gs->insert( "updateMirrorList", checked );
    } );
}

void
InstallTypePage::onActivate()
{
    // also set global storage to indicate which type is currently selected
    auto* gs = Calamares::JobQueue::instance()->globalStorage();
    if ( ui->onlineRadioBtn->isChecked() )
    {
        gs->insert( InstallTypePage::GSInstallTypeKey, true );
    }
    else
    {
        gs->insert( InstallTypePage::GSInstallTypeKey, false );
    }

    bool updateMirrorList = gs->value( "updateMirrorList" ).toBool();
    ui->updateMirrorCheckBox->setChecked( updateMirrorList );

    if ( !m_mirrorList->isLoaded() )
    {
        QString mirrorListSrcFile = m_config.value( "mirrorListSourcePath" ).toString();

        cDebug() << "Install Type:"
                 << "calling MirrorList::load"
                 << "source path:" << mirrorListSrcFile;
        m_mirrorList->load( mirrorListSrcFile );
    }
}

void
InstallTypePage::setConfigurationMap( const QVariantMap& configurationMap )
{
    m_config = configurationMap;
    bool updateMirrorList = m_config.value( "updateMirrorList" ).toBool();
    auto* gs = Calamares::JobQueue::instance()->globalStorage();
    gs->insert( "updateMirrorList", updateMirrorList );
}

Calamares::JobList
InstallTypePage::createJobs()
{
    Calamares::JobList jobList;
    auto* gs = Calamares::JobQueue::instance()->globalStorage();
    bool updateMirrorList = gs->value( "updateMirrorList" ).toBool();
    if ( updateMirrorList )
    {
        QString mirrorListDir = m_config.value( "mirrorlistDir" ).toString();
        QString mirrorListFile = m_config.value( "mirrorlistFile" ).toString();

        Calamares::Job* mirrorUpdateJob = new UpdateMirrorListJob( m_mirrorList, mirrorListDir, mirrorListFile );
        Calamares::job_ptr job( mirrorUpdateJob );
        jobList.append( job );
    }
    return jobList;
}

void
InstallTypePage::regionChanged( int index )
{
    QString region = ui->regionComboBox->itemText( index );
    m_mirrorList->setSelectedRegion( region );
    int serverIndex = m_mirrorList->serverIndex();
    QStringList prettyUrls = m_mirrorList->serverPrettyUrls( region );
    ui->serverComboBox->clear();
    ui->serverComboBox->addItems( prettyUrls );
    if ( serverIndex >= 0 )
    {
        ui->serverComboBox->setCurrentIndex( serverIndex );
    }
}

void
InstallTypePage::serverIndexChanged( int index )
{
    m_mirrorList->setServerIndex( index );
}