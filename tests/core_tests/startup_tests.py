
#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright 2020 - 2022 Pionix GmbH and Contributors to EVerest

import logging
import os
import pytest
import time
import threading
import queue

from enum import Enum

from everest.testing.core_utils.fixtures import *
from everest.testing.core_utils.everest_core import EverestCore, Requirement

from everest.framework import Module, RuntimeSession


@pytest.mark.asyncio
async def test_000_startup_check(everest_core: EverestCore):
    logging.info(">>>>>>>>> test_000_startup_check <<<<<<<<<")
    everest_core.start()

class Mode(Enum):
    Basic = 0
    HlcAc = 1
    HlcDc = 2

class ProbeModule:
    def __init__(self, session: RuntimeSession):
        m = Module('probe', session)
        self._setup = m.say_hello()

        # subscribe to session events
        evse_manager_ff = self._setup.connections['connector_1'][0]
        m.subscribe_variable(evse_manager_ff, 'session_event',
                             self._handle_evse_manager_event)

        self._msg_queue = queue.Queue()
        self._energy_wh_import = 0

        self._ready_event = threading.Event()
        self._mod = m
        m.init_done(self._ready)

    def _ready(self):
        self._ready_event.set()

    def _handle_evse_manager_event(self, args):
        self._msg_queue.put(args['event'])
        if 'transaction_finished' in args:
            self._energy_wh_import = args['transaction_finished']['meter_value']['energy_Wh_import']['total']

    def test(self, timeout: float, mode: Mode) -> bool:
        end_of_time = time.time() + timeout

        logging.info("Wating for ready event...")
        if not self._ready_event.wait(timeout):
            logging.error("Ready event not received. Timeout.")
            return False

        # fetch fulfillment
        car_sim_ff = self._setup.connections['test_control'][0]

        # enable simulator
        self._mod.call_command(car_sim_ff, 'enable', {'value': True})

        cmd = {}

        if mode == Mode.HlcAc:
            cmd = {'value': 'sleep 1;iso_wait_slac_matched;iso_start_v2g_session AC;iso_wait_pwr_ready;iso_draw_power_regulated 16,3;iso_wait_for_stop 20;iso_wait_v2g_session_stopped;unplug'}
        elif mode == Mode.HlcDc:
            cmd = {'value': 'sleep 1;iso_wait_slac_matched;iso_start_v2g_session DC;iso_wait_pwr_ready;iso_wait_for_stop 20;iso_wait_v2g_session_stopped;unplug'}
        else:
            cmd = {'value': 'sleep 1;iec_wait_pwr_ready;sleep 1;draw_power_regulated 16,3;sleep 10;unplug'}

        # start charging simulation
        self._mod.call_command(car_sim_ff, 'execute_charging_session', cmd)

        expected_events = ['TransactionStarted', 'TransactionFinished']

        logging.info("Waiting for expected events...")
        while len(expected_events) > 0:
            time_left = end_of_time - time.time()

            if time_left < 0:
                logging.error("Timeout waiting for expected events")
                return False

            try:
                logging.info("Waiting for event: %s", expected_events[0])
                event = self._msg_queue.get(timeout=time_left)
                logging.info("Received event: %s", event)
                if expected_events[0] == event:
                    logging.info("Event was expected, removing from list.")
                    expected_events.pop(0)
            except queue.Empty:
                return False
        logging.info("Total energy import: %f Wh", self._energy_wh_import)
        if self._energy_wh_import <= 0:
            return False
        return True


@pytest.mark.everest_core_config('config-sil.yaml')
@pytest.mark.asyncio
async def test_001_start_test_module(everest_core: EverestCore):
    logging.info(">>>>>>>>> test_001_start_test_module <<<<<<<<<")

    test_connections = {
        'test_control': [Requirement('ev_manager', 'main')],
        'connector_1': [Requirement('connector_1', 'evse')]
    }

    everest_core.start(standalone_module='probe', test_connections=test_connections)
    logging.info("everest-core ready, waiting for probe module")

    session = RuntimeSession(str(everest_core.prefix_path), str(everest_core.everest_config_path))

    probe = ProbeModule(session)

    if everest_core.status_listener.wait_for_status(10, ["ALL_MODULES_STARTED"]):
        everest_core.all_modules_started_event.set()
        logging.info("set all modules started event...")

    assert probe.test(20, Mode.Basic)

@pytest.mark.everest_core_config('config-sil.yaml')
@pytest.mark.asyncio
async def test_002_start_test_module_ac_hlc(everest_core: EverestCore):
    logging.info(">>>>>>>>> test_002_start_test_module_ac_hlc <<<<<<<<<")

    test_connections = {
        'test_control': [Requirement('ev_manager', 'main')],
        'connector_1': [Requirement('connector_1', 'evse')]
    }

    everest_core.start(standalone_module='probe', test_connections=test_connections)
    logging.info("everest-core ready, waiting for probe module")

    session = RuntimeSession(str(everest_core.prefix_path), str(everest_core.everest_config_path))

    probe = ProbeModule(session)

    if everest_core.status_listener.wait_for_status(10, ["ALL_MODULES_STARTED"]):
        everest_core.all_modules_started_event.set()
        logging.info("set all modules started event...")

    assert probe.test(90, Mode.HlcAc)

@pytest.mark.everest_core_config('config-sil-dc.yaml')
@pytest.mark.asyncio
async def test_003_start_test_module_dc(everest_core: EverestCore):
    logging.info(">>>>>>>>> test_003_start_test_module_dc <<<<<<<<<")

    test_connections = {
        'test_control': [Requirement('ev_manager', 'main')],
        'connector_1': [Requirement('evse_manager', 'evse')]
    }

    everest_core.start(standalone_module='probe', test_connections=test_connections)
    logging.info("everest-core ready, waiting for probe module")

    session = RuntimeSession(str(everest_core.prefix_path), str(everest_core.everest_config_path))

    probe = ProbeModule(session)

    if everest_core.status_listener.wait_for_status(10, ["ALL_MODULES_STARTED"]):
        everest_core.all_modules_started_event.set()
        logging.info("set all modules started event...")

    assert probe.test(90, Mode.HlcDc)
