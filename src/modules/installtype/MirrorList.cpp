#include "MirrorList.h"

#include "GlobalStorage.h"
#include "JobQueue.h"
#include "utils/Logger.h"

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QStringList>

const QString MirrorList::GSMirrorRegionKey = "selectedMirrorRegion";
const QString MirrorList::GSMirrorServerKey = "selectedMirrorServer";

MirrorList::MirrorList( QObject* parent )
    : QObject( parent )
    , m_selectedServerIndex( -1 )
{
}

void
MirrorList::load( QString mirrorListFilePath )
{
    if ( !m_mirrorListFilePath.isEmpty() )
    {
        m_regionNames.clear();
        m_prettyMap.clear();
        m_fullMap.clear();

        emit selectedRegionChanged( "" );
        emit serverIndexChanged( -1 );
    }

    m_mirrorListFilePath = mirrorListFilePath;

    cDebug() << "Mirror List Source Path:" << m_mirrorListFilePath;

    QFile mirrorListFile( m_mirrorListFilePath );
    if ( !mirrorListFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        emit loadFailed( mirrorListFile.errorString() );

        cError() << "Failed to load mirrorlist:" << mirrorListFile.errorString();

        return;
    }

    // read line by line
    bool listStarted = false;
    QString currRegion = "", firstRegion = "";
    int regionCount = 0, urlCount = 0;
    while ( !mirrorListFile.atEnd() )
    {
        QByteArray lineBytes = mirrorListFile.readLine();
        QString line = QString::fromStdString( lineBytes.toStdString() );
        line = line.trimmed();

        if ( line.startsWith( "##" ) && !listStarted )
        {
            // initial comment lines. skip them
            continue;
        }

        if ( line.isEmpty() && !listStarted )
        {
            // there will always be a blank line before starting
            // the actual mirror list
            listStarted = true;
            continue;
        }

        if ( line.isEmpty() )
        {
            // just skip any additional empty line
            continue;
        }

        if ( line.startsWith( "##" ) && listStarted )
        {
            // this is a server link along with its origin country
            QString region = line.remove( "##" ).trimmed();

            // add to the list
            m_regionNames.append( region );
            m_prettyMap[ region ] = QStringList();
            m_fullMap[ region ] = QStringList();

            if ( currRegion == 0 )
            {
                firstRegion = region;
            }

            // keep track of current region
            currRegion = region;
            regionCount++;

            continue;
        }

        if ( line.startsWith( "#Server" ) && listStarted )
        {
            QString fullUrl = line.remove( "#Server =" ).trimmed();
            QString prettyUrl = fullUrl;
            prettyUrl
                = prettyUrl.startsWith( "https://" ) ? prettyUrl.remove( "https://" ) : prettyUrl.remove( "http://" );
            int pos = prettyUrl.indexOf( "/" );
            int len = prettyUrl.length() - pos;
            prettyUrl = prettyUrl.remove( pos, len );

            m_fullMap[ currRegion ].append( fullUrl );
            m_prettyMap[ currRegion ].append( prettyUrl );

            urlCount++;
        }
    }

    mirrorListFile.close();

    cWarning() << "MirrorList:" << regionCount << "regions are found";
    cWarning() << "MirrorList:" << urlCount << "server urls found";

    m_selectedRegion = firstRegion.isEmpty() ? "" : firstRegion;
    m_selectedServerIndex = -1;
    if ( m_prettyMap.find( m_selectedRegion ) != m_prettyMap.end() )
    {
        m_selectedServerIndex = m_prettyMap[ m_selectedRegion ].isEmpty() ? -1 : 0;
    }

    updateGsValues();

    emit loaded();
}

bool
MirrorList::isLoaded() const
{
    return !m_regionNames.isEmpty();
}

int
MirrorList::serverIndex() const
{
    return m_selectedServerIndex;
}

QString
MirrorList::selectedRegion() const
{
    if ( m_selectedRegion.isEmpty() || m_selectedServerIndex < 0 )
    {
        return "";
    }

    return m_selectedRegion;
}

QString
MirrorList::selectedFullServer() const
{
    if ( m_selectedRegion.isEmpty() || m_selectedServerIndex < 0 )
    {
        return "";
    }

    return m_fullMap[ m_selectedRegion ][ m_selectedServerIndex ];
}

QStringList
MirrorList::regionNames() const
{
    return m_regionNames;
}

QStringList
MirrorList::serverFullUrls( QString region ) const
{
    if ( m_fullMap.find( region ) != m_fullMap.end() )
    {
        return m_fullMap[ region ];
    }

    return QStringList();
}

QStringList
MirrorList::serverPrettyUrls( QString region ) const
{
    if ( m_prettyMap.find( region ) != m_prettyMap.end() )
    {
        return m_prettyMap[ region ];
    }

    return QStringList();
}

void
MirrorList::updateGsValues()
{
    auto* gs = Calamares::JobQueue::instance()->globalStorage();
    if ( m_selectedRegion.isEmpty() || m_selectedServerIndex < 0 )
    {
        gs->insert( MirrorList::GSMirrorRegionKey, "" );
        gs->insert( MirrorList::GSMirrorServerKey, "" );
    }
    else
    {
        gs->insert( MirrorList::GSMirrorRegionKey, m_selectedRegion );
        gs->insert( MirrorList::GSMirrorServerKey, m_fullMap[ m_selectedRegion ][ m_selectedServerIndex ] );
    }
}

void
MirrorList::setSelectedRegion( QString region )
{
    if ( region.isEmpty() || m_regionNames.count( region ) <= 0 )
    {
        return;
    }

    m_selectedRegion = region;
    m_selectedServerIndex = m_prettyMap[ m_selectedRegion ].isEmpty() ? -1 : 0;

    updateGsValues();

    emit selectedRegionChanged( m_selectedRegion );
    emit serverIndexChanged( m_selectedServerIndex );
}

void
MirrorList::setServerIndex( int index )
{
    m_selectedServerIndex = index;

    updateGsValues();

    emit serverIndexChanged( m_selectedServerIndex );
}
