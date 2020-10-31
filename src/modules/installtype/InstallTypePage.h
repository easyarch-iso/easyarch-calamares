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


#ifndef INSTALLTYPE_PAGE_H
#define INSTALLTYPE_PAGE_H

#include "Job.h"

#include <QVariantMap>
#include <QWidget>

namespace Ui
{
class Page_InstallType;
}

class MirrorList;

class InstallTypePage : public QWidget
{
    Q_OBJECT
public:
    InstallTypePage( QWidget* parent = nullptr );

    void onActivate();
    void setConfigurationMap( const QVariantMap& configurationMap );
    Calamares::JobList createJobs();

public:
    static const QString GSInstallTypeKey;

private:
    QVariantMap m_config;
    MirrorList* m_mirrorList;
    Ui::Page_InstallType* ui;

private slots:
    void regionChanged( int index );
    void serverIndexChanged( int index );
};

#endif  // INSTALLTYPE_PAGE_H
