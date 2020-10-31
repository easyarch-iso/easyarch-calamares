/* === This file is part of EasyArch - <https://gitlab.com/easyarch1> ===
 *
 *   Copyright 2020, Asif Mahmud Shimon <ams.eee09@gmail.com>
 *
 *   EasyArch is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   EasyArch is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with EasyArch. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INSTALLTYPE_MIRROR_LIST_H
#define INSTALLTYPE_MIRROR_LIST_H

#include <QMap>
#include <QObject>

class QString;
class QStringList;

class MirrorList : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString selectedRegion READ selectedRegion WRITE setSelectedRegion NOTIFY selectedRegionChanged );
    Q_PROPERTY( int serverIndex READ serverIndex WRITE setServerIndex NOTIFY serverIndexChanged );

public:
    using MirrorMap = QMap< QString, QStringList >;

public:
    MirrorList( QObject* parent = nullptr );

    /**
     * @brief This method is supposed to be called once
     * 
     * Load the mirror list from given file path.
     */
    void load( QString mirrorListFilePath );
    bool isLoaded() const;

    QString selectedRegion() const;
    int serverIndex() const;
    QString selectedFullServer() const;
    QStringList regionNames() const;
    QStringList serverPrettyUrls( QString region ) const;
    QStringList serverFullUrls( QString region ) const;

private:
    void updateGsValues();

public:
    static const QString GSMirrorRegionKey;
    static const QString GSMirrorServerKey;

public slots:
    void setSelectedRegion( QString region );
    void setServerIndex( int index );

signals:
    void loaded();
    void loadFailed( QString error );
    void serverIndexChanged( int index );
    void selectedRegionChanged( QString country );

private:
    int m_selectedServerIndex;
    QString m_selectedRegion;
    QString m_mirrorListFilePath;
    QStringList m_regionNames;
    MirrorMap m_prettyMap, m_fullMap;
};

#endif  // INSTALLTYPE_MIRROR_LIST_H
