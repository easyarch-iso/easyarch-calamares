#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# === This file is part of Calamares - <https://github.com/calamares> ===
#
#   Copyright 2014, Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#   Copyright 2015-2017, Teo Mrnjavac <teo@kde.org>
#   Copyright 2016-2017, Kyle Robbertze <kyle@aims.ac.za>
#   Copyright 2017, Alf Gaida <agaida@siduction.org>
#   Copyright 2018, Adriaan de Groot <groot@kde.org>
#   Copyright 2018, Philip Müller <philm@manjaro.org>
#   Copyright 2020, Asif Mahmud Shimon <ams.eee09@gmail.com>
#
#   Calamares is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   Calamares is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with Calamares. If not, see <http://www.gnu.org/licenses/>.

import os
import abc
from string import Template
import subprocess
import json

import requests

import libcalamares
from libcalamares.utils import check_target_env_call, target_env_call
from libcalamares.utils import gettext_path, gettext_languages

import gettext

_translation = gettext.translation(
    "calamares-python",
    localedir=gettext_path(),
    languages=gettext_languages(),
    fallback=True,
)
_ = _translation.gettext
_n = _translation.ngettext


total_packages = 0  # For the entire job
completed_packages = 0  # Done so far for this job

INSTALL = object()
REMOVE = object()
mode_packages = None  # Changes to INSTALL or REMOVE

status = ""


def _change_mode(mode):
    global mode_packages
    mode_packages = mode
    libcalamares.job.setprogress(completed_packages * 1.0 / total_packages)


def pretty_name():
    return _("Install packages.")


def pretty_status_message():
    return status


def report_progress(package_count=0):
    global completed_packages

    # avoid division by zero
    if not total_packages:
        return

    completed_packages += package_count
    progress = (completed_packages * 1.0) / total_packages
    libcalamares.job.setprogress(progress)
    libcalamares.utils.debug(
        "Pretty name: {!s}, setting progress to {}".format(
            pretty_name(),
            progress,
        )
    )


class PackageManager(metaclass=abc.ABCMeta):
    """
    Package manager base class. A subclass implements package management
    for a specific backend, and must have a class property `backend`
    with the string identifier for that backend.

    Subclasses are collected below to populate the list of possible
    backends.
    """

    backend = None

    @abc.abstractmethod
    def install(self, pkgs, from_local=False):
        """
        Install a list of packages (named) into the system.
        Although this handles lists, in practice it is called
        with one package at a time.

        @param pkgs: list[str]
            list of package names
        @param from_local: bool
            if True, then these are local packages (on disk) and the
            pkgs names are paths.
        """
        pass

    @abc.abstractmethod
    def remove(self, pkgs):
        """
        Removes packages.

        @param pkgs: list[str]
            list of package names
        """
        pass

    @abc.abstractmethod
    def update_db(self):
        pass

    def run(self, script):
        if script != "":
            check_target_env_call(script.split(" "))

    def install_package(self, packagedata, from_local=False):
        """
        Install a package from a single entry in the install list.
        This can be either a single package name, or an object
        with pre- and post-scripts. If @p packagedata is a dict,
        it is assumed to follow the documented structure.

        @param packagedata: str|dict
        @param from_local: bool
            see install.from_local
        """
        if isinstance(packagedata, str):
            self.install([packagedata], from_local=from_local)
        else:
            self.run(packagedata["pre-script"])
            self.install([packagedata["package"]], from_local=from_local)
            self.run(packagedata["post-script"])

    def remove_package(self, packagedata):
        """
        Remove a package from a single entry in the remove list.
        This can be either a single package name, or an object
        with pre- and post-scripts. If @p packagedata is a dict,
        it is assumed to follow the documented structure.

        @param packagedata: str|dict
        """
        if isinstance(packagedata, str):
            self.remove([packagedata])
        else:
            self.run(packagedata["pre-script"])
            self.remove([packagedata["package"]])
            self.run(packagedata["post-script"])


