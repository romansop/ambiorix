#!/usr/bin/env pytest3
""" Conftest for testing python amx bindings.
"""
import os
import subprocess
import time
import glob
import pytest
import pamx

SCRIPTDIR = os.path.dirname(os.path.realpath(__file__))


# pylint: disable=redefined-outer-name
# pylint: disable=unused-argument
# pylint: disable=consider-using-with
@pytest.fixture(scope="session")
def bus(scope="session"):
    """Fixture to manage bus processes"""
    subprocess.Popen(['sudo',
                      'ubusd',
                      '&'])
    subprocess.Popen(['sudo',
                      'pcb_sysbus',
                      '-n', 'bus',
                      '-I', '/tmp/local'])
    time.sleep(1)
    yield
    os.system("sudo kill -9 $(pgrep bus)")


# pylint: disable=redefined-outer-name
# pylint: disable=unused-argument
# pylint: disable=consider-using-with
@pytest.fixture(scope="session")
def test_models(bus):
    """Fixture to set up test data models"""
    phonebook = subprocess.Popen(['amxrt',
                                  f'{SCRIPTDIR}/model/phonebook.odl'])
    conversion = subprocess.Popen(['amxrt',
                                   f'{SCRIPTDIR}/model/conversion.odl'])
    time.sleep(1)
    yield
    phonebook.terminate()
    conversion.terminate()


# pylint: disable=redefined-outer-name
# pylint: disable=unused-argument
@pytest.fixture(scope="session", autouse=True)
def connection(test_models):
    """Fixture to establish connection to the pcb bus with url."""
    pcb_backends = glob.glob("/usr/bin/mods/amxb/*-mod-amxb-pcb.so")
    if not pcb_backends:
        def_path = "/usr/bin/mods/amxb/mod-amxb-pcb.so"
        pamx.backend.load(def_path)
    for backend in pcb_backends:
        pamx.backend.load(backend)

    connection = pamx.bus.connect("pcb:/tmp/local")
    connection.access_level = "protected"
    yield connection
