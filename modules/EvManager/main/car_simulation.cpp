// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#include "car_simulation.hpp"

#include "constants.hpp"

#include <everest/logging.hpp>

constexpr double MS_FACTOR = (1.0 / 60.0 / 60.0 / 1000.0);

void CarSimulation::state_machine() {
    using types::ev_board_support::EvCpState;

    const auto state_has_changed = sim_data.state != sim_data.last_state;
    sim_data.last_state = sim_data.state;

    switch (sim_data.state) {
    case SimState::UNPLUGGED:
        if (state_has_changed) {

            r_ev_board_support->call_set_cp_state(EvCpState::A);
            r_ev_board_support->call_allow_power_on(false);
            // Wait for physical plugin (ev BSP sees state A on CP and not Disconnected)

            sim_data.slac_state = "UNMATCHED";
            if (!r_ev.empty()) {
                r_ev[0]->call_stop_charging();
            }
        }
        break;
    case SimState::PLUGGED_IN:
        if (state_has_changed) {
            r_ev_board_support->call_set_cp_state(EvCpState::B);
            r_ev_board_support->call_allow_power_on(false);
            simulate_soc();
        }
        break;
    case SimState::CHARGING_REGULATED:
        if (state_has_changed || sim_data.pwm_duty_cycle != sim_data.last_pwm_duty_cycle) {
            sim_data.last_pwm_duty_cycle = sim_data.pwm_duty_cycle;
            // do not draw power if EVSE paused by stopping PWM
            if (sim_data.pwm_duty_cycle > 7.0 && sim_data.pwm_duty_cycle < 97.0) {
                r_ev_board_support->call_set_cp_state(EvCpState::C);
                r_ev_board_support->call_allow_power_on(true);
            } else {
                r_ev_board_support->call_set_cp_state(EvCpState::B);
                r_ev_board_support->call_allow_power_on(false);
            }
        }
        break;
    case SimState::CHARGING_FIXED:
        // Todo(sl): What to do here
        if (state_has_changed) {
            // Also draw power if EVSE stopped PWM - this is a break the rules simulator->mode to test the charging
            // implementation!
            r_ev_board_support->call_set_cp_state(EvCpState::C);
            r_ev_board_support->call_allow_power_on(true);
        }
        break;

    case SimState::ERROR_E:
        if (state_has_changed) {
            r_ev_board_support->call_set_cp_state(EvCpState::E);
            r_ev_board_support->call_allow_power_on(false);
        }
        break;
    case SimState::DIODE_FAIL:
        if (state_has_changed) {
            r_ev_board_support->call_diode_fail(true);
            r_ev_board_support->call_allow_power_on(false);
        }
        break;
    case SimState::ISO_POWER_READY:
        if (state_has_changed) {
            r_ev_board_support->call_set_cp_state(EvCpState::C);
        }
        break;
    case SimState::ISO_CHARGING_REGULATED:
        if (state_has_changed) {
            r_ev_board_support->call_set_cp_state(EvCpState::C);
            r_ev_board_support->call_allow_power_on(true);
        }
        break;
    case SimState::BCB_TOGGLE:
        if (sim_data.bcb_toggle_C) {
            r_ev_board_support->call_set_cp_state(EvCpState::C);
            sim_data.bcb_toggle_C = false;
        } else {
            r_ev_board_support->call_set_cp_state(EvCpState::B);
            sim_data.bcb_toggle_C = true;
            ++sim_data.bcb_toggles;
        }
        break;
    default:
        sim_data.state = SimState::UNPLUGGED;
        break;
    }

    if (not state_has_changed and
        (sim_data.state == SimState::CHARGING_REGULATED or sim_data.state == SimState::CHARGING_FIXED or
         sim_data.state == SimState::ISO_CHARGING_REGULATED)) {
        simulate_soc();
    }
    timepoint_last_update = std::chrono::steady_clock::now();
};