class PMPacmanWrapper(PackageManager):
    backend = "pacman-wrapper"
    # NOTE: this is a possible breaking change. if live user group
    # changes at later point, and this is not set as such, the
    # installer will fail to install any package from aur.
    liveuser = "live"
    livegroup = "users"

    def _can_pacman_install(self, pkg) -> bool:
        res = target_env_call(["pacman", "-Ss", "--quiet", pkg])
        return res == 0

    def _get_aur_info(self, pkg) -> [dict, None]:
        res = requests.get(
            "https://aur.archlinux.org/rpc.php",
            params={
                "v": "5",
                "type": "info",
                "by": "name",
                "arg": pkg,
            },
        )
        if res.status_code != 200:
            return None
        data = res.json()
        if data["resultcount"] == 0:
            return None
        for result in data["results"]:
            if result["Name"] == pkg:
                return result
        return None

    def _install_deps(self, deps):
        if not deps:
            return
        deps = [deps] if isinstance(deps, str) else deps
        check_target_env_call(
            [
                "pacman",
                "-S",
                "--noconfirm",
            ]
            + deps
        )

    def _install_one_pkg(self, pkg, from_local=False):
        if from_local:
            check_target_env_call(["pacman", "-U", "--noconfirm", pkg])
        else:
            if self._can_pacman_install(pkg):
                check_target_env_call(["pacman", "-S", "--noconfirm", pkg])
                return
            # try installing from AUR

            # collect package info from AUR
            pkginfo = self._get_aur_info(pkg)
            if not pkginfo:
                check_target_env_call(["rm", "-rf", self.cachedir])
                libcalamares.utils.warning(f"Could not get info from AUR.")
                return

            # install make dependencies, if there are any
            makedeps = pkginfo.get("MakeDepends", None)
            if makedeps:
                self._install_deps(makedeps)

            # install dependencies, if there are any
            deps = pkginfo.get("Depends", None)
            if deps:
                self._install_deps(deps)

            # run the custom aur helper from the iso
            pkgurl = "https://aur.archlinux.org"
            pkgurl += "" if pkginfo["URLPath"].startswith("/") else "/"
            pkgurl += pkginfo["URLPath"]
            pkgtar = os.path.split(pkgurl)[1]
            check_target_env_call(
                [
                    "/usr/local/bin/aurpkg.sh",
                    f"{self.liveuser}",
                    f"{self.livegroup}",
                    f"{pkg}",
                    f"{pkgtar}",
                    f"{pkgurl}",
                ]
            )

    def install(self, pkgs, from_local=False):
        for pkg in pkgs:
            self._install_one_pkg(pkg, from_local)

    def remove(self, pkgs):
        check_target_env_call(["pacman", "-Rs", "--noconfirm"] + pkgs)

    def update_db(self):
        check_target_env_call(["pacman", "-Sy"])

    def update_system(self):
        check_target_env_call(["pacman", "-Su", "--noconfirm"])


# Collect all the subclasses of PackageManager defined above,
# and index them based on the backend property of each class.
backend_managers = [
    (c.backend, c)
    for c in globals().values()
    if type(c) is abc.ABCMeta and issubclass(c, PackageManager) and c.backend
]


def count_total_packages_to_process(operations):
    """
    Count and return total number of packages to process.

    Processing involves install, try_install, remove,
    try_remove etc operations.

    :param operations: List[Dict]

    :return: int
    """
    _total_packages = 0

    package_listing_keys = [
        "install",
        "try_install",
        "remove",
        "try_remove",
        "localInstall",
    ]

    for entry in operations:
        for key in entry.keys():
            package_list = entry[key]
            if key in package_listing_keys:
                _total_packages += len(package_list)
    return _total_packages


