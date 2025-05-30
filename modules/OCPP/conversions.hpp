// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#ifndef OCPP_V16_CONVERSIONS_HPP
#define OCPP_V16_CONVERSIONS_HPP

#include <generated/types/display_message.hpp>
#include <generated/types/evse_manager.hpp>
#include <generated/types/iso15118.hpp>
#include <generated/types/ocpp.hpp>
#include <generated/types/reservation.hpp>
#include <generated/types/session_cost.hpp>
#include <generated/types/system.hpp>

#include <ocpp/common/types.hpp>
#include <ocpp/v16/messages/BootNotification.hpp>
#include <ocpp/v16/messages/DataTransfer.hpp>
#include <ocpp/v16/messages/GetDiagnostics.hpp>
#include <ocpp/v16/messages/LogStatusNotification.hpp>
#include <ocpp/v16/messages/ReserveNow.hpp>
#include <ocpp/v16/messages/StopTransaction.hpp>
#include <ocpp/v16/types.hpp>
#include <ocpp/v2/messages/DeleteCertificate.hpp>
#include <ocpp/v2/messages/Get15118EVCertificate.hpp>

namespace module {
namespace conversions {

/// \brief Converts a given types::system::FirmwareUpdateStatusEnum \p status to a ocpp::FirmwareStatusNotification.
ocpp::FirmwareStatusNotification
to_ocpp_firmware_status_notification(const types::system::FirmwareUpdateStatusEnum status);

/// \brief Converts a given types::evse_manager::StartSessionReason \p reason to a ocpp::SessionStartedReason.
ocpp::SessionStartedReason to_ocpp_session_started_reason(const types::evse_manager::StartSessionReason reason);

/// \brief Converts a given types::ocpp::DataTransferStatus \p status to a ocpp::v16::DataTransferStatus.
ocpp::v16::DataTransferStatus to_ocpp_data_transfer_status(const types::ocpp::DataTransferStatus status);

/// \brief Converts a given types::evse_manager::StopTransactionReason \p reason to a ocpp::v16::Reason.
ocpp::v16::Reason to_ocpp_reason(const types::evse_manager::StopTransactionReason reason);

/// \brief Converts a given types::iso15118::CertificateActionEnum \p action to a
/// ocpp::v2::CertificateActionEnum.
ocpp::v2::CertificateActionEnum to_ocpp_certificate_action_enum(const types::iso15118::CertificateActionEnum action);

/// \brief Converts a given types::reservation::ReservationResult \p result to a ocpp::v16::ReservationStatus.
ocpp::v16::ReservationStatus to_ocpp_reservation_status(const types::reservation::ReservationResult result);

/// \brief Converts a given types::system::UploadLogsStatus \p status to a ocpp::v16::LogStatusEnumType.
ocpp::v16::LogStatusEnumType to_ocpp_log_status_enum_type(const types::system::UploadLogsStatus status);

/// \brief Converts a given types::system::UpdateFirmwareResponse \p response to a
/// ocpp::v16::UpdateFirmwareStatusEnumType.
ocpp::v16::UpdateFirmwareStatusEnumType
to_ocpp_update_firmware_status_enum_type(const types::system::UpdateFirmwareResponse response);

/// \brief Converts a given types::iso15118::HashAlgorithm \p hash_algorithm to a
/// ocpp::v16::HashAlgorithmEnumType.
ocpp::v16::HashAlgorithmEnumType to_ocpp_hash_algorithm_enum_type(const types::iso15118::HashAlgorithm hash_algorithm);

/// \brief  Converts a given types::iso15118::Status \p status to a ocpp::v2::Iso15118EVCertificateStatusEnum.
ocpp::v16::BootReasonEnum to_ocpp_boot_reason_enum(const types::system::BootReason reason);

/// \brief  Converts a given types::powermeter::Powermeter \p powermeter to a ocpp::Powermeter
ocpp::Powermeter to_ocpp_power_meter(const types::powermeter::Powermeter& powermeter);

/// \brief Converts a given vector of types::temperature::Temperature \p powermeter to a vector of ocpp::Temperature
std::vector<ocpp::Temperature> to_ocpp_temperatures(const std::vector<types::temperature::Temperature>& temperatures);

/// \brief Converts a given types::iso15118::HashAlgorithm \p hash_algorithm to a ocpp::v2::HashAlgorithmEnum.
ocpp::v2::HashAlgorithmEnum to_ocpp_hash_algorithm_enum(const types::iso15118::HashAlgorithm hash_algorithm);

/// \brief Converts a given ocpp::v16::Reason \p reason to a types::evse_manager::StopTransactionReason.
types::evse_manager::StopTransactionReason to_everest_stop_transaction_reason(const ocpp::v16::Reason reason);

/// \brief Converts a given ocpp::v16::ResetType \p type to a types::system::ResetType.
types::system::ResetType to_everest_reset_type(const ocpp::v16::ResetType type);

/// \brief Converts a given ocpp::v2::Iso15118EVCertificateStatusEnum \p status to a types::iso15118::Status.
types::iso15118::Status to_everest_iso15118_status(const ocpp::v2::Iso15118EVCertificateStatusEnum status);

/// \brief Converts a given ocpp::v2::CertificateActionEnum \p action to a
/// types::iso15118::CertificateActionEnum.
types::iso15118::CertificateActionEnum to_everest_certificate_action_enum(const ocpp::v2::CertificateActionEnum action);

/// \brief Converts a given ocpp::v2::AuthorizeCertificateStatusEnum \p status to a
/// types::authorization::CertificateStatus.
types::authorization::CertificateStatus
to_everest_certificate_status(const ocpp::v2::AuthorizeCertificateStatusEnum status);

/// \brief Converts a given ocpp::v16::AuthorizationStatus \p status to a types::authorization::AuthorizationStatus.
types::authorization::AuthorizationStatus to_everest_authorization_status(const ocpp::v16::AuthorizationStatus status);

/// \brief Converts a given ocpp::v2::AuthorizationStatusEnum \p status to a
/// types::authorization::AuthorizationStatus.
types::authorization::AuthorizationStatus
to_everest_authorization_status(const ocpp::v2::AuthorizationStatusEnum status);

/// \brief Convert ocpp::v16::EnhancedChargingSchedulePeriod to types::ocpp::ChargingSchedulePeriod
types::ocpp::ChargingSchedulePeriod
to_charging_schedule_period(const ocpp::v16::EnhancedChargingSchedulePeriod& period);

/// \brief Convert ocpp::v16::EnhancedChargingSchedule to types::ocpp::ChargingSchedule
types::ocpp::ChargingSchedule to_charging_schedule(const ocpp::v16::EnhancedChargingSchedule& schedule);

/// \brief Converts a given ocpp::v16::BootNotificationResponse \p boot_notification_response to a
/// types::ocpp::BootNotificationResponse
types::ocpp::BootNotificationResponse
to_everest_boot_notification_response(const ocpp::v16::BootNotificationResponse& boot_notification_response);

/// \brief Converts a given ocpp::v16::RegistrationStatus \p registration_status to a
/// types::ocpp::RegistrationStatus
types::ocpp::RegistrationStatus
to_everest_registration_status(const ocpp::v16::RegistrationStatus& registration_status);

ocpp::v16::DataTransferResponse
to_ocpp_data_transfer_response(const types::display_message::SetDisplayMessageResponse& set_display_message_response);

} // namespace conversions
} // namespace module

#endif // OCPP_V16_CONVERSIONS_HPP
