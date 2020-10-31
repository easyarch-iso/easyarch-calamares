/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2020 Corentin Noël <corentin.noel@collabora.com>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#ifndef CLEARMOUNTSJOBTESTS_H
#define CLEARMOUNTSJOBTESTS_H

#include <QObject>
#include <core/device.h>

class CreateLayoutsTests : public QObject
{
    Q_OBJECT
public:
    CreateLayoutsTests();

private Q_SLOTS:
    void testFixedSizePartition();
    void testPercentSizePartition();
    void testMixedSizePartition();
    void init();
    void cleanup();
};

class TestDevice : public Device
{
public:
    TestDevice( const QString& name, const qint64 logicalSectorSize, const qint64 totalLogicalSectors );
};

#endif