def subst_locale(plist):
    """
    Returns a locale-aware list of packages, based on @p plist.
    Package names that contain LOCALE are localized with the
    BCP47 name of the chosen system locale; if the system
    locale is 'en' (e.g. English, US) then these localized
    packages are dropped from the list.

    @param plist: list[str|dict]
        Candidate packages to install.
    @return: list[str|dict]
    """
    locale = libcalamares.globalstorage.value("locale")
    if not locale:
        # It is possible to skip the locale-setting entirely.
        # Then pretend it is "en", so that {LOCALE}-decorated
        # package names are removed from the list.
        locale = "en"

    ret = []
    for packagedata in plist:
        if isinstance(packagedata, str):
            packagename = packagedata
        else:
            packagename = packagedata["package"]

        # Update packagename: substitute LOCALE, and drop packages
        # if locale is en and LOCALE is in the package name.
        if locale != "en":
            packagename = Template(packagename).safe_substitute(LOCALE=locale)
        elif "LOCALE" in packagename:
            packagename = None

        if packagename is not None:
            # Put it back in packagedata
            if isinstance(packagedata, str):
                packagedata = packagename
            else:
                packagedata["package"] = packagename

            ret.append(packagedata)

    return ret


def run_operations(pkgman, entry):
    """
    Call package manager with suitable parameters for the given
    package actions.

    :param pkgman: PackageManager
        This is the manager that does the actual work.
    :param entry: dict
        Keys are the actions -- e.g. "install" -- to take, and the values
        are the (list of) packages to apply the action to. The actions are
        not iterated in a specific order, so it is recommended to use only
        one action per dictionary. The list of packages may be package
        names (strings) or package information dictionaries with pre-
        and post-scripts.
    """
    global completed_packages, mode_packages, status

    # dict_str = json.dumps(entry)
    # libcalamares.utils.debug(
    #     "{} ! Processing entry: \n{}".format(
    #         pretty_name(),
    #         dict_str,
    #     )
    # )

    status = "Processing packages..."
    report_progress()

    for key in entry.keys():
        package_list = subst_locale(entry[key])
        if key == "install":
            # libcalamares.utils.debug(
            #     "{} ! Mode {}".format(
            #         pretty_name(),
            #         "INSTALL",
            #     )
            # )
            # _change_mode(INSTALL)
            if all([isinstance(x, str) for x in package_list]):
                for package in package_list:
                    status = "Installing {}... ({}/{})".format(
                        package,
                        completed_packages + 1,
                        total_packages,
                    )
                    report_progress()
                    pkgman.install_package(package)
                    report_progress(1)
            else:
                for package in package_list:
                    status = "Installing {}... ({}/{})".format(
                        package,
                        completed_packages + 1,
                        total_packages,
                    )
                    report_progress()
                    pkgman.install_package(package)
                    report_progress(1)
        elif key == "try_install":
            # libcalamares.utils.debug(
            #     "{} ! Mode {}".format(
            #         pretty_name(),
            #         "TRY_INSTALL",
            #     )
            # )
            # _change_mode(INSTALL)

            # we make a separate package manager call for each package so a
            # single failing package won't stop all of them
            for package in package_list:
                try:
                    status = "Installing {}... ({}/{})".format(
                        package,
                        completed_packages + 1,
                        total_packages,
                    )
                    report_progress()
                    pkgman.install_package(package)
                    report_progress(1)
                except subprocess.CalledProcessError:
                    warn_text = "Could not install package "
                    warn_text += str(package)
                    libcalamares.utils.warning(warn_text)
        elif key == "remove":
            # libcalamares.utils.debug(
            #     "{} ! Mode {}".format(
            #         pretty_name(),
            #         "REMOVE",
            #     )
            # )
            # _change_mode(REMOVE)
            if all([isinstance(x, str) for x in package_list]):
                for package in package_list:
                    status = "Removing {}... ({}/{})".format(
                        package,
                        completed_packages + 1,
                        total_packages,
                    )
                    report_progress()
                    pkgman.remove_package(package)
                    report_progress(1)
            else:
                for package in package_list:
                    status = "Removing {}... ({}/{})".format(
                        package,
                        completed_packages + 1,
                        total_packages,
                    )
                    report_progress()
                    pkgman.remove_package(package)
                    report_progress(1)
        elif key == "try_remove":
            # libcalamares.utils.debug(
            #     "{} ! Mode {}".format(
            #         pretty_name(),
            #         "TRY_REMOVE",
            #     )
            # )
            # _change_mode(REMOVE)

            for package in package_list:
                try:
                    status = "Removing {}... ({}/{})".format(
                        package,
                        completed_packages + 1,
                        total_packages,
                    )
                    report_progress()
                    pkgman.remove_package(package)
                    report_progress(1)
                except subprocess.CalledProcessError:
                    warn_text = "Could not remove package "
                    warn_text += str(package)
                    libcalamares.utils.warning(warn_text)
        elif key == "localInstall":
            # libcalamares.utils.debug(
            #     "{} ! Mode {}".format(
            #         pretty_name(),
            #         "LOCAL_INSTALL",
            #     )
            # )
            # _change_mode(INSTALL)
            if all([isinstance(x, str) for x in package_list]):
                for package in package_list:
                    status = "Installing {}... ({}/{})".format(
                        package,
                        completed_packages + 1,
                        total_packages,
                    )
                    report_progress()
                    pkgman.install_package(package, from_local=True)
                    report_progress(1)
            else:
                for package in package_list:
                    status = "Installing {}... ({}/{})".format(
                        package,
                        completed_packages + 1,
                        total_packages,
                    )
                    report_progress()
                    pkgman.install_package(package, from_local=True)
                    report_progress(1)
        elif key == "source":
            libcalamares.utils.debug("Package-list from {!s}".format(entry[key]))
        else:
            libcalamares.utils.warning(
                "Unknown package-operation key {!s}".format(key),
            )
    # _change_mode(None)