void CarSimulation::simulate_soc() {
    const double ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timepoint_last_update)
            .count();
    const double factor = MS_FACTOR * ms;
    double power = 0.0;
    types::evse_manager::EVInfo ev_info;
    switch (charge_mode) {
    case ChargeMode::None:
        // nothing to do
        break;
    case ChargeMode::AC:
        power = charge_current_a * config.ac_nominal_voltage;
        ev_info.target_current = charge_current_a;
        ev_info.target_voltage = 0;
        break;
    case ChargeMode::ACThreePhase:
        power = charge_current_a * config.ac_nominal_voltage * 3.0;
        ev_info.target_current = charge_current_a;
        ev_info.target_voltage = 0;
        break;
    case ChargeMode::DC:
        power = config.dc_target_current * config.dc_target_voltage;
        ev_info.target_current = config.dc_target_current;
        ev_info.target_voltage = config.dc_target_voltage;
        break;
    }

    if (sim_data.battery_charge_wh > sim_data.battery_capacity_wh) {
        sim_data.battery_charge_wh = sim_data.battery_capacity_wh;
    } else {
        sim_data.battery_charge_wh += power * factor;
    }

    ev_info.soc = (sim_data.battery_charge_wh / sim_data.battery_capacity_wh) * 100.0;
    if (ev_info.soc > 100.0) {
        ev_info.soc = 100.0;
    }
    ev_info.battery_capacity = sim_data.battery_capacity_wh;
    ev_info.battery_full_soc = 100;

    p_ev_manager->publish_ev_info(ev_info);
}

bool CarSimulation::sleep(const CmdArguments& arguments, size_t loop_interval_ms) {
    if (not sim_data.sleep_ticks_left.has_value()) {
        const auto sleep_time = std::stold(arguments[0]);
        const auto sleep_time_ms = sleep_time * 1000;
        sim_data.sleep_ticks_left = static_cast<long long>(sleep_time_ms / loop_interval_ms) + 1;
    }
    auto& sleep_ticks_left = sim_data.sleep_ticks_left.value();
    sleep_ticks_left -= 1;
    if (not(sleep_ticks_left > 0)) {
        sim_data.sleep_ticks_left.reset();
        return true;
    } else {
        return false;
    }
}

bool CarSimulation::iec_wait_pwr_ready(const CmdArguments& arguments) {
    sim_data.state = SimState::PLUGGED_IN;
    return (sim_data.pwm_duty_cycle > 7.0f && sim_data.pwm_duty_cycle < 97.0f);
}

bool CarSimulation::iso_wait_pwm_is_running(const CmdArguments& arguments) {
    sim_data.state = SimState::PLUGGED_IN;
    return (sim_data.pwm_duty_cycle > 4.0f && sim_data.pwm_duty_cycle < 97.0f);
}

bool CarSimulation::draw_power_regulated(const CmdArguments& arguments) {
    charge_current_a = std::stod(arguments[0]);
    r_ev_board_support->call_set_ac_max_current(charge_current_a);
    if (arguments[1] == constants::THREE_PHASES) {
        r_ev_board_support->call_set_three_phases(true);
        charge_mode = ChargeMode::ACThreePhase;
    } else {
        r_ev_board_support->call_set_three_phases(false);
        charge_mode = ChargeMode::AC;
    }
    sim_data.state = SimState::CHARGING_REGULATED;
    return true;
}

bool CarSimulation::draw_power_fixed(const CmdArguments& arguments) {
    charge_current_a = std::stod(arguments[0]);
    r_ev_board_support->call_set_ac_max_current(charge_current_a);
    if (arguments[1] == constants::THREE_PHASES) {
        r_ev_board_support->call_set_three_phases(true);
        charge_mode = ChargeMode::ACThreePhase;
    } else {
        r_ev_board_support->call_set_three_phases(false);
        charge_mode = ChargeMode::AC;
    }
    sim_data.state = SimState::CHARGING_FIXED;
    return true;
}

bool CarSimulation::pause(const CmdArguments& arguments) {
    sim_data.state = SimState::PLUGGED_IN;
    return true;
}

bool CarSimulation::unplug(const CmdArguments& arguments) {
    sim_data.state = SimState::UNPLUGGED;
    charge_mode = ChargeMode::None;
    return true;
}

bool CarSimulation::error_e(const CmdArguments& arguments) {
    sim_data.state = SimState::ERROR_E;
    return true;
}

