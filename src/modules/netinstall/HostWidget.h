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


#ifndef NETINSTALL_HOST_WIDGET_H
#define NETINSTALL_HOST_WIDGET_H

#include "locale/TranslatableConfiguration.h"

#include <QWidget>

class Config;
class NetInstallPage;
class NoNetInstallPage;

class QPaintEvent;
class QHBoxLayout;

class HostWidget : public QWidget
{
    Q_OBJECT

public:
    HostWidget( Config* config, QWidget* parent = nullptr );

    void onActivate();
    void setPageTitle( CalamaresUtils::Locale::TranslatedString* title );

private:
    bool m_isOnlineInstall;
    NetInstallPage* m_netInstallPage;
    NoNetInstallPage* m_noNetInstallPage;
    QHBoxLayout* m_layout;
};

#endif  // NETINSTALL_HOST_WIDGET_H