def run():
    """
    Calls routine with detected package manager to install locale packages
    or remove drivers not needed on the installed system.

    :return:
    """
    global mode_packages, total_packages, completed_packages, status

    if not libcalamares.globalstorage.value("isOnlineInstall"):
        libcalamares.utils.warning(
            "Running in offline mode. Skipping Package Installation."
        )
        return None

    backend = libcalamares.job.configuration.get("backend")

    for identifier, impl in backend_managers:
        if identifier == backend:
            pkgman = impl()
            break
    else:
        return "Bad backend", 'backend="{}"'.format(backend)

    skip_this = libcalamares.job.configuration.get("skip_if_no_internet", False)
    if skip_this and not libcalamares.globalstorage.value("hasInternet"):
        libcalamares.utils.warning("Package installation has been skipped: no internet")
        return None

    update_db = libcalamares.job.configuration.get("update_db", False)
    if update_db and libcalamares.globalstorage.value("hasInternet"):
        status = "Updating package database..."
        libcalamares.job.setprogress(0.0)
        pkgman.update_db()

    update_system = libcalamares.job.configuration.get("update_system", False)
    if update_system and libcalamares.globalstorage.value("hasInternet"):
        status = "Updating installed packages..."
        libcalamares.job.setprogress(0.0)
        pkgman.update_system()

    operations = libcalamares.job.configuration.get("operations", [])
    if libcalamares.globalstorage.contains("packageOperations"):
        operations += libcalamares.globalstorage.value("packageOperations")

    mode_packages = None
    total_packages = count_total_packages_to_process(operations)
    completed_packages = 0

    if not total_packages:
        # Avoids potential divide-by-zero in progress reporting
        libcalamares.utils.debug("No package to process")
        return None

    libcalamares.job.setprogress(0.0)
    libcalamares.utils.debug(
        "Pretty Name! {} Total number of packages - {}".format(
            pretty_name(),
            total_packages,
        )
    )

    for entry in operations:
        libcalamares.utils.debug(pretty_name())
        run_operations(pkgman, entry)

    mode_packages = None

    libcalamares.job.setprogress(1.0)
    libcalamares.utils.debug(pretty_name())

    return None