bool CarSimulation::diode_fail(const CmdArguments& arguments) {
    sim_data.state = SimState::DIODE_FAIL;
    return true;
}

bool CarSimulation::rcd_current(const CmdArguments& arguments) {
    sim_data.rcd_current_ma = std::stof(arguments[0]);
    r_ev_board_support->call_set_rcd_error(sim_data.rcd_current_ma);
    return true;
}

bool CarSimulation::iso_wait_slac_matched(const CmdArguments& arguments) {
    sim_data.state = SimState::PLUGGED_IN;

    if (sim_data.slac_state == "UNMATCHED") {
        EVLOG_debug << "Slac UNMATCHED";
        if (!r_slac.empty()) {
            EVLOG_debug << "Slac trigger matching";
            r_slac[0]->call_reset();
            r_slac[0]->call_trigger_matching();
            sim_data.slac_state = "TRIGGERED";
        }
    }
    if (sim_data.slac_state == "MATCHED") {
        EVLOG_debug << "Slac Matched";
        return true;
    }
    return false;
}

bool CarSimulation::iso_wait_pwr_ready(const CmdArguments& arguments) {
    if (sim_data.iso_pwr_ready) {
        sim_data.state = SimState::ISO_POWER_READY;
        return true;
    }
    return false;
}

bool CarSimulation::iso_dc_power_on(const CmdArguments& arguments) {
    sim_data.state = SimState::ISO_POWER_READY;
    if (sim_data.dc_power_on) {
        sim_data.state = SimState::ISO_CHARGING_REGULATED;
        r_ev_board_support->call_allow_power_on(true);
        charge_mode = ChargeMode::DC;
        return true;
    }

    if (sim_data.iso_charger_paused) {

        const auto cmds =
            std::array<std::string, 2>{"pause;iso_wait_v2g_session_stopped;sleep 2;iso_wait_pwm_is_running;",
                                       "iso_wait_pwr_ready;iso_wait_for_stop 36000"};

        EVLOG_info << "Charger wants to pause the session";
        r_ev_board_support->call_allow_power_on(false);

        // NOTE(sl): Change when the Energymode has more then 2 values
        const std::string energy_mode = (sim_data.energy_mode == EnergyMode::AC) ? "AC" : "DC";
        const std::string iso_start_v2g_session = "iso_start_v2g_session " + energy_mode + ";";

        auto& modify_session_cmds = sim_data.modify_charging_session_cmds.emplace();

        modify_session_cmds = cmds[0];
        modify_session_cmds += iso_start_v2g_session;
        modify_session_cmds += cmds[1];

        sim_data.iso_pwr_ready = false;
        sim_data.sleep_ticks_left.reset();
        sim_data.iso_charger_paused = false;

        // NOTE(sl): return false, otherwise the simulation will end too early before the session cmds can be adjusted
    }

    return false;
}

bool CarSimulation::iso_start_v2g_session(const CmdArguments& arguments, bool three_phases) {
    const auto& energy_mode = arguments[0];

    if (energy_mode == constants::AC) {
        sim_data.energy_mode = EnergyMode::AC;
        if (three_phases == false) {
            r_ev[0]->call_start_charging(types::iso15118::EnergyTransferMode::AC_single_phase_core);
            charge_mode = ChargeMode::AC;
        } else {
            r_ev[0]->call_start_charging(types::iso15118::EnergyTransferMode::AC_three_phase_core);
            charge_mode = ChargeMode::ACThreePhase;
        }
    } else if (energy_mode == constants::DC) {
        r_ev[0]->call_start_charging(types::iso15118::EnergyTransferMode::DC_extended);
        sim_data.energy_mode = EnergyMode::DC;
        charge_mode = ChargeMode::DC;
    } else {
        return false;
    }
    return true;
}

bool CarSimulation::iso_draw_power_regulated(const CmdArguments& arguments) {
    charge_current_a = std::stod(arguments[0]);
    r_ev_board_support->call_set_ac_max_current(charge_current_a);
    if (arguments[1] == constants::THREE_PHASES) {
        r_ev_board_support->call_set_three_phases(true);
        charge_mode = ChargeMode::ACThreePhase;
    } else {
        r_ev_board_support->call_set_three_phases(false);
        charge_mode = ChargeMode::AC;
    }
    sim_data.state = SimState::ISO_CHARGING_REGULATED;
    return true;
}

