import libcalamares
from libcalamares.utils import check_target_env_call
from libcalamares.utils import gettext_path, gettext_languages

import gettext

_ = gettext.translation(
    "calamares-python",
    localedir=libcalamares.utils.gettext_path(),
    languages=libcalamares.utils.gettext_languages(),
    fallback=True,
).gettext


def pretty_name():
    return _("Initialize archlinux keys.")


def run():
    """Initialize the keyring before installing any package"""
    is_online__install = libcalamares.globalstorage.value("isOnlineInstall")
    has_internet = libcalamares.globalstorage.value("hasInternet")

    # key population doesn't need internet so do it anyway
    pacman_key_init = [
        "pacman-key",
        "--init",
    ]
    pacman_key_populate = [
        "pacman-key",
        "--populate",
        "archlinux",
    ]

    check_target_env_call(pacman_key_init)
    check_target_env_call(pacman_key_populate)

    # if online install mode selected sync pacman and
    # update if internet is avaialable
    if has_internet:
        pacman_update = [
            "pacman",
            "-Syy",
        ]
        check_target_env_call(pacman_update)
        if is_online__install:
            pacman_arch_keyring = [
                "pacman",
                "-S",
                "--noconfirm",
                "archlinux-keyring",
            ]
            check_target_env_call(pacman_arch_keyring)

    return None
