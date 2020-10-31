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


#ifndef NO_NET_INSTALL_PAGE_H
#define NO_NET_INSTALL_PAGE_H

#include <QWidget>

namespace Ui
{
class Page_NoNetinst;
};

class NoNetInstallPage : public QWidget
{
    Q_OBJECT

public:
    NoNetInstallPage( QWidget* parent = nullptr );

private:
    Ui::Page_NoNetinst* ui;
};


#endif  // NO_NET_INSTALL_PAGE_H