bool CarSimulation::iso_stop_charging(const CmdArguments& arguments) {
    r_ev[0]->call_stop_charging();
    r_ev_board_support->call_allow_power_on(false);
    sim_data.state = SimState::PLUGGED_IN;
    charge_mode = ChargeMode::None;
    return true;
}

bool CarSimulation::iso_wait_for_stop(const CmdArguments& arguments, size_t loop_interval_ms) {
    if (not sim_data.sleep_ticks_left.has_value()) {
        const auto sleep_time_ms = std::stold(arguments[0]) * 1000;
        sim_data.sleep_ticks_left = static_cast<long long>(sleep_time_ms / loop_interval_ms) + 1;
    }
    auto& sleep_ticks_left = sim_data.sleep_ticks_left.value();
    sleep_ticks_left -= 1;
    if (not(sleep_ticks_left > 0)) {
        r_ev[0]->call_stop_charging();
        r_ev_board_support->call_allow_power_on(false);
        sim_data.state = SimState::PLUGGED_IN;
        sim_data.sleep_ticks_left.reset();
        return true;
    }
    if (sim_data.iso_stopped) {
        EVLOG_info << "POWER OFF iso stopped";
        r_ev_board_support->call_allow_power_on(false);
        sim_data.state = SimState::PLUGGED_IN;
        sim_data.sleep_ticks_left.reset();
        return true;
    }

    if (sim_data.iso_charger_paused) {

        const auto cmds =
            std::array<std::string, 2>{"pause;iso_wait_v2g_session_stopped;sleep 2;iso_wait_pwm_is_running;",
                                       "iso_wait_pwr_ready;iso_wait_for_stop 36000"};

        EVLOG_info << "Charger wants to pause the session";
        r_ev_board_support->call_allow_power_on(false);

        // NOTE(sl): Change when the Energymode has more then 2 values
        const std::string energy_mode = (sim_data.energy_mode == EnergyMode::AC) ? "AC" : "DC";
        const std::string iso_start_v2g_session = "iso_start_v2g_session " + energy_mode + ";";

        auto& modify_session_cmds = sim_data.modify_charging_session_cmds.emplace();

        modify_session_cmds = cmds[0];
        modify_session_cmds += iso_start_v2g_session;
        modify_session_cmds += cmds[1];

        sim_data.iso_pwr_ready = false;
        sim_data.sleep_ticks_left.reset();
        sim_data.iso_charger_paused = false;

        // NOTE(sl): return false, otherwise the simulation will end too early before the session cmds can be adjusted
        return false;
    }
    return false;
}

bool CarSimulation::iso_wait_v2g_session_stopped(const CmdArguments& arguments) {
    if (sim_data.v2g_finished) {
        sim_data.v2g_finished = false;
        return true;
    }
    return false;
}

bool CarSimulation::iso_pause_charging(const CmdArguments& arguments) {
    r_ev[0]->call_pause_charging();
    sim_data.state = SimState::PLUGGED_IN;
    sim_data.iso_pwr_ready = false;
    return true;
}

bool CarSimulation::iso_wait_for_resume(const CmdArguments& arguments) {
    return false;
}

bool CarSimulation::iso_start_bcb_toggle(const CmdArguments& arguments) {
    sim_data.v2g_finished = false;
    sim_data.state = SimState::BCB_TOGGLE;
    if (sim_data.bcb_toggles >= std::stoul(arguments[0]) || sim_data.bcb_toggles == 3) {
        sim_data.bcb_toggles = 0;
        sim_data.state = SimState::PLUGGED_IN;
        return true;
    }
    return false;
}

bool CarSimulation::wait_for_real_plugin(const CmdArguments& arguments) {
    using types::board_support_common::Event;
    if (sim_data.actual_bsp_event == Event::A) {
        EVLOG_info << "Real plugin detected";
        sim_data.state = SimState::PLUGGED_IN;
        return true;
    }
    return false;
}
