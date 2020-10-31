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


#ifndef INSTALLTYPE_VIEW_STEP_H
#define INSTALLTYPE_VIEW_STEP_H

#include "DllMacro.h"
#include "utils/PluginFactory.h"
#include "viewpages/ViewStep.h"

#include <QObject>

class InstallTypePage;

class PLUGINDLLEXPORT InstallTypeViewStep : public Calamares::ViewStep
{
    Q_OBJECT

public:
    explicit InstallTypeViewStep( QObject* parent = nullptr );
    virtual ~InstallTypeViewStep() override;

    QString prettyName() const override;

    QWidget* widget() override;

    bool isNextEnabled() const override;
    bool isBackEnabled() const override;

    bool isAtBeginning() const override;
    bool isAtEnd() const override;

    Calamares::JobList jobs() const override;

    void onActivate() override;

    void setConfigurationMap( const QVariantMap& configurationMap ) override;

private:
    InstallTypePage* m_widget;
};

CALAMARES_PLUGIN_FACTORY_DECLARATION( InstallTypeViewStepFactory )

#endif  // INSTALLTYPE_VIEW_STEP_H
