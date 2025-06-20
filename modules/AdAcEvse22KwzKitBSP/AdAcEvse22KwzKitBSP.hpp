// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
// Portions (c) 2025 Analog Devices Inc.
#ifndef AD_ACEVSE22KWZ_KIT_HPP
#define AD_ACEVSE22KWZ_KIT_HPP

//
// AUTO GENERATED - MARKED REGIONS WILL BE KEPT
// template version 2
//

#include "ld-ev.hpp"

// headers for provided interface implementations
#include <generated/interfaces/evse_board_support/Implementation.hpp>
#include <generated/interfaces/powermeter/Implementation.hpp>

// ev@4bf81b14-a215-475c-a1d3-0a484ae48918:v1
#include "adkit_comms/evSerial.hpp"
// ev@4bf81b14-a215-475c-a1d3-0a484ae48918:v1

namespace module {

struct Conf {
    std::string serial_port;
    int baud_rate;
    std::string reset_gpio_chip;
    int reset_gpio;
    int caps_min_current_A;
    int caps_max_current_A;
};

class AdAcEvse22KwzKitBSP : public Everest::ModuleBase {
public:
    AdAcEvse22KwzKitBSP() = delete;
    AdAcEvse22KwzKitBSP(const ModuleInfo& info, Everest::TelemetryProvider& telemetry,
                        std::unique_ptr<powermeterImplBase> p_powermeter,
                        std::unique_ptr<evse_board_supportImplBase> p_board_support, Conf& config) :
        ModuleBase(info),
        telemetry(telemetry),
        p_powermeter(std::move(p_powermeter)),
        p_board_support(std::move(p_board_support)),
        config(config){};

    Everest::TelemetryProvider& telemetry;
    const std::unique_ptr<powermeterImplBase> p_powermeter;
    const std::unique_ptr<evse_board_supportImplBase> p_board_support;
    const Conf& config;

    // ev@1fce4c5e-0ab8-41bb-90f7-14277703d2ac:v1
    void publish_external_telemetry_livedata(const std::string& topic, const Everest::TelemetryMap& data);
    evSerial serial;
    void clear_errors_on_unplug();
    // ev@1fce4c5e-0ab8-41bb-90f7-14277703d2ac:v1

protected:
    // ev@4714b2ab-a24f-4b95-ab81-36439e1478de:v1
    // insert your protected definitions here
    // ev@4714b2ab-a24f-4b95-ab81-36439e1478de:v1

private:
    friend class LdEverest;
    void init();
    void ready();

    // ev@211cfdbe-f69a-4cd6-a4ec-f8aaa3d1b6c8:v1
    // insert your private definitions here
    Everest::TelemetryMap telemetry_power_path_controller_version;
    Everest::TelemetryMap telemetry_power_path_controller;
    Everest::TelemetryMap telemetry_power_switch;
    Everest::TelemetryMap telemetry_rcd;
    std::mutex telemetry_mutex;
    Everest::Thread telemetryThreadHandle;
    void error_handling(ErrorFlags e);
    ErrorFlags last_error_flags;

    std::atomic_bool error_MREC2GroundFailure{false};
    // ev@211cfdbe-f69a-4cd6-a4ec-f8aaa3d1b6c8:v1
};

// ev@087e516b-124c-48df-94fb-109508c7cda9:v1
Everest::json power_meter_data_to_json(const PowerMeter& p);
Everest::json keep_alive_lo_to_json(const KeepAliveLo& k);
std::string error_type_to_string(ErrorFlags s);
// ev@087e516b-124c-48df-94fb-109508c7cda9:v1

} // namespace module

#endif // AD_ACEVSE22KWZ_KIT_HPP
