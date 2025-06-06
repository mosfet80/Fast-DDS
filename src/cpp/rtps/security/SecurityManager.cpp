// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file SecurityManager.h
 */

#include <rtps/security/SecurityManager.h>

#include <cassert>
#include <chrono>
#include <mutex>
#include <thread>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/EndpointSecurityAttributes.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipantListener.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/discovery/endpoint/EDP.h>
#include <rtps/builtin/discovery/participant/PDP.h>
#include <rtps/messages/CDRMessage.hpp>
#include <rtps/history/TopicPayloadPoolRegistry.hpp>
#include <rtps/network/NetworkFactory.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/reader/StatelessReader.hpp>
#include <rtps/security/accesscontrol/AccessControl.h>
#include <rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#include <rtps/security/accesscontrol/SecurityMaskUtilities.h>
#include <rtps/security/authentication/Authentication.h>
#include <rtps/security/exceptions/SecurityException.h>
#include <rtps/writer/StatefulWriter.hpp>
#include <rtps/writer/StatelessWriter.hpp>
#include <security/OpenSSLInit.hpp>

#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_WRITER (1 << 20)
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_READER (1 << 21)
#define BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_WRITER (1 << 22)
#define BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_READER (1 << 23)
#define BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_WRITER (1 << 24)
#define BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_READER (1 << 25)

#define AUTHENTICATION_PARTICIPANT_STATELESS_MESSAGE "dds.sec.auth"
#define GMCLASSID_SECURITY_PARTICIPANT_CRYPTO_TOKENS "dds.sec.participant_crypto_tokens"
#define GMCLASSID_SECURITY_READER_CRYPTO_TOKENS "dds.sec.datareader_crypto_tokens"
#define GMCLASSID_SECURITY_WRITER_CRYPTO_TOKENS "dds.sec.datawriter_crypto_tokens"

// TODO(Ricardo) Add event because stateless messages can be not received.

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace security;

inline bool usleep_bool()
{
    std::this_thread::sleep_for (std::chrono::milliseconds(100));
    return true;
}

static CacheChange_t* create_change_for_message(
        const ParticipantGenericMessage& message,
        WriterHistory* history)
{
    uint32_t cdr_size = static_cast<uint32_t>(ParticipantGenericMessageHelper::serialized_size(message));
    cdr_size += (4 - (cdr_size % 4)) & (4 - 1); // Align to 4 bytes
    cdr_size += 4; // Encapsulation
    return history->create_change(cdr_size, ALIVE, c_InstanceHandle_Unknown);
}

SecurityManager::SecurityManager(
        RTPSParticipantImpl* participant,
        ISecurityPluginFactory& plugin_factory)
    : participant_stateless_message_listener_(*this)
    , participant_volatile_message_secure_listener_(*this)
    , participant_(participant)
    , factory_(plugin_factory)
    , domain_id_(0)
    , auth_last_sequence_number_(1)
    , crypto_last_sequence_number_(1)
    , temp_reader_proxies_({
                participant->get_attributes().allocation.locators.max_unicast_locators,
                participant->get_attributes().allocation.locators.max_multicast_locators,
                participant->get_attributes().allocation.data_limits,
                participant->get_attributes().allocation.content_filter})
    , temp_writer_proxies_({
                participant->get_attributes().allocation.locators.max_unicast_locators,
                participant->get_attributes().allocation.locators.max_multicast_locators,
                participant->get_attributes().allocation.data_limits})
{
    assert(participant != nullptr);
}

SecurityManager::~SecurityManager()
{
    destroy();
}

bool SecurityManager::init(
        ParticipantSecurityAttributes& attributes,
        const PropertyPolicy& participant_properties)
{
    SecurityException exception;
    try
    {
        domain_id_ = participant_->get_domain_id();
        auto part_attributes = participant_->get_attributes();
        const PropertyPolicy log_properties = PropertyPolicyHelper::get_properties_with_prefix(
            part_attributes.properties,
            "dds.sec.log.builtin.DDS_LogTopic.");

        // length(log_properties) == 0 considered as logging disable.
        if (PropertyPolicyHelper::length(log_properties) > 0)
        {
            logging_plugin_ = factory_.create_logging_plugin(participant_properties);

            if (logging_plugin_ != nullptr)
            {
                LogOptions log_options;
                log_options.distribute = false;
                log_options.log_level = LoggingLevel::ERROR_LEVEL;
                log_options.log_file = "";

                const std::string* const distribute = PropertyPolicyHelper::find_property(log_properties, "distribute");
                if (distribute != nullptr)
                {
                    if (!distribute->compare("true") || !distribute->compare("1"))
                    {
                        log_options.distribute = true;
                    }
                    else if (!distribute->compare("false") || !distribute->compare("0"))
                    {
                        log_options.distribute = false;
                    }
                    else
                    {
                        throw SecurityException("Unknown value '" + *distribute + "' for LogOptions::distribute.");
                    }
                }

                const std::string* const log_level =
                        PropertyPolicyHelper::find_property(log_properties, "logging_level");
                if (log_level != nullptr)
                {
                    if (!string_to_LogLevel(*log_level, log_options.log_level, exception))
                    {
                        throw exception;
                    }
                }

                const std::string* const log_file = PropertyPolicyHelper::find_property(log_properties, "log_file");
                if (log_file != nullptr)
                {
                    log_options.log_file = *log_file;
                }

                if (!(logging_plugin_->set_guid(participant_->getGuid(), exception) &&
                        logging_plugin_->set_domain_id(domain_id_, exception)))
                {
                    throw exception;
                }

                if (!( logging_plugin_->set_log_options(log_options, exception) &&
                        logging_plugin_->enable_logging(exception)))
                {
                    throw exception;
                }
            }
            else
            {
                //TODO(artivis): If the factory fails instantiating 'authentication_plugin_',
                // a EPROSIMA_LOG_INFO is issued and this init function returns true. Is it a bug?
                // in the meantime we'll adopt a similar behavior here.
                EPROSIMA_LOG_INFO(SECURITY, "Could not create logging plugin. Security logging will be disabled.");
            }
        }

        const auto log_info_message = [this](const char* msg)
                {
                    if (logging_plugin_)
                    {
                        SecurityException logging_exception;
                        logging_plugin_->log(LoggingLevel::INFORMATIONAL_LEVEL, msg, "SecurityManager,init",
                                logging_exception);
                    }
                    else
                    {
                        EPROSIMA_LOG_INFO(SECURITY, msg);
                    }
                };

        authentication_plugin_ = factory_.create_authentication_plugin(participant_properties);

        if (authentication_plugin_ != nullptr)
        {
            // retrieve authentication properties, if any
            const PropertyPolicy auth_handshake_properties = PropertyPolicyHelper::get_properties_with_prefix(
                part_attributes.properties,
                "dds.sec.auth.builtin.PKI-DH.");

            // if auth_handshake_properties is empty, the default values are used
            if (PropertyPolicyHelper::length(auth_handshake_properties) > 0)
            {
                auth_handshake_props_.parse_from_property_policy(auth_handshake_properties);
            }

            authentication_plugin_->set_logger(logging_plugin_, exception);

            // Validate local participant
            GUID_t adjusted_participant_key;
            ValidationResult_t ret = VALIDATION_FAILED;

            do
            {
                ret = authentication_plugin_->validate_local_identity(&local_identity_handle_,
                                adjusted_participant_key,
                                domain_id_,
                                part_attributes,
                                participant_->getGuid(),
                                exception);
            }
            while (ret == VALIDATION_PENDING_RETRY && usleep_bool());

            if (ret == VALIDATION_OK)
            {
                assert(local_identity_handle_ != nullptr);
                assert(!local_identity_handle_->nil());

                // Set participant guid
                participant_->setGuid(adjusted_participant_key);

                access_plugin_ = factory_.create_access_control_plugin(participant_properties);

                if (access_plugin_ != nullptr)
                {
                    access_plugin_->set_logger(logging_plugin_, exception);

                    local_permissions_handle_ = access_plugin_->validate_local_permissions(
                        *authentication_plugin_, *local_identity_handle_,
                        domain_id_,
                        part_attributes,
                        exception);

                    if (local_permissions_handle_ != nullptr)
                    {
                        if (!local_permissions_handle_->nil())
                        {
                            if (access_plugin_->check_create_participant(*local_permissions_handle_,
                                    domain_id_,
                                    part_attributes, exception))
                            {
                                // Set credentials.
                                PermissionsCredentialToken* token = nullptr;
                                if (access_plugin_->get_permissions_credential_token(
                                            &token, *local_permissions_handle_, exception))
                                {

                                    if (authentication_plugin_->set_permissions_credential_and_token(
                                                *local_identity_handle_, *token, exception))
                                    {
                                        if (!access_plugin_->get_participant_sec_attributes(*local_permissions_handle_,
                                                attributes, exception))
                                        {
                                            access_plugin_->return_permissions_handle(local_permissions_handle_,
                                                    exception);
                                            local_permissions_handle_ = nullptr;
                                        }
                                    }
                                    else
                                    {
                                        access_plugin_->return_permissions_handle(local_permissions_handle_, exception);
                                        local_permissions_handle_ = nullptr;
                                    }

                                    access_plugin_->return_permissions_credential_token(token, exception);
                                }
                                else
                                {
                                    access_plugin_->return_permissions_handle(local_permissions_handle_, exception);
                                    local_permissions_handle_ = nullptr;
                                }
                            }
                            else
                            {
                                access_plugin_->return_permissions_handle(local_permissions_handle_, exception);
                                local_permissions_handle_ = nullptr;
                            }
                        }
                        else
                        {
                            access_plugin_->return_permissions_handle(local_permissions_handle_, exception);
                            local_permissions_handle_ = nullptr;
                        }
                    }
                    else
                    {
                        log_info_message(exception.what());
                    }
                }
                else
                {
                    log_info_message("Access control plugin not configured");
                }

                if (access_plugin_ == nullptr || local_permissions_handle_ != nullptr)
                {
                    crypto_plugin_ = factory_.create_cryptography_plugin(participant_properties);

                    if (crypto_plugin_ != nullptr)
                    {
                        crypto_plugin_->set_logger(logging_plugin_, exception);

                        local_participant_crypto_handle_ =
                                crypto_plugin_->cryptokeyfactory()->register_local_participant(
                            *local_identity_handle_,
                            *local_permissions_handle_,
                            participant_properties.properties(),
                            attributes,
                            exception);

                        if (local_participant_crypto_handle_)
                        {
                            assert(!local_participant_crypto_handle_->nil());
                        }
                        else
                        {
                            log_info_message(exception.what());
                        }
                    }
                    else
                    {
                        log_info_message(exception.what());
                    }
                }
                else
                {
                    log_info_message("Cryptography plugin not configured");
                }

                if ((access_plugin_ == nullptr || local_permissions_handle_ != nullptr) &&
                        (crypto_plugin_ == nullptr || local_participant_crypto_handle_))
                {
                    // Should be activated here, to enable encription buffer on created entities
                    throw true;
                }
                else
                {
                    if (access_plugin_ != nullptr && local_permissions_handle_ == nullptr)
                    {
                        EPROSIMA_LOG_ERROR(SECURITY, "Participant is not allowed with its own permissions file.");
                    }

                    if (crypto_plugin_ != nullptr && local_participant_crypto_handle_ == nullptr)
                    {
                        EPROSIMA_LOG_ERROR(SECURITY, "Participant cryptography could not be configured.");
                    }
                }
            }

            // NOTE: This makes Participant creation fails, in some occasions without any info of what happened.
            // However, it has been decided to leave it this way.
            // For future developers struggling with security debugging issues, remember that the exception variable
            // at this point has relevant information.
            throw false;
        }
        else
        {
            log_info_message("Authentication plugin not configured. Security will be disabled");
        }
    }
    catch (const SecurityException& e)
    {
        EPROSIMA_LOG_ERROR(SECURITY, "Logging plugin not configured. Security logging will be disabled. ("
                << e.what() << ").");
        delete logging_plugin_;
        logging_plugin_ = nullptr;
        return false;
    }
    catch (bool e)
    {
        if (!e)
        {
            // Unexpected code path. Let's log any errors
            EPROSIMA_LOG_ERROR(SECURITY, "Error while configuring security plugin.");
            if (0 != strlen(exception.what()))
            {
                EPROSIMA_LOG_ERROR(SECURITY, exception.what());
            }

            cancel_init();
            return false;
        }
    }

    enable_security_manager();
    return true;
}

void SecurityManager::cancel_init()
{
    SecurityException exception;
    if (local_participant_crypto_handle_)
    {
        crypto_plugin_->cryptokeyfactory()->unregister_participant(local_participant_crypto_handle_, exception);
    }

    if (crypto_plugin_ != nullptr)
    {
        delete crypto_plugin_;
        crypto_plugin_ = nullptr;
    }

    //TODO(Ricardo) Return local_permissions

    if (access_plugin_ != nullptr)
    {
        if (local_permissions_handle_ != nullptr)
        {
            access_plugin_->return_permissions_handle(local_permissions_handle_, exception);
        }

        delete access_plugin_;
        access_plugin_ = nullptr;
    }

    if (authentication_plugin_ != nullptr)
    {
        if (local_identity_handle_ != nullptr)
        {
            authentication_plugin_->return_identity_handle(local_identity_handle_, exception);
        }

        delete authentication_plugin_;
        authentication_plugin_ = nullptr;
    }

    disable_security_manager();
}

void SecurityManager::destroy()
{
    disable_security_manager();

    if (authentication_plugin_ != nullptr)
    {
        shared_lock<shared_mutex> _(mutex_);

        SecurityException exception;

        // writer_handles_ and reader_handles_ contents reference the discovered_participants_ ones and are disposed on
        // this collection removal

        for (auto& dp_it : discovered_participants_)
        {
            std::shared_ptr<ParticipantCryptoHandle> participant_crypto_handle = dp_it.second->get_participant_crypto();
            if (participant_crypto_handle != nullptr)
            {
                crypto_plugin_->cryptokeyfactory()->unregister_participant(participant_crypto_handle, exception);
            }

            PermissionsHandle* permissions_handle = dp_it.second->get_permissions_handle();
            if (permissions_handle != nullptr)
            {
                access_plugin_->return_permissions_handle(permissions_handle, exception);
            }

            std::shared_ptr<SecretHandle> shared_secret_handle = dp_it.second->get_shared_secret();
            if (shared_secret_handle != nullptr)
            {
                authentication_plugin_->return_sharedsecret_handle(shared_secret_handle, exception);
            }

            remove_discovered_participant_info(dp_it.second->get_auth());
        }

        if (local_participant_crypto_handle_)
        {
            crypto_plugin_->cryptokeyfactory()->unregister_participant(local_participant_crypto_handle_, exception);
        }

        if (local_permissions_handle_ != nullptr)
        {
            access_plugin_->return_permissions_handle(local_permissions_handle_, exception);
        }

        if (local_identity_handle_ != nullptr)
        {
            authentication_plugin_->return_identity_handle(local_identity_handle_, exception);
        }

    }

    std::lock_guard<shared_mutex> _(mutex_);

    // check the logic
    delete_entities();

    writer_handles_.clear();
    reader_handles_.clear();
    discovered_participants_.clear();

    local_permissions_handle_ = nullptr;
    local_identity_handle_ = nullptr;

    if (crypto_plugin_ != nullptr)
    {
        delete crypto_plugin_;
        crypto_plugin_ = nullptr;
    }

    if (access_plugin_ != nullptr)
    {
        delete access_plugin_;
        access_plugin_ = nullptr;
    }

    if (authentication_plugin_ != nullptr)
    {
        delete authentication_plugin_;
        authentication_plugin_ = nullptr;
    }

    if (logging_plugin_ != nullptr)
    {
        delete logging_plugin_;
        logging_plugin_ = nullptr;
    }
}

void SecurityManager::remove_discovered_participant_info(
        const DiscoveredParticipantInfo::AuthUniquePtr& auth_ptr)
{
    SecurityException exception;

    if (auth_ptr)
    {
        if (auth_ptr->handshake_handle_ != nullptr)
        {
            authentication_plugin_->return_handshake_handle(auth_ptr->handshake_handle_, exception);
            auth_ptr->handshake_handle_ = nullptr;
        }

        authentication_plugin_->return_identity_handle(auth_ptr->identity_handle_, exception);
        auth_ptr->identity_handle_ = nullptr;

        if (auth_ptr->change_sequence_number_ != SequenceNumber_t::unknown())
        {
            participant_stateless_message_writer_history_->remove_change(auth_ptr->change_sequence_number_);
            auth_ptr->change_sequence_number_ = SequenceNumber_t::unknown();
        }
    }
}

bool SecurityManager::restore_discovered_participant_info(
        const GUID_t& remote_participant_key,
        DiscoveredParticipantInfo::AuthUniquePtr& auth_ptr)
{
    SecurityException exception;
    bool returned_value = false;

    shared_lock<shared_mutex> _(mutex_);

    auto dp_it = discovered_participants_.find(remote_participant_key);

    if (dp_it != discovered_participants_.end())
    {
        dp_it->second->set_auth(auth_ptr);
        returned_value = true;
    }
    else
    {
        remove_discovered_participant_info(std::move(auth_ptr));
    }

    return returned_value;
}

bool SecurityManager::discovered_participant(
        const ParticipantProxyData& participant_data)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    // Early return when ParticipantSecurityInfo does not match
    auto& sec_attrs = participant_->security_attributes();
    if (!sec_attrs.match(participant_data.security_attributes_, participant_data.plugin_security_attributes_))
    {
        return false;
    }

    if (authentication_plugin_ == nullptr)
    {
        participant_->pdp()->notifyAboveRemoteEndpoints(participant_data, true);
        return true;
    }

    SecurityException exception;
    AuthenticationStatus auth_status = AUTHENTICATION_INIT;

    // Create or find information
    bool undiscovered = false;
    DiscoveredParticipantInfo::AuthUniquePtr remote_participant_info;
    // Use the information from the collection
    const ParticipantProxyData* remote_participant_data = nullptr;
    {
        std::lock_guard<shared_mutex> _(mutex_);

        auto map_ret = discovered_participants_.insert(
            std::make_pair(
                participant_data.guid,
                std::unique_ptr<DiscoveredParticipantInfo>(
                    new DiscoveredParticipantInfo(
                        auth_status,
                        participant_data))));

        undiscovered = map_ret.second;
        remote_participant_info = map_ret.first->second->get_auth();
        remote_participant_data = &map_ret.first->second->participant_data();
    }

    bool notify_part_authorized = false;
    if (undiscovered && remote_participant_info && remote_participant_data != nullptr)
    {
        // Configure the timed event but do not start it
        const GUID_t guid = remote_participant_data->guid;
        remote_participant_info->event_.reset(new TimedEvent(participant_->getEventResource(),
                [&, guid]() -> bool
                {
                    resend_handshake_message_token(guid);
                    return true;
                },
                static_cast<double>(auth_handshake_props_.initial_handshake_resend_period_ms_)));

        IdentityHandle* remote_identity_handle = nullptr;

        // Validate remote participant.
        ValidationResult_t validation_ret = authentication_plugin_->validate_remote_identity(&remote_identity_handle,
                        *local_identity_handle_,
                        remote_participant_data->identity_token_,
                        remote_participant_data->guid, exception);

        switch (validation_ret)
        {
            case VALIDATION_OK:
                assert(remote_identity_handle != nullptr);
                auth_status = AUTHENTICATION_OK;
                break;
            case VALIDATION_PENDING_HANDSHAKE_REQUEST:
                assert(remote_identity_handle != nullptr);
                auth_status = AUTHENTICATION_REQUEST_NOT_SEND;
                break;
            case VALIDATION_PENDING_HANDSHAKE_MESSAGE:
                assert(remote_identity_handle != nullptr);
                auth_status = AUTHENTICATION_WAITING_REQUEST;
                break;
            case VALIDATION_PENDING_RETRY:
            // TODO(Ricardo) Send event.
            default:

                on_validation_failed(*remote_participant_data, exception);

                std::lock_guard<shared_mutex> _(mutex_);

                // Remove created element, because authentication failed.
                discovered_participants_.erase(remote_participant_data->guid);

                //TODO(Ricardo) cryptograhy registration in AUTHENTICAITON_OK
                return false;
        }

        EPROSIMA_LOG_INFO(SECURITY, "Discovered participant " << remote_participant_data->guid);

        // Match entities
        match_builtin_endpoints(*remote_participant_data);

        // Store new remote handle.
        remote_participant_info->auth_status_ = auth_status;
        remote_participant_info->identity_handle_ = remote_identity_handle;

        // TODO(Ricardo) Start cryptography if authentication ok in this point.
        // If authentication is successful, inform user about it.
        if (auth_status == AUTHENTICATION_OK)
        {
            //TODO(Ricardo) Shared secret on this case?
            std::shared_ptr<SecretHandle> ss;
            notify_part_authorized = participant_authorized(*remote_participant_data, remote_participant_info, ss);
        }
    }
    else
    {
        // If cannot retrieve the authentication info pointer then return, because
        // it is used in other thread.
        if (!remote_participant_info)
        {
            return false;
        }

        if (remote_participant_info->auth_status_ == AUTHENTICATION_FAILED)
        {
            remote_participant_info->auth_status_ = AUTHENTICATION_REQUEST_NOT_SEND;
        }
    }

    bool returnedValue = true;

    if (remote_participant_info->auth_status_ == AUTHENTICATION_REQUEST_NOT_SEND)
    {
        // Maybe send request.
        returnedValue = on_process_handshake(*remote_participant_data, remote_participant_info,
                        MessageIdentity(), HandshakeMessageToken(), notify_part_authorized);
    }

    restore_discovered_participant_info(remote_participant_data->guid, remote_participant_info);

    if (notify_part_authorized)
    {
        notify_participant_authorized(*remote_participant_data);
    }

    return returnedValue;
}

void SecurityManager::remove_participant(
        const ParticipantProxyData& participant_data)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return;
    }

    // Unmatch from builtin endpoints.
    unmatch_builtin_endpoints(participant_data);

    {
        shared_lock<shared_mutex> sl(mutex_);

        auto dp_it = discovered_participants_.find(participant_data.guid);

        if (dp_it != discovered_participants_.end())
        {
            SecurityException exception;

            std::shared_ptr<ParticipantCryptoHandle> participant_crypto_handle =
                    dp_it->second->get_participant_crypto();
            if (participant_crypto_handle != nullptr)
            {
                crypto_plugin_->cryptokeyfactory()->unregister_participant(participant_crypto_handle,
                        exception);
            }

            PermissionsHandle* permissions_handle = dp_it->second->get_permissions_handle();
            if (permissions_handle != nullptr)
            {
                access_plugin_->return_permissions_handle(permissions_handle, exception);
            }

            std::shared_ptr<SecretHandle> shared_secret_handle = dp_it->second->get_shared_secret();
            if (shared_secret_handle != nullptr)
            {
                authentication_plugin_->return_sharedsecret_handle(shared_secret_handle, exception);
            }

            remove_discovered_participant_info(dp_it->second->get_auth());

        }
    }

    {
        std::lock_guard<shared_mutex> _(mutex_);

        for (auto& local_reader : reader_handles_)
        {
            for (auto wit = local_reader.second.associated_writers.begin();
                    local_reader.second.associated_writers.end() != wit;
                    )
            {
                if (wit->first.guidPrefix == participant_data.guid.guidPrefix)
                {
                    wit = local_reader.second.associated_writers.erase(wit);
                }
                else
                {
                    ++wit;
                }
            }
        }

        for (auto& local_writer : writer_handles_)
        {
            for (auto rit = local_writer.second.associated_readers.begin();
                    local_writer.second.associated_readers.end() != rit;
                    )
            {
                if (rit->first.guidPrefix == participant_data.guid.guidPrefix)
                {
                    rit = local_writer.second.associated_readers.erase(rit);
                }
                else
                {
                    ++rit;
                }
            }
        }

        discovered_participants_.erase(participant_data.guid);
    }
}

bool SecurityManager::on_process_handshake(
        const ParticipantProxyData& participant_data,
        DiscoveredParticipantInfo::AuthUniquePtr& remote_participant_info,
        MessageIdentity&& message_identity,
        HandshakeMessageToken&& message_in,
        bool& notify_part_authorized)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    HandshakeMessageToken* handshake_message = nullptr;
    SecurityException exception;

    ValidationResult_t ret = VALIDATION_FAILED;

    assert(remote_participant_info->identity_handle_ != nullptr);

    EPROSIMA_LOG_INFO(SECURITY, "Processing handshake from participant " << participant_data.guid);

    if (remote_participant_info->auth_status_ == AUTHENTICATION_REQUEST_NOT_SEND)
    {
        ret = authentication_plugin_->begin_handshake_request(&remote_participant_info->handshake_handle_,
                        &handshake_message,
                        *local_identity_handle_,
                        *remote_participant_info->identity_handle_,
                        participant_->pdp()->get_participant_proxy_data_serialized(BIGEND),
                        exception);
    }
    else if (remote_participant_info->auth_status_ == AUTHENTICATION_WAITING_REQUEST)
    {
        assert(!remote_participant_info->handshake_handle_);
        ret = authentication_plugin_->begin_handshake_reply(&remote_participant_info->handshake_handle_,
                        &handshake_message,
                        std::move(message_in),
                        *remote_participant_info->identity_handle_,
                        *local_identity_handle_,
                        participant_->pdp()->get_participant_proxy_data_serialized(BIGEND),
                        exception);
    }
    else if (remote_participant_info->auth_status_ == AUTHENTICATION_WAITING_REPLY ||
            remote_participant_info->auth_status_ == AUTHENTICATION_WAITING_FINAL)
    {
        assert(remote_participant_info->handshake_handle_);
        ret = authentication_plugin_->process_handshake(&handshake_message,
                        std::move(message_in),
                        *remote_participant_info->handshake_handle_,
                        exception);
    }
    else if (remote_participant_info->auth_status_ == AUTHENTICATION_OK)
    {
        return true;
    }

    if (ret == VALIDATION_FAILED)
    {
        remote_participant_info->auth_status_ = AUTHENTICATION_FAILED;
        on_validation_failed(participant_data, exception);
        return false;
    }

    assert(remote_participant_info->handshake_handle_ != nullptr);

    // Remove previous change
    remote_participant_info->event_->cancel_timer();
    if (remote_participant_info->change_sequence_number_ != SequenceNumber_t::unknown())
    {
        participant_stateless_message_writer_history_->remove_change(remote_participant_info->change_sequence_number_);
        remote_participant_info->change_sequence_number_ = SequenceNumber_t::unknown();
    }
    int64_t expected_sequence_number = 0;

    bool handshake_message_send = true;

    if (ret == VALIDATION_PENDING_HANDSHAKE_MESSAGE ||
            ret == VALIDATION_OK_WITH_FINAL_MESSAGE)
    {
        handshake_message_send = false;

        assert(handshake_message);

        // Send hanshake message

        // Create message
        ParticipantGenericMessage message = generate_authentication_message(std::move(message_identity),
                        participant_data.guid, *handshake_message);

        CacheChange_t* change = create_change_for_message(
            message,
            participant_stateless_message_writer_history_);

        if (change != nullptr)
        {
            // Serialize message
            CDRMessage_t aux_msg(0);
            aux_msg.wraps = true;
            aux_msg.buffer = change->serializedPayload.data;
            aux_msg.length = change->serializedPayload.length;
            aux_msg.max_size = change->serializedPayload.max_size;

            // Serialize encapsulation
            CDRMessage::addOctet(&aux_msg, 0);
            aux_msg.msg_endian = DEFAULT_ENDIAN;
            change->serializedPayload.encapsulation = PL_DEFAULT_ENCAPSULATION;
            CDRMessage::addOctet(&aux_msg, DEFAULT_ENCAPSULATION);
            CDRMessage::addUInt16(&aux_msg, 0);

            if (CDRMessage::addParticipantGenericMessage(&aux_msg, message))
            {
                change->serializedPayload.length = aux_msg.length;

                // Send
                EPROSIMA_LOG_INFO(SECURITY, "Authentication handshake sent to participant " <<
                        participant_data.guid);
                if (participant_stateless_message_writer_history_->add_change(change))
                {
                    handshake_message_send = true;
                    expected_sequence_number = message.message_identity().sequence_number();
                    remote_participant_info->change_sequence_number_ = change->sequenceNumber;
                    remote_participant_info->handshake_requests_sent_++;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(SECURITY, "WriterHistory cannot add the CacheChange_t");
                    // Return the handshake handle
                    authentication_plugin_->return_handshake_handle(remote_participant_info->handshake_handle_,
                            exception);
                    remote_participant_info->handshake_handle_ = nullptr;
                    participant_stateless_message_writer_history_->release_change(change);
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(SECURITY, "Cannot serialize ParticipantGenericMessage");
                participant_stateless_message_writer_history_->release_change(change);
            }
        }
        else
        {
            // Return the handshake handle
            authentication_plugin_->return_handshake_handle(remote_participant_info->handshake_handle_, exception);
            remote_participant_info->handshake_handle_ = nullptr;
            EPROSIMA_LOG_ERROR(SECURITY, "WriterHistory cannot retrieve a CacheChange_t");
        }
    }

    bool returnedValue = false;
    AuthenticationStatus pre_auth_status = remote_participant_info->auth_status_;

    if (handshake_message_send)
    {
        switch (ret)
        {
            case VALIDATION_OK:
            case VALIDATION_OK_WITH_FINAL_MESSAGE:
            case VALIDATION_PENDING_HANDSHAKE_MESSAGE:
            {
                remote_participant_info->auth_status_ = AUTHENTICATION_OK;
                if (ret == VALIDATION_PENDING_HANDSHAKE_MESSAGE)
                {
                    if (pre_auth_status == AUTHENTICATION_REQUEST_NOT_SEND)
                    {
                        remote_participant_info->auth_status_ = AUTHENTICATION_WAITING_REPLY;
                    }
                    else if (pre_auth_status == AUTHENTICATION_WAITING_REQUEST)
                    {
                        remote_participant_info->auth_status_ = AUTHENTICATION_WAITING_FINAL;
                    }
                }

                // if authentication was finished, starts encryption.
                if (remote_participant_info->auth_status_ == AUTHENTICATION_OK)
                {
                    std::shared_ptr<SecretHandle> shared_secret_handle = authentication_plugin_->get_shared_secret(
                        *remote_participant_info->handshake_handle_, exception);
                    if (!participant_authorized(participant_data, remote_participant_info,
                            shared_secret_handle))
                    {
                        authentication_plugin_->return_sharedsecret_handle(shared_secret_handle, exception);
                    }
                    else
                    {
                        notify_part_authorized = true;
                    }

                }

                if (ret == VALIDATION_PENDING_HANDSHAKE_MESSAGE)
                {
                    remote_participant_info->expected_sequence_number_ = expected_sequence_number;
                    // Avoid DoS attack by exponentially increasing event interval
                    auto time_ms = remote_participant_info->event_->getIntervalMilliSec();
                    remote_participant_info->event_->update_interval_millisec(
                        time_ms * auth_handshake_props_.handshake_resend_period_gain_);
                    remote_participant_info->event_->restart_timer();
                }

                returnedValue = true;
            }
            break;
            case VALIDATION_PENDING_RETRY:
            // TODO(Ricardo) Send event.
            default:
                break;
        }
    }

    return returnedValue;
}

bool SecurityManager::create_entities()
{
    if (create_participant_stateless_message_entities())
    {
        if (crypto_plugin_ == nullptr || create_participant_volatile_message_secure_entities())
        {
            EPROSIMA_LOG_INFO(SECURITY, "Initialized security manager for participant " << participant_->getGuid());
            return true;
        }

        delete_participant_stateless_message_entities();
    }

    cancel_init();
    return false;
}

void SecurityManager::delete_entities()
{
    delete_participant_volatile_message_secure_entities();
    delete_participant_stateless_message_entities();
}

bool SecurityManager::create_participant_stateless_message_entities()
{
    create_participant_stateless_message_pool();
    if (create_participant_stateless_message_writer())
    {
        if (create_participant_stateless_message_reader())
        {
            return true;
        }
        delete_participant_stateless_message_writer();
    }

    delete_participant_stateless_message_pool();
    return false;
}

void SecurityManager::delete_participant_stateless_message_entities()
{
    delete_participant_stateless_message_reader();
    delete_participant_stateless_message_writer();
    delete_participant_stateless_message_pool();
}

void SecurityManager::create_participant_stateless_message_pool()
{
    participant_stateless_message_writer_hattr_ =
    { PREALLOCATED_WITH_REALLOC_MEMORY_MODE, PARTICIPANT_STATELESS_MESSAGE_PAYLOAD_DEFAULT_SIZE, 20, 100};
    participant_stateless_message_reader_hattr_ =
    { PREALLOCATED_WITH_REALLOC_MEMORY_MODE, PARTICIPANT_STATELESS_MESSAGE_PAYLOAD_DEFAULT_SIZE, 10, 5000};

    BasicPoolConfig cfg{ PREALLOCATED_WITH_REALLOC_MEMORY_MODE, PARTICIPANT_STATELESS_MESSAGE_PAYLOAD_DEFAULT_SIZE};
    participant_stateless_message_pool_ = TopicPayloadPoolRegistry::get("DCPSParticipantStatelessMessage", cfg);

    PoolConfig writer_cfg = PoolConfig::from_history_attributes(participant_stateless_message_writer_hattr_);
    participant_stateless_message_pool_->reserve_history(writer_cfg, false);

    PoolConfig reader_cfg = PoolConfig::from_history_attributes(participant_stateless_message_reader_hattr_);
    participant_stateless_message_pool_->reserve_history(reader_cfg, true);
}

void SecurityManager::delete_participant_stateless_message_pool()
{
    if (participant_stateless_message_pool_)
    {
        PoolConfig writer_cfg = PoolConfig::from_history_attributes(participant_stateless_message_writer_hattr_);
        participant_stateless_message_pool_->release_history(writer_cfg, false);

        PoolConfig reader_cfg = PoolConfig::from_history_attributes(participant_stateless_message_reader_hattr_);
        participant_stateless_message_pool_->release_history(reader_cfg, true);

        participant_stateless_message_pool_.reset();
    }
}

bool SecurityManager::create_participant_stateless_message_writer()
{
    participant_stateless_message_writer_history_ = new WriterHistory(
        participant_stateless_message_writer_hattr_,
        participant_stateless_message_pool_);

    const RTPSParticipantAttributes& pattr = participant_->get_attributes();

    WriterAttributes watt;
    watt.endpoint.external_unicast_locators = pattr.builtin.metatraffic_external_unicast_locators;
    watt.endpoint.ignore_non_matching_locators = pattr.ignore_non_matching_locators;
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.reliabilityKind = BEST_EFFORT;
    watt.endpoint.topicKind = NO_KEY;
    watt.matched_readers_allocation = pattr.allocation.participants;
    watt.separate_sending = true;

    RTPSWriter* wout = nullptr;
    if (participant_->createWriter(&wout, watt,
            participant_stateless_message_writer_history_, nullptr,
            participant_stateless_message_writer_entity_id, true))
    {
        participant_->set_endpoint_rtps_protection_supports(wout, false);
        participant_stateless_message_writer_ = dynamic_cast<StatelessWriter*>(wout);
        auth_source_guid = participant_stateless_message_writer_->getGuid();

        return true;
    }

    EPROSIMA_LOG_ERROR(SECURITY, "Participant Stateless Message Writer creation failed");
    delete(participant_stateless_message_writer_history_);
    participant_stateless_message_writer_history_ = nullptr;

    return false;
}

void SecurityManager::delete_participant_stateless_message_writer()
{
    if (participant_stateless_message_writer_ != nullptr)
    {
        participant_->deleteUserEndpoint(participant_stateless_message_writer_->getGuid());
        participant_stateless_message_writer_ = nullptr;
    }

    if (participant_stateless_message_writer_history_ != nullptr)
    {
        delete participant_stateless_message_writer_history_;
        participant_stateless_message_writer_history_ = nullptr;
    }
}

bool SecurityManager::create_participant_stateless_message_reader()
{
    participant_stateless_message_reader_history_ = new ReaderHistory(participant_stateless_message_reader_hattr_);

    const RTPSParticipantAttributes& pattr = participant_->get_attributes();

    ReaderAttributes ratt;
    ratt.endpoint.topicKind = NO_KEY;
    ratt.endpoint.reliabilityKind = BEST_EFFORT;
    if (!pattr.builtin.avoid_builtin_multicast)
    {
        ratt.endpoint.multicastLocatorList = pattr.builtin.metatrafficMulticastLocatorList;
    }
    ratt.endpoint.unicastLocatorList = pattr.builtin.metatrafficUnicastLocatorList;
    ratt.endpoint.external_unicast_locators = pattr.builtin.metatraffic_external_unicast_locators;
    ratt.endpoint.ignore_non_matching_locators = pattr.ignore_non_matching_locators;
    ratt.matched_writers_allocation = pattr.allocation.participants;

    RTPSReader* rout = nullptr;
    if (participant_->createReader(&rout, ratt, participant_stateless_message_pool_,
            participant_stateless_message_reader_history_, &participant_stateless_message_listener_,
            participant_stateless_message_reader_entity_id, true, true))
    {
        participant_->set_endpoint_rtps_protection_supports(rout, false);
        participant_stateless_message_reader_ = dynamic_cast<StatelessReader*>(rout);

        return true;
    }

    EPROSIMA_LOG_ERROR(SECURITY, "Participant Stateless Message Reader creation failed");
    delete(participant_stateless_message_reader_history_);
    participant_stateless_message_reader_history_ = nullptr;
    return false;
}

void SecurityManager::delete_participant_stateless_message_reader()
{
    if (participant_stateless_message_reader_ != nullptr)
    {
        participant_->deleteUserEndpoint(participant_stateless_message_reader_->getGuid());
        participant_stateless_message_reader_ = nullptr;
    }

    if (participant_stateless_message_reader_history_ != nullptr)
    {
        delete participant_stateless_message_reader_history_;
        participant_stateless_message_reader_history_ = nullptr;
    }
}

bool SecurityManager::create_participant_volatile_message_secure_entities()
{
    create_participant_volatile_message_secure_pool();

    if (create_participant_volatile_message_secure_writer())
    {
        if (create_participant_volatile_message_secure_reader())
        {
            return true;
        }

        delete_participant_volatile_message_secure_writer();
    }

    delete_participant_volatile_message_secure_pool();
    return false;
}

void SecurityManager::delete_participant_volatile_message_secure_entities()
{
    delete_participant_volatile_message_secure_reader();
    delete_participant_volatile_message_secure_writer();
    delete_participant_volatile_message_secure_pool();
}

void SecurityManager::create_participant_volatile_message_secure_pool()
{
    participant_volatile_message_secure_hattr_ =
    { PREALLOCATED_WITH_REALLOC_MEMORY_MODE, PARTICIPANT_VOLATILE_MESSAGE_PAYLOAD_DEFAULT_SIZE, 10, 0 };

    PoolConfig pool_cfg = PoolConfig::from_history_attributes(participant_volatile_message_secure_hattr_);
    participant_volatile_message_secure_pool_ =
            TopicPayloadPoolRegistry::get("DCPSParticipantVolatileMessageSecure", pool_cfg);
    participant_volatile_message_secure_pool_->reserve_history(pool_cfg, false);
    participant_volatile_message_secure_pool_->reserve_history(pool_cfg, true);
}

void SecurityManager::delete_participant_volatile_message_secure_pool()
{
    if (participant_volatile_message_secure_pool_)
    {
        PoolConfig pool_cfg = PoolConfig::from_history_attributes(participant_volatile_message_secure_hattr_);
        participant_volatile_message_secure_pool_->release_history(pool_cfg, true);
        participant_volatile_message_secure_pool_->release_history(pool_cfg, false);
        participant_volatile_message_secure_pool_.reset();
    }
}

bool SecurityManager::create_participant_volatile_message_secure_writer()
{
    participant_volatile_message_secure_writer_history_ =
            new WriterHistory(participant_volatile_message_secure_hattr_, participant_volatile_message_secure_pool_);

    const RTPSParticipantAttributes& pattr = participant_->get_attributes();

    WriterAttributes watt;
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.reliabilityKind = RELIABLE;
    watt.endpoint.topicKind = NO_KEY;
    watt.endpoint.durabilityKind = VOLATILE;
    watt.endpoint.unicastLocatorList = pattr.builtin.metatrafficUnicastLocatorList;
    watt.endpoint.external_unicast_locators = pattr.builtin.metatraffic_external_unicast_locators;
    watt.endpoint.ignore_non_matching_locators = pattr.ignore_non_matching_locators;
    watt.endpoint.security_attributes().is_submessage_protected = true;
    watt.endpoint.security_attributes().plugin_endpoint_attributes =
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
    watt.matched_readers_allocation = pattr.allocation.participants;
    watt.separate_sending = true;

    RTPSWriter* wout = nullptr;
    if (participant_->createWriter(&wout, watt,
            participant_volatile_message_secure_writer_history_,
            this, participant_volatile_message_secure_writer_entity_id, true))
    {
        participant_->set_endpoint_rtps_protection_supports(wout, false);
        participant_volatile_message_secure_writer_ = dynamic_cast<StatefulWriter*>(wout);
        return true;
    }

    EPROSIMA_LOG_ERROR(SECURITY, "Participant Volatile Message Writer creation failed");
    delete(participant_volatile_message_secure_writer_history_);
    participant_volatile_message_secure_writer_history_ = nullptr;

    return false;
}

void SecurityManager::delete_participant_volatile_message_secure_writer()
{
    if (participant_volatile_message_secure_writer_ != nullptr)
    {
        participant_->deleteUserEndpoint(participant_volatile_message_secure_writer_->getGuid());
        participant_volatile_message_secure_writer_ = nullptr;
    }

    if (participant_volatile_message_secure_writer_history_ != nullptr)
    {
        delete participant_volatile_message_secure_writer_history_;
        participant_volatile_message_secure_writer_history_ = nullptr;
    }
}

bool SecurityManager::create_participant_volatile_message_secure_reader()
{
    participant_volatile_message_secure_reader_history_ =
            new ReaderHistory(participant_volatile_message_secure_hattr_);

    const RTPSParticipantAttributes& pattr = participant_->get_attributes();

    ReaderAttributes ratt;
    ratt.endpoint.topicKind = NO_KEY;
    ratt.endpoint.reliabilityKind = RELIABLE;
    ratt.endpoint.durabilityKind = VOLATILE;
    ratt.endpoint.unicastLocatorList = pattr.builtin.metatrafficUnicastLocatorList;
    ratt.endpoint.external_unicast_locators = pattr.builtin.metatraffic_external_unicast_locators;
    ratt.endpoint.ignore_non_matching_locators = pattr.ignore_non_matching_locators;
    ratt.endpoint.security_attributes().is_submessage_protected = true;
    ratt.endpoint.security_attributes().plugin_endpoint_attributes =
            PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
    ratt.matched_writers_allocation = pattr.allocation.participants;

    RTPSReader* rout = nullptr;
    if (participant_->createReader(&rout, ratt, participant_volatile_message_secure_pool_,
            participant_volatile_message_secure_reader_history_, &participant_volatile_message_secure_listener_,
            participant_volatile_message_secure_reader_entity_id, true, true))
    {
        participant_->set_endpoint_rtps_protection_supports(rout, false);
        participant_volatile_message_secure_reader_ = dynamic_cast<StatefulReader*>(rout);

        return true;
    }

    EPROSIMA_LOG_ERROR(SECURITY, "Participant Volatile Message Reader creation failed");
    delete(participant_volatile_message_secure_reader_history_);
    participant_volatile_message_secure_reader_history_ = nullptr;
    return false;
}

void SecurityManager::delete_participant_volatile_message_secure_reader()
{
    if (participant_volatile_message_secure_reader_ != nullptr)
    {
        participant_->deleteUserEndpoint(participant_volatile_message_secure_reader_->getGuid());
        participant_volatile_message_secure_reader_ = nullptr;
    }

    if (participant_volatile_message_secure_reader_history_ != nullptr)
    {
        delete participant_volatile_message_secure_reader_history_;
        participant_volatile_message_secure_reader_history_ = nullptr;
    }
}

ParticipantGenericMessage SecurityManager::generate_authentication_message(
        const MessageIdentity& related_message_identity,
        const GUID_t& destination_participant_key,
        HandshakeMessageToken& handshake_message) const
{
    ParticipantGenericMessage message;

    message.message_identity().source_guid(auth_source_guid);
    message.message_identity().sequence_number(auth_last_sequence_number_.fetch_add(1));
    message.related_message_identity(related_message_identity);
    message.destination_participant_key(destination_participant_key);
    message.message_class_id(AUTHENTICATION_PARTICIPANT_STATELESS_MESSAGE);
    message.message_data().push_back(handshake_message);

    return message;
}

ParticipantGenericMessage SecurityManager::generate_participant_crypto_token_message(
        const GUID_t& destination_participant_key,
        ParticipantCryptoTokenSeq& crypto_tokens) const
{
    ParticipantGenericMessage message;

    message.message_identity().source_guid(auth_source_guid);
    message.message_identity().sequence_number(crypto_last_sequence_number_.fetch_add(1));
    message.destination_participant_key(destination_participant_key);
    message.message_class_id(GMCLASSID_SECURITY_PARTICIPANT_CRYPTO_TOKENS);
    message.message_data().assign(crypto_tokens.begin(), crypto_tokens.end());

    return message;
}

ParticipantGenericMessage SecurityManager::generate_writer_crypto_token_message(
        const GUID_t& destination_participant_key,
        const GUID_t& destination_endpoint_key,
        const GUID_t& source_endpoint_key,
        ParticipantCryptoTokenSeq& crypto_tokens) const
{
    ParticipantGenericMessage message;

    message.message_identity().source_guid(auth_source_guid);
    message.message_identity().sequence_number(crypto_last_sequence_number_.fetch_add(1));
    message.destination_participant_key(destination_participant_key);
    message.destination_endpoint_key(destination_endpoint_key);
    message.source_endpoint_key(source_endpoint_key);
    message.message_class_id(GMCLASSID_SECURITY_WRITER_CRYPTO_TOKENS);
    message.message_data().assign(crypto_tokens.begin(), crypto_tokens.end());

    return message;
}

ParticipantGenericMessage SecurityManager::generate_reader_crypto_token_message(
        const GUID_t& destination_participant_key,
        const GUID_t& destination_endpoint_key,
        const GUID_t& source_endpoint_key,
        ParticipantCryptoTokenSeq& crypto_tokens) const
{
    ParticipantGenericMessage message;

    message.message_identity().source_guid(auth_source_guid);
    message.message_identity().sequence_number(crypto_last_sequence_number_.fetch_add(1));
    message.destination_participant_key(destination_participant_key);
    message.destination_endpoint_key(destination_endpoint_key);
    message.source_endpoint_key(source_endpoint_key);
    message.message_class_id(GMCLASSID_SECURITY_READER_CRYPTO_TOKENS);
    message.message_data().assign(crypto_tokens.begin(), crypto_tokens.end());

    return message;
}

void SecurityManager::process_participant_stateless_message(
        const CacheChange_t* const change)
{
    assert(change);

    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return;
    }

    // Deserialize message
    ParticipantGenericMessage message;
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.length = change->serializedPayload.length;
    aux_msg.max_size = change->serializedPayload.max_size;

    // Read encapsulation
    aux_msg.pos += 1;
    octet encapsulation = 0;
    CDRMessage::readOctet(&aux_msg, &encapsulation);
    if (encapsulation == CDR_BE)
    {
        aux_msg.msg_endian = BIGEND;
    }
    else if (encapsulation == CDR_LE)
    {
        aux_msg.msg_endian = LITTLEEND;
    }
    else
    {
        return;
    }

    aux_msg.pos += 2;

    CDRMessage::readParticipantGenericMessage(&aux_msg, message);

    if (message.message_class_id().compare(AUTHENTICATION_PARTICIPANT_STATELESS_MESSAGE) == 0)
    {
        if (message.message_identity().source_guid() == GUID_t::unknown())
        {
            EPROSIMA_LOG_INFO(SECURITY,
                    "Bad ParticipantGenericMessage. message_identity.source_guid is GUID_t::unknown()");
            return;
        }
        if (message.destination_participant_key() != participant_->getGuid())
        {
            EPROSIMA_LOG_INFO(SECURITY, "Destination of ParticipantGenericMessage is not me");
            return;
        }
        if (message.destination_endpoint_key() != GUID_t::unknown())
        {
            EPROSIMA_LOG_INFO(SECURITY,
                    "Bad ParticipantGenericMessage. destination_endpoint_key is not GUID_t::unknown()");
            return;
        }
        if (message.source_endpoint_key() != GUID_t::unknown())
        {
            EPROSIMA_LOG_INFO(SECURITY, "Bad ParticipantGenericMessage. source_endpoint_key is not GUID_t::unknown()");
            return;
        }

        const GUID_t remote_participant_key(message.message_identity().source_guid().guidPrefix,
                c_EntityId_RTPSParticipant);
        DiscoveredParticipantInfo::AuthUniquePtr remote_participant_info;
        const ParticipantProxyData* participant_data = nullptr;

        {
            shared_lock<shared_mutex> _(mutex_);

            auto dp_it = discovered_participants_.find(remote_participant_key);
            if (dp_it != discovered_participants_.end())
            {
                remote_participant_info = dp_it->second->get_auth();
                participant_data = &(dp_it->second->participant_data());
            }
        }

        if (remote_participant_info && participant_data)
        {
            if (remote_participant_info->auth_status_ == AUTHENTICATION_WAITING_REQUEST)
            {
                assert(!remote_participant_info->handshake_handle_);

                // Preconditions
                if (message.related_message_identity().source_guid() != GUID_t::unknown())
                {
                    EPROSIMA_LOG_INFO(SECURITY,
                            "Bad ParticipantGenericMessage. related_message_identity.source_guid is not GUID_t::unknown()");
                    restore_discovered_participant_info(remote_participant_key, remote_participant_info);
                    return;
                }
                if (message.message_data().size() != 1)
                {
                    EPROSIMA_LOG_INFO(SECURITY, "Bad ParticipantGenericMessage. message_data size is not 1");
                    restore_discovered_participant_info(remote_participant_key, remote_participant_info);
                    return;
                }
            }
            else if (remote_participant_info->auth_status_ == AUTHENTICATION_WAITING_REPLY ||
                    remote_participant_info->auth_status_ == AUTHENTICATION_WAITING_FINAL)
            {
                assert(remote_participant_info->handshake_handle_);

                if (message.related_message_identity().source_guid() == GUID_t::unknown() &&
                        remote_participant_info->auth_status_ == AUTHENTICATION_WAITING_FINAL)
                {
                    // Maybe the reply was missed. Resent.
                    if (remote_participant_info->change_sequence_number_ != SequenceNumber_t::unknown())
                    {
                        // Remove previous change and send a new one.
                        CacheChange_t* p_change =
                                participant_stateless_message_writer_history_->remove_change_and_reuse(
                            remote_participant_info->change_sequence_number_);
                        remote_participant_info->change_sequence_number_ = SequenceNumber_t::unknown();

                        if (p_change != nullptr)
                        {
                            if (participant_stateless_message_writer_history_->add_change(p_change))
                            {
                                remote_participant_info->change_sequence_number_ = p_change->sequenceNumber;
                                remote_participant_info->handshake_requests_sent_++;
                            }
                            //TODO (Ricardo) What to do if not added?
                        }

                        restore_discovered_participant_info(remote_participant_key, remote_participant_info);
                        return;
                    }
                }

                // Preconditions
                if (message.related_message_identity().source_guid()
                        != participant_stateless_message_writer_->getGuid())
                {
                    EPROSIMA_LOG_INFO(SECURITY,
                            "Bad ParticipantGenericMessage. related_message_identity.source_guid is not mine");
                    restore_discovered_participant_info(remote_participant_key, remote_participant_info);
                    return;
                }
                if (message.related_message_identity().sequence_number()
                        != remote_participant_info->expected_sequence_number_)
                {
                    EPROSIMA_LOG_INFO(SECURITY,
                            "Bad ParticipantGenericMessage. related_message_identity.sequence_number is not expected");
                    restore_discovered_participant_info(remote_participant_key, remote_participant_info);
                    return;
                }
                if (message.message_data().size() != 1)
                {
                    EPROSIMA_LOG_INFO(SECURITY, "Bad ParticipantGenericMessage. message_data size is not 1");
                    restore_discovered_participant_info(remote_participant_key, remote_participant_info);
                    return;
                }
            }
            else if (remote_participant_info->auth_status_ == AUTHENTICATION_OK)
            {
                // Preconditions
                if (message.related_message_identity().source_guid()
                        != participant_stateless_message_writer_->getGuid())
                {
                    EPROSIMA_LOG_INFO(SECURITY,
                            "Bad ParticipantGenericMessage. related_message_identity.source_guid is not mine");
                    restore_discovered_participant_info(remote_participant_key, remote_participant_info);
                    return;
                }
                if (message.related_message_identity().sequence_number()
                        != remote_participant_info->expected_sequence_number_)
                {
                    EPROSIMA_LOG_INFO(SECURITY,
                            "Bad ParticipantGenericMessage. related_message_identity.sequence_number is not expected");
                    restore_discovered_participant_info(remote_participant_key, remote_participant_info);
                    return;
                }
                if (message.message_data().size() != 1)
                {
                    EPROSIMA_LOG_INFO(SECURITY, "Bad ParticipantGenericMessage. message_data size is not 1");
                    restore_discovered_participant_info(remote_participant_key, remote_participant_info);
                    return;
                }

                // Maybe final message was missed. Resent.
                if (remote_participant_info->change_sequence_number_ != SequenceNumber_t::unknown())
                {
                    // Remove previous change and send a new one.
                    CacheChange_t* p_change = participant_stateless_message_writer_history_->remove_change_and_reuse(
                        remote_participant_info->change_sequence_number_);
                    remote_participant_info->change_sequence_number_ = SequenceNumber_t::unknown();

                    if (p_change != nullptr)
                    {
                        if (participant_stateless_message_writer_history_->add_change(p_change))
                        {
                            remote_participant_info->change_sequence_number_ = p_change->sequenceNumber;
                            remote_participant_info->handshake_requests_sent_++;
                        }
                        //TODO (Ricardo) What to do if not added?
                    }

                    restore_discovered_participant_info(remote_participant_key, remote_participant_info);
                    return;
                }
            }
            else
            {
                restore_discovered_participant_info(remote_participant_key, remote_participant_info);
                return;
            }

            bool notify_part_authorized = false;
            on_process_handshake(*participant_data, remote_participant_info,
                    std::move(message.message_identity()),
                    std::move(message.message_data().at(0)), notify_part_authorized);

            restore_discovered_participant_info(remote_participant_key, remote_participant_info);

            if (notify_part_authorized)
            {
                notify_participant_authorized(*participant_data);
            }
        }
        else
        {
            EPROSIMA_LOG_INFO(SECURITY, "Received Authentication message but not found related remote_participant_key");
        }
    }
    else
    {
        EPROSIMA_LOG_INFO(SECURITY, "Discarted ParticipantGenericMessage with class id " << message.message_class_id());
    }
}

void SecurityManager::process_participant_volatile_message_secure(
        const CacheChange_t* const change)
{
    assert(change);

    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return;
    }

    // Deserialize message
    ParticipantGenericMessage message;
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.length = change->serializedPayload.length;
    aux_msg.max_size = change->serializedPayload.max_size;

    // Read encapsulation
    aux_msg.pos += 1;
    octet encapsulation = 0;
    CDRMessage::readOctet(&aux_msg, &encapsulation);
    if (encapsulation == CDR_BE)
    {
        aux_msg.msg_endian = BIGEND;
    }
    else if (encapsulation == CDR_LE)
    {
        aux_msg.msg_endian = LITTLEEND;
    }
    else
    {
        return;
    }

    aux_msg.pos += 2;

    CDRMessage::readParticipantGenericMessage(&aux_msg, message);

    if (message.message_class_id().compare(GMCLASSID_SECURITY_PARTICIPANT_CRYPTO_TOKENS) == 0)
    {
        if (message.message_identity().source_guid() == GUID_t::unknown())
        {
            EPROSIMA_LOG_INFO(SECURITY,
                    "Bad ParticipantGenericMessage. message_identity.source_guid is GUID_t::unknown()");
            return;
        }
        if (message.destination_participant_key() != participant_->getGuid())
        {
            EPROSIMA_LOG_INFO(SECURITY, "Destination of ParticipantGenericMessage is not me");
            return;
        }
        if (message.destination_endpoint_key() != GUID_t::unknown())
        {
            EPROSIMA_LOG_INFO(SECURITY,
                    "Bad ParticipantGenericMessage. destination_endpoint_key is not GUID_t::unknown()");
            return;
        }
        if (message.source_endpoint_key() != GUID_t::unknown())
        {
            EPROSIMA_LOG_INFO(SECURITY, "Bad ParticipantGenericMessage. source_endpoint_key is not GUID_t::unknown()");
            return;
        }

        const GUID_t remote_participant_key(message.message_identity().source_guid().guidPrefix,
                c_EntityId_RTPSParticipant);
        std::shared_ptr<ParticipantCryptoHandle> remote_participant_crypto;
        DiscoveredParticipantInfo::AuthUniquePtr remote_participant_info;

        // Search remote participant crypto handle.
        {
            shared_lock<shared_mutex> lock(mutex_);

            auto dp_it = discovered_participants_.find(remote_participant_key);

            if (dp_it != discovered_participants_.end())
            {
                if (dp_it->second->get_participant_crypto() == nullptr)
                {
                    return;
                }

                remote_participant_crypto = dp_it->second->get_participant_crypto();
                remote_participant_info = dp_it->second->get_auth();
            }
            else
            {
                EPROSIMA_LOG_INFO(SECURITY,
                        "Received Participant Cryptography message but not found related remote_participant_key");
            }
        }

        if (remote_participant_crypto != nullptr)
        {
            SecurityException exception;

            if (!crypto_plugin_->cryptokeyexchange()->set_remote_participant_crypto_tokens(
                        *local_participant_crypto_handle_,
                        *remote_participant_crypto,
                        message.message_data(),
                        exception))
            {
                EPROSIMA_LOG_ERROR(SECURITY, "Cannot set remote participant crypto tokens ("
                        << remote_participant_key << ") - (" << exception.what() << ")");
            }
            else
            {
                // Release the change from the participant_stateless_message_writer_pool_
                // As both participants have already authorized each other

                if (remote_participant_info &&
                        remote_participant_info->change_sequence_number_ != SequenceNumber_t::unknown())
                {
                    participant_stateless_message_writer_history_->remove_change(
                        remote_participant_info->change_sequence_number_);
                    remote_participant_info->change_sequence_number_ = SequenceNumber_t::unknown();
                }
            }
        }
        else
        {
            std::lock_guard<shared_mutex> _(mutex_);
            remote_participant_pending_messages_.emplace(remote_participant_key, std::move(message.message_data()));
        }

        if (remote_participant_info)
        {
            restore_discovered_participant_info(remote_participant_key, remote_participant_info);
        }
    }
    else if (message.message_class_id().compare(GMCLASSID_SECURITY_READER_CRYPTO_TOKENS) == 0)
    {
        if (message.message_identity().source_guid() == GUID_t::unknown())
        {
            EPROSIMA_LOG_INFO(SECURITY,
                    "Bad ParticipantGenericMessage. message_identity.source_guid is GUID_t::unknown()");
            return;
        }
        if (message.destination_participant_key() != participant_->getGuid())
        {
            EPROSIMA_LOG_INFO(SECURITY, "Destination of ParticipantGenericMessage is not me");
            return;
        }
        if (message.destination_endpoint_key() == GUID_t::unknown())
        {
            EPROSIMA_LOG_INFO(SECURITY, "Bad ParticipantGenericMessage. destination_endpoint_key is GUID_t::unknown()");
            return;
        }
        if (message.source_endpoint_key() == GUID_t::unknown())
        {
            EPROSIMA_LOG_INFO(SECURITY, "Bad ParticipantGenericMessage. source_endpoint_key is GUID_t::unknown()");
            return;
        }

        // Search remote writer handle.
        GUID_t writer_guid;
        ReaderProxyData* reader_data = nullptr;

        {
            std::lock_guard<shared_mutex> _(mutex_);

            auto wr_it = writer_handles_.find(message.destination_endpoint_key());

            if (wr_it != writer_handles_.end())
            {
                auto rd_it = wr_it->second.associated_readers.find(message.source_endpoint_key());

                if (rd_it != wr_it->second.associated_readers.end())
                {
                    SecurityException exception;

                    if (crypto_plugin_->cryptokeyexchange()->set_remote_datareader_crypto_tokens(
                                *wr_it->second.writer_handle,
                                *std::get<1>(rd_it->second),
                                message.message_data(),
                                exception))
                    {
                        writer_guid = wr_it->first;
                        reader_data = &(std::get<0>(rd_it->second));
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(SECURITY, "Cannot set remote reader crypto tokens ("
                                << message.source_endpoint_key() << ") - (" << exception.what() << ")");
                    }
                }
                else
                {
                    remote_reader_pending_messages_.emplace(std::make_pair(message.source_endpoint_key(),
                            message.destination_endpoint_key()),
                            std::move(message.message_data()));
                }
            }
            else
            {
                EPROSIMA_LOG_INFO(SECURITY, "Received Reader Cryptography message but not found local writer " <<
                        message.destination_endpoint_key());
            }
        }

        // If writer was found and setting of crypto tokens works, then tell core to match writer and reader.
        if (writer_guid != GUID_t::unknown())
        {
            participant_->pairing_remote_reader_with_local_writer_after_security(writer_guid,
                    *reader_data);
        }
    }
    else if (message.message_class_id().compare(GMCLASSID_SECURITY_WRITER_CRYPTO_TOKENS) == 0)
    {
        if (message.message_identity().source_guid() == GUID_t::unknown())
        {
            EPROSIMA_LOG_INFO(SECURITY,
                    "Bad ParticipantGenericMessage. message_identity.source_guid is GUID_t::unknown()");
            return;
        }
        if (message.destination_participant_key() != participant_->getGuid())
        {
            EPROSIMA_LOG_INFO(SECURITY, "Destination of ParticipantGenericMessage is not me");
            return;
        }
        if (message.destination_endpoint_key() == GUID_t::unknown())
        {
            EPROSIMA_LOG_INFO(SECURITY, "Bad ParticipantGenericMessage. destination_endpoint_key is GUID_t::unknown()");
            return;
        }
        if (message.source_endpoint_key() == GUID_t::unknown())
        {
            EPROSIMA_LOG_INFO(SECURITY, "Bad ParticipantGenericMessage. source_endpoint_key is GUID_t::unknown()");
            return;
        }

        // Search remote writer handle.
        GUID_t reader_guid;
        WriterProxyData* writer_data = nullptr;

        {
            std::lock_guard<shared_mutex> _(mutex_);
            auto rd_it = reader_handles_.find(message.destination_endpoint_key());

            if (rd_it != reader_handles_.end())
            {
                auto wr_it = rd_it->second.associated_writers.find(message.source_endpoint_key());

                if (wr_it != rd_it->second.associated_writers.end())
                {
                    SecurityException exception;

                    if (crypto_plugin_->cryptokeyexchange()->set_remote_datawriter_crypto_tokens(
                                *rd_it->second.reader_handle,
                                *std::get<1>(wr_it->second),
                                message.message_data(),
                                exception))
                    {
                        reader_guid = rd_it->first;
                        writer_data = &(std::get<0>(wr_it->second));
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(SECURITY, "Cannot set remote writer crypto tokens ("
                                << message.source_endpoint_key() << ") - (" << exception.what() << ")");
                    }
                }
                else
                {
                    remote_writer_pending_messages_.emplace(std::make_pair(message.source_endpoint_key(),
                            message.destination_endpoint_key()),
                            std::move(message.message_data()));
                }
            }
            else
            {
                EPROSIMA_LOG_INFO(SECURITY, "Received Writer Cryptography message but not found local reader " <<
                        message.destination_endpoint_key());
            }
        }

        // If reader was found and setting of crypto tokens works, then tell core to match reader and writer.
        if (reader_guid != GUID_t::unknown())
        {
            participant_->pairing_remote_writer_with_local_reader_after_security(reader_guid, *writer_data);
        }
    }
    else
    {
        EPROSIMA_LOG_INFO(SECURITY, "Discarded ParticipantGenericMessage with class id " << message.message_class_id());
    }
}

void SecurityManager::ParticipantStatelessMessageListener::on_new_cache_change_added(
        RTPSReader* reader,
        const CacheChange_t* const change)
{
    manager_.process_participant_stateless_message(change);

    ReaderHistory* history = reader->get_history();
    assert(history);
    history->remove_change(const_cast<CacheChange_t*>(change));
}

void SecurityManager::ParticipantVolatileMessageListener::on_new_cache_change_added(
        RTPSReader* reader,
        const CacheChange_t* const change)
{
    manager_.process_participant_volatile_message_secure(change);

    ReaderHistory* history = reader->get_history();
    assert(history);
    history->remove_change(const_cast<CacheChange_t*>(change));
}

bool SecurityManager::get_identity_token(
        IdentityToken** identity_token) const
{
    assert(identity_token);

    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    if (authentication_plugin_)
    {
        SecurityException exception;
        return authentication_plugin_->get_identity_token(identity_token,
                       *local_identity_handle_, exception);
    }

    return false;
}

bool SecurityManager::return_identity_token(
        IdentityToken* identity_token) const
{
    if (identity_token == nullptr)
    {
        return true;
    }

    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    if (authentication_plugin_)
    {
        SecurityException exception;
        return authentication_plugin_->return_identity_token(identity_token,
                       exception);
    }

    return false;
}

bool SecurityManager::get_permissions_token(
        PermissionsToken** permissions_token) const
{
    assert(permissions_token);

    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    if (access_plugin_)
    {
        SecurityException exception;
        return access_plugin_->get_permissions_token(permissions_token,
                       *local_permissions_handle_, exception);
    }

    return false;
}

bool SecurityManager::return_permissions_token(
        PermissionsToken* permissions_token) const
{
    if (permissions_token == nullptr)
    {
        return true;
    }

    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    if (access_plugin_)
    {
        SecurityException exception;
        return access_plugin_->return_permissions_token(permissions_token,
                       exception);
    }

    return false;
}

uint32_t SecurityManager::builtin_endpoints() const
{
    uint32_t be = 0;

    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return be;
    }

    if (participant_stateless_message_reader_ != nullptr)
    {
        be |= BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_READER;
    }
    if (participant_stateless_message_writer_ != nullptr)
    {
        be |= BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_WRITER;
    }
    if (participant_volatile_message_secure_reader_ != nullptr)
    {
        be |= BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_READER;
    }
    if (participant_volatile_message_secure_writer_ != nullptr)
    {
        be |= BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_WRITER;
    }

    return be;
}

bool SecurityManager::check_guid_comes_from(
        const GUID_t& adjusted,
        const GUID_t& original) const
{
    bool ret = (original == adjusted);
    if (!ret && authentication_plugin_ != nullptr)
    {
        shared_lock<shared_mutex> _(mutex_);
        auto part_it = discovered_participants_.find(adjusted);
        if (part_it != discovered_participants_.end())
        {
            ret = part_it->second->check_guid_comes_from(authentication_plugin_, adjusted, original);
        }
    }
    return ret;
}

void SecurityManager::match_builtin_endpoints(
        const ParticipantProxyData& participant_data)
{
    uint32_t builtin_endpoints = participant_data.m_available_builtin_endpoints;
    const NetworkFactory& network = participant_->network_factory();

    if (participant_stateless_message_reader_ != nullptr &&
            builtin_endpoints & BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_WRITER)
    {

        auto temp_stateless_writer_proxy_data_ = get_temporary_writer_proxies_pool().get();

        temp_stateless_writer_proxy_data_->clear();
        temp_stateless_writer_proxy_data_->guid.guidPrefix = participant_data.guid.guidPrefix;
        temp_stateless_writer_proxy_data_->guid.entityId = participant_stateless_message_writer_entity_id;
        temp_stateless_writer_proxy_data_->persistence_guid = temp_stateless_writer_proxy_data_->guid;
        temp_stateless_writer_proxy_data_->set_remote_locators(participant_data.metatraffic_locators, network, false,
                participant_data.is_from_this_host());
        temp_stateless_writer_proxy_data_->topic_kind = NO_KEY;
        temp_stateless_writer_proxy_data_->reliability.kind =
                eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
        temp_stateless_writer_proxy_data_->durability.kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
        participant_stateless_message_reader_->matched_writer_add_edp(*temp_stateless_writer_proxy_data_);
    }

    if (participant_stateless_message_writer_ != nullptr &&
            builtin_endpoints & BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_READER)
    {
        auto temp_stateless_reader_proxy_data_ = get_temporary_reader_proxies_pool().get();

        temp_stateless_reader_proxy_data_->clear();
        temp_stateless_reader_proxy_data_->expects_inline_qos = false;
        temp_stateless_reader_proxy_data_->guid.guidPrefix = participant_data.guid.guidPrefix;
        temp_stateless_reader_proxy_data_->guid.entityId = participant_stateless_message_reader_entity_id;
        temp_stateless_reader_proxy_data_->set_remote_locators(participant_data.metatraffic_locators, network, false,
                participant_data.is_from_this_host());
        temp_stateless_reader_proxy_data_->topic_kind = NO_KEY;
        temp_stateless_reader_proxy_data_->reliability.kind =
                eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
        temp_stateless_reader_proxy_data_->durability.kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
        participant_stateless_message_writer_->matched_reader_add_edp(*temp_stateless_reader_proxy_data_);
    }
}

void SecurityManager::match_builtin_key_exchange_endpoints(
        const ParticipantProxyData& participant_data)
{
    uint32_t builtin_endpoints = participant_data.m_available_builtin_endpoints;
    const NetworkFactory& network = participant_->network_factory();

    if (participant_volatile_message_secure_reader_ != nullptr &&
            builtin_endpoints & BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_WRITER)
    {
        auto temp_volatile_writer_proxy_data_ = get_temporary_writer_proxies_pool().get();

        temp_volatile_writer_proxy_data_->clear();
        temp_volatile_writer_proxy_data_->guid.guidPrefix = participant_data.guid.guidPrefix;
        temp_volatile_writer_proxy_data_->guid.entityId = participant_volatile_message_secure_writer_entity_id;
        temp_volatile_writer_proxy_data_->persistence_guid = temp_volatile_writer_proxy_data_->guid;
        temp_volatile_writer_proxy_data_->set_remote_locators(participant_data.metatraffic_locators, network, false,
                participant_data.is_from_this_host());
        temp_volatile_writer_proxy_data_->topic_kind = NO_KEY;
        temp_volatile_writer_proxy_data_->reliability.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        temp_volatile_writer_proxy_data_->durability.kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
        participant_volatile_message_secure_reader_->matched_writer_add_edp(*temp_volatile_writer_proxy_data_);
    }

    if (participant_volatile_message_secure_writer_ != nullptr &&
            builtin_endpoints & BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_READER)
    {
        auto temp_volatile_reader_proxy_data_ = get_temporary_reader_proxies_pool().get();

        temp_volatile_reader_proxy_data_->clear();
        temp_volatile_reader_proxy_data_->expects_inline_qos = false;
        temp_volatile_reader_proxy_data_->guid.guidPrefix = participant_data.guid.guidPrefix;
        temp_volatile_reader_proxy_data_->guid.entityId = participant_volatile_message_secure_reader_entity_id;
        temp_volatile_reader_proxy_data_->set_remote_locators(participant_data.metatraffic_locators, network, false,
                participant_data.is_from_this_host());
        temp_volatile_reader_proxy_data_->topic_kind = NO_KEY;
        temp_volatile_reader_proxy_data_->reliability.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        temp_volatile_reader_proxy_data_->durability.kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
        participant_volatile_message_secure_writer_->matched_reader_add_edp(*temp_volatile_reader_proxy_data_);
    }
}

void SecurityManager::unmatch_builtin_endpoints(
        const ParticipantProxyData& participant_data)
{
    uint32_t builtin_endpoints = participant_data.m_available_builtin_endpoints;
    GUID_t tmp_guid;
    tmp_guid.guidPrefix = participant_data.guid.guidPrefix;

    if (participant_stateless_message_reader_ != nullptr &&
            builtin_endpoints & BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_WRITER)
    {
        tmp_guid.entityId = participant_stateless_message_writer_entity_id;
        participant_stateless_message_reader_->matched_writer_remove(tmp_guid, false);
    }

    if (participant_stateless_message_writer_ != nullptr &&
            builtin_endpoints & BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_READER)
    {
        tmp_guid.entityId = participant_stateless_message_reader_entity_id;
        participant_stateless_message_writer_->matched_reader_remove(tmp_guid);
    }

    if (participant_volatile_message_secure_reader_ != nullptr &&
            builtin_endpoints & BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_WRITER)
    {
        tmp_guid.entityId = participant_volatile_message_secure_writer_entity_id;
        participant_volatile_message_secure_reader_->matched_writer_remove(tmp_guid, false);
    }

    if (participant_volatile_message_secure_writer_ != nullptr &&
            builtin_endpoints & BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_READER)
    {
        tmp_guid.entityId = participant_volatile_message_secure_reader_entity_id;
        participant_volatile_message_secure_writer_->matched_reader_remove(tmp_guid);
    }
}

void SecurityManager::exchange_participant_crypto(
        std::shared_ptr<ParticipantCryptoHandle> remote_participant_crypto,
        const GUID_t& remote_participant_guid)
{
    SecurityException exception;

    // Get participant crypto tokens.
    ParticipantCryptoTokenSeq local_participant_crypto_tokens;
    if (crypto_plugin_->cryptokeyexchange()->create_local_participant_crypto_tokens(local_participant_crypto_tokens,
            *local_participant_crypto_handle_, *remote_participant_crypto, exception))
    {
        ParticipantGenericMessage message = generate_participant_crypto_token_message(remote_participant_guid,
                        local_participant_crypto_tokens);

        CacheChange_t* change = create_change_for_message(
            message,
            participant_volatile_message_secure_writer_history_);

        if (change != nullptr)
        {
            // Serialize message
            CDRMessage_t aux_msg(0);
            aux_msg.wraps = true;
            aux_msg.buffer = change->serializedPayload.data;
            aux_msg.length = change->serializedPayload.length;
            aux_msg.max_size = change->serializedPayload.max_size;

            // Serialize encapsulation
            CDRMessage::addOctet(&aux_msg, 0);
            aux_msg.msg_endian = DEFAULT_ENDIAN;
            change->serializedPayload.encapsulation = PL_DEFAULT_ENCAPSULATION;
            CDRMessage::addOctet(&aux_msg, DEFAULT_ENCAPSULATION);
            CDRMessage::addUInt16(&aux_msg, 0);

            if (CDRMessage::addParticipantGenericMessage(&aux_msg, message))
            {
                change->serializedPayload.length = aux_msg.length;

                // Send
                if (!participant_volatile_message_secure_writer_history_->add_change(change))
                {
                    participant_volatile_message_secure_writer_history_->release_change(change);
                    EPROSIMA_LOG_ERROR(SECURITY, "WriterHistory cannot add the CacheChange_t");
                }
            }
            else
            {
                participant_volatile_message_secure_writer_history_->release_change(change);
                EPROSIMA_LOG_ERROR(SECURITY, "Cannot serialize ParticipantGenericMessage");
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(SECURITY, "WriterHistory cannot retrieve a CacheChange_t");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(SECURITY, "Error generating crypto token. (" << exception.what() << ")");
    }
}

// TODO (Ricardo) Change participant_data
std::shared_ptr<ParticipantCryptoHandle> SecurityManager::register_and_match_crypto_endpoint(
        IdentityHandle& remote_participant_identity,
        SecretHandle& shared_secret)
{
    if (crypto_plugin_ == nullptr)
    {
        return nullptr;
    }

    NilHandle nil_handle;
    SecurityException exception;

    // Register remote participant into crypto plugin.
    std::shared_ptr<ParticipantCryptoHandle> remote_participant_crypto =
            crypto_plugin_->cryptokeyfactory()->register_matched_remote_participant(*local_participant_crypto_handle_,
                    remote_participant_identity, nil_handle, shared_secret, exception);

    if (remote_participant_crypto != nullptr)
    {
        return remote_participant_crypto;
    }
    else
    {
        EPROSIMA_LOG_ERROR(SECURITY,
                "Error registering remote participant in cryptography plugin. (" << exception.what() << ")");
    }

    return nullptr;
}

bool SecurityManager::encode_rtps_message(
        const CDRMessage_t& input_message,
        CDRMessage_t& output_message,
        const std::vector<GuidPrefix_t>& receiving_list) const
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    shared_lock<shared_mutex> _(mutex_);

    if (crypto_plugin_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(SECURITY, "Trying to encode rtps message without set cryptography plugin.");
        return false;
    }

    assert(receiving_list.size() > 0);

    std::vector<std::shared_ptr<ParticipantCryptoHandle>> receiving_crypto_list;
    for (const auto& remote_participant : receiving_list)
    {
        const GUID_t remote_participant_key(remote_participant, c_EntityId_RTPSParticipant);

        if (remote_participant_key == participant_->getGuid())
        {
            receiving_crypto_list.push_back(local_participant_crypto_handle_);
        }
        else
        {
            auto dp_it = discovered_participants_.find(remote_participant_key);

            if (dp_it != discovered_participants_.cend() && dp_it->second->get_participant_crypto() != nullptr)
            {
                receiving_crypto_list.push_back(dp_it->second->get_participant_crypto());
            }
            else
            {
                EPROSIMA_LOG_INFO(SECURITY, "Cannot encode message for participant " << remote_participant_key);
            }
        }
    }

    SecurityException exception;
    return crypto_plugin_->cryptotransform()->encode_rtps_message(output_message,
                   input_message, *local_participant_crypto_handle_, receiving_crypto_list,
                   exception);
}

int SecurityManager::decode_rtps_message(
        const CDRMessage_t& message,
        CDRMessage_t& out_message,
        const GuidPrefix_t& remote_participant) const
{
    if (message.buffer[message.pos] != SRTPS_PREFIX)
    {
        return 1;
    }

    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return 0;
    }

    if (crypto_plugin_ == nullptr)
    {
        return 0;
    }

    // Init output buffer
    CDRMessage::initCDRMsg(&out_message);

    std::shared_ptr<const ParticipantCryptoHandle> remote_participant_crypto_handle;

    const GUID_t remote_participant_key(remote_participant, c_EntityId_RTPSParticipant);

    if (remote_participant_key == participant_->getGuid())
    {
        remote_participant_crypto_handle = local_participant_crypto_handle_;
    }
    else
    {
        shared_lock<shared_mutex> _(mutex_);

        auto dp_it = discovered_participants_.find(remote_participant_key);

        if (dp_it != discovered_participants_.end())
        {
            remote_participant_crypto_handle = dp_it->second->get_participant_crypto();
        }
    }

    int returnedValue = -1;

    if (remote_participant_crypto_handle != nullptr)
    {
        SecurityException exception;
        bool ret = crypto_plugin_->cryptotransform()->decode_rtps_message(out_message,
                        message,
                        *local_participant_crypto_handle_,
                        *remote_participant_crypto_handle,
                        exception);

        if (ret)
        {
            returnedValue = 0;
        }
        else
        {
            EPROSIMA_LOG_INFO(SECURITY, "Cannot decode rtps message from participant " << remote_participant_key <<
                    "(" << exception.what() << ")");
        }
    }
    else
    {
        EPROSIMA_LOG_INFO(SECURITY,
                "Cannot decode message. Participant " << remote_participant_key << " is not yet authorized");
    }

    return returnedValue;
}

bool SecurityManager::register_local_writer(
        const GUID_t& writer_guid,
        const PropertyPolicy& writer_properties,
        EndpointSecurityAttributes& security_attributes)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    SecurityException exception;
    bool returned_value = get_datawriter_sec_attributes(writer_properties, security_attributes);

    if (returned_value && crypto_plugin_ != nullptr && (security_attributes.is_submessage_protected ||
            security_attributes.is_payload_protected))
    {
        DatawriterCryptoHandle* writer_handle = crypto_plugin_->cryptokeyfactory()->register_local_datawriter(
            *local_participant_crypto_handle_, writer_properties.properties(), security_attributes, exception);

        if (writer_handle != nullptr && !writer_handle->nil())
        {
            std::lock_guard<shared_mutex> _(mutex_);
            writer_handles_.emplace(writer_guid, writer_handle);
        }
        else
        {
            EPROSIMA_LOG_ERROR(SECURITY, "Cannot register local writer in crypto plugin. (" << exception.what() << ")");
            returned_value = false;
        }
    }

    return returned_value;
}

bool SecurityManager::get_datawriter_sec_attributes(
        const PropertyPolicy& writer_properties,
        EndpointSecurityAttributes& security_attributes)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    bool returned_value = true;
    SecurityException exception;

    if (access_plugin_ != nullptr)
    {
        // Extract topic and partitions.
        std::string topic_name, partitions_str;
        std::vector<std::string> partitions;
        const std::string* property_value = PropertyPolicyHelper::find_property(writer_properties,
                        "topic_name");

        if (property_value != nullptr)
        {
            topic_name = *property_value;
        }

        property_value = PropertyPolicyHelper::find_property(writer_properties,
                        "partitions");

        if (property_value != nullptr)
        {
            partitions_str = *property_value;

            // Extract partitions.
            std::size_t initial_pos = 0, last_pos = partitions_str.find_first_of(';');
            while (last_pos != std::string::npos)
            {
                partitions.emplace_back(partitions_str.begin() + initial_pos,
                        partitions_str.begin() + last_pos);
                initial_pos = last_pos + 1;
                last_pos = partitions_str.find_first_of(';', last_pos + 1);
            }
            partitions.emplace_back(partitions_str.begin() + initial_pos, partitions_str.end());
        }

        if (!topic_name.empty())
        {
            if (access_plugin_->check_create_datawriter(*local_permissions_handle_,
                    domain_id_, topic_name, partitions, exception))
            {
                if ((returned_value = access_plugin_->get_datawriter_sec_attributes(*local_permissions_handle_,
                        topic_name, partitions, security_attributes, exception)) == false)
                {
                    EPROSIMA_LOG_ERROR(SECURITY, "Error getting security attributes of local writer " <<
                            " (" << exception.what() << ")" << std::endl);
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(SECURITY, "Error checking creation of local writer " <<
                        " (" << exception.what() << ")" << std::endl);
                returned_value = false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(SECURITY, "Error. No topic_name." << std::endl);
            returned_value = false;
        }
    }

    return returned_value;
}

bool SecurityManager::register_local_builtin_writer(
        const GUID_t& writer_guid,
        EndpointSecurityAttributes& security_attributes)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    bool returned_value = true;
    SecurityException exception;

    if (crypto_plugin_ != nullptr && security_attributes.is_submessage_protected &&
            writer_guid.entityId != participant_volatile_message_secure_writer_entity_id)
    {
        PropertySeq auxProps;
        DatawriterCryptoHandle* writer_handle = crypto_plugin_->cryptokeyfactory()->register_local_datawriter(
            *local_participant_crypto_handle_, auxProps, security_attributes, exception);

        if (writer_handle != nullptr && !writer_handle->nil())
        {
            std::lock_guard<shared_mutex> _(mutex_);
            writer_handles_.emplace(writer_guid, writer_handle);
        }
        else
        {
            EPROSIMA_LOG_ERROR(SECURITY, "Cannot register local writer in crypto plugin. (" << exception.what() << ")");
            returned_value = false;
        }
    }

    return returned_value;
}

bool SecurityManager::unregister_local_writer(
        const GUID_t& writer_guid)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    if (crypto_plugin_ == nullptr)
    {
        return false;
    }

    std::lock_guard<shared_mutex> _(mutex_);

    auto local_writer = writer_handles_.find(writer_guid);

    if (local_writer != writer_handles_.end())
    {
        SecurityException exception;

        for (auto& rit : local_writer->second.associated_readers)
        {
            crypto_plugin_->cryptokeyfactory()->unregister_datareader(std::get<1>(rit.second), exception);
        }

        crypto_plugin_->cryptokeyfactory()->unregister_datawriter(local_writer->second.writer_handle,
                exception);
        writer_handles_.erase(local_writer);

        return true;
    }

    return false;
}

bool SecurityManager::register_local_reader(
        const GUID_t& reader_guid,
        const PropertyPolicy& reader_properties,
        EndpointSecurityAttributes& security_attributes)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    SecurityException exception;
    bool returned_value = get_datareader_sec_attributes(reader_properties, security_attributes);

    if (returned_value && crypto_plugin_ != nullptr && (security_attributes.is_submessage_protected ||
            security_attributes.is_payload_protected))
    {

        DatareaderCryptoHandle* reader_handle = crypto_plugin_->cryptokeyfactory()->register_local_datareader(
            *local_participant_crypto_handle_, reader_properties.properties(), security_attributes, exception);

        if (reader_handle != nullptr && !reader_handle->nil())
        {
            std::lock_guard<shared_mutex> _(mutex_);
            reader_handles_.emplace(reader_guid, reader_handle);
        }
        else
        {
            EPROSIMA_LOG_ERROR(SECURITY, "Cannot register local reader in crypto plugin. (" << exception.what() << ")");
            returned_value = false;
        }
    }

    return returned_value;
}

bool SecurityManager::get_datareader_sec_attributes(
        const PropertyPolicy& reader_properties,
        EndpointSecurityAttributes& security_attributes)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    bool returned_value = true;
    SecurityException exception;

    if (access_plugin_ != nullptr)
    {
        // Extract topic and partitions.
        std::string topic_name, partitions_str;
        std::vector<std::string> partitions;
        const std::string* property_value = PropertyPolicyHelper::find_property(reader_properties,
                        "topic_name");

        if (property_value != nullptr)
        {
            topic_name = *property_value;
        }

        property_value = PropertyPolicyHelper::find_property(reader_properties,
                        "partitions");

        if (property_value != nullptr)
        {
            partitions_str = *property_value;

            // Extract partitions.
            std::size_t initial_pos = 0, last_pos = partitions_str.find_first_of(';');
            while (last_pos != std::string::npos)
            {
                partitions.emplace_back(partitions_str.begin() + initial_pos,
                        partitions_str.begin() + last_pos);
                initial_pos = last_pos + 1;
                last_pos = partitions_str.find_first_of(';', last_pos + 1);
            }
            partitions.emplace_back(partitions_str.begin() + initial_pos, partitions_str.end());
        }

        if (!topic_name.empty())
        {
            if (access_plugin_->check_create_datareader( *local_permissions_handle_,
                    domain_id_, topic_name, partitions, exception))
            {
                if ((returned_value = access_plugin_->get_datareader_sec_attributes(*local_permissions_handle_,
                        topic_name, partitions, security_attributes, exception)) == false)
                {
                    EPROSIMA_LOG_ERROR(SECURITY, "Error getting security attributes of local reader " <<
                            " (" << exception.what() << ")" << std::endl);
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(SECURITY, "Error checking creation of local reader " <<
                        " (" << exception.what() << ")" << std::endl);
                returned_value = false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(SECURITY, "Error. No topic_name." << std::endl);
            returned_value = false;
        }
    }

    return returned_value;
}

bool SecurityManager::register_local_builtin_reader(
        const GUID_t& reader_guid,
        EndpointSecurityAttributes& security_attributes)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    bool returned_value = true;
    SecurityException exception;

    if (crypto_plugin_ != nullptr && security_attributes.is_submessage_protected &&
            reader_guid.entityId != participant_volatile_message_secure_reader_entity_id)
    {
        PropertySeq auxProps;
        DatareaderCryptoHandle* reader_handle = crypto_plugin_->cryptokeyfactory()->register_local_datareader(
            *local_participant_crypto_handle_, auxProps, security_attributes, exception);

        if (reader_handle != nullptr && !reader_handle->nil())
        {
            std::lock_guard<shared_mutex> _(mutex_);
            reader_handles_.emplace(reader_guid, reader_handle);
        }
        else
        {
            EPROSIMA_LOG_ERROR(SECURITY, "Cannot register local reader in crypto plugin. (" << exception.what() << ")");
            returned_value = false;
        }
    }

    return returned_value;
}

bool SecurityManager::unregister_local_reader(
        const GUID_t& reader_guid)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    if (crypto_plugin_ == nullptr)
    {
        return false;
    }

    std::lock_guard<shared_mutex> _(mutex_);

    auto local_reader = reader_handles_.find(reader_guid);

    if (local_reader != reader_handles_.end())
    {
        SecurityException exception;

        for (auto& wit : local_reader->second.associated_writers)
        {
            crypto_plugin_->cryptokeyfactory()->unregister_datawriter(std::get<1>(wit.second), exception);
        }

        crypto_plugin_->cryptokeyfactory()->unregister_datareader(local_reader->second.reader_handle, exception);
        reader_handles_.erase(local_reader);

        return true;
    }

    return false;
}

bool SecurityManager::discovered_reader(
        const GUID_t& writer_guid,
        const GUID_t& remote_participant_key,
        ReaderProxyData& remote_reader_data,
        const EndpointSecurityAttributes& security_attributes)
{
    return discovered_reader(writer_guid, remote_participant_key, remote_reader_data, security_attributes, false);
}

void SecurityManager::remove_reader(
        const GUID_t& writer_guid,
        const GUID_t& /*remote_participant_key*/,
        const GUID_t& remote_reader_guid)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return;
    }

    if (crypto_plugin_ == nullptr)
    {
        return;
    }

    std::lock_guard<shared_mutex> _(mutex_);

    auto local_writer = writer_handles_.find(writer_guid);

    if (local_writer != writer_handles_.end())
    {
        SecurityException exception;

        auto rit = local_writer->second.associated_readers.find(remote_reader_guid);

        if (rit != local_writer->second.associated_readers.end())
        {
            crypto_plugin_->cryptokeyfactory()->unregister_datareader(std::get<1>(rit->second), exception);
            local_writer->second.associated_readers.erase(rit);
        }
        else
        {
            EPROSIMA_LOG_INFO(SECURITY, "Cannot find remote reader " << remote_reader_guid << std::endl);
        }
    }
}

bool SecurityManager::discovered_builtin_reader(
        const GUID_t& writer_guid,
        const GUID_t& remote_participant_key,
        ReaderProxyData& remote_reader_data,
        const EndpointSecurityAttributes& security_attributes)
{
    return discovered_reader(writer_guid, remote_participant_key, remote_reader_data, security_attributes, true);
}

bool SecurityManager::discovered_reader(
        const GUID_t& writer_guid,
        const GUID_t& remote_participant_key,
        ReaderProxyData& remote_reader_data,
        const EndpointSecurityAttributes& security_attributes,
        bool is_builtin)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    std::unique_lock<shared_mutex> lock(mutex_);

    PermissionsHandle* remote_permissions = nullptr;
    std::shared_ptr<ParticipantCryptoHandle> remote_participant_crypto_handle;
    std::shared_ptr<SecretHandle> shared_secret_handle;
    AuthenticationStatus auth_status(AUTHENTICATION_INIT);

    if (!security_attributes.match(remote_reader_data.security_attributes_,
            remote_reader_data.plugin_security_attributes_))
    {
        return false;
    }

    if (remote_participant_key == participant_->getGuid())
    {
        remote_participant_crypto_handle = local_participant_crypto_handle_;
    }
    else
    {
        auto dp_it = discovered_participants_.find(remote_participant_key);

        if (dp_it != discovered_participants_.end())
        {
            remote_permissions = dp_it->second->get_permissions_handle();
            remote_participant_crypto_handle = dp_it->second->get_participant_crypto();
            shared_secret_handle = dp_it->second->get_shared_secret();
            auth_status = dp_it->second->get_auth_status();
        }
    }

    // assert(access_plugin_ == nullptr || remote_permissions != nullptr);
    // assert(crypto_plugin_ == nullptr || remote_participant_crypto_handle != nullptr);

    bool relay_only = false;
    bool returned_value = true;
    SecurityException exception;

    if (!is_builtin)
    {
        //! Check if it is an unathenticated participant
        if (participant_->security_attributes().allow_unauthenticated_participants &&
                auth_status != AUTHENTICATION_NOT_AVAILABLE && auth_status != AUTHENTICATION_OK &&
                (security_attributes.is_write_protected || security_attributes.is_read_protected))
        {
            //!Do not match if read or write protection is enabled for this local endpoint
            return false;
        }

        if (access_plugin_ != nullptr && remote_permissions != nullptr)
        {
            if ((returned_value = access_plugin_->check_remote_datareader(
                        *remote_permissions, domain_id_, remote_reader_data, relay_only, exception)) == false)
            {
                EPROSIMA_LOG_ERROR(SECURITY, "Error checking create remote reader " << remote_reader_data.guid
                                                                                    << " (" << exception.what() << ")");
            }
        }
    }

    if (returned_value && crypto_plugin_ != nullptr && (security_attributes.is_submessage_protected ||
            security_attributes.is_payload_protected))
    {
        bool is_key_exchange = (remote_reader_data.guid.entityId
                == participant_volatile_message_secure_reader_entity_id);
        auto local_writer = writer_handles_.find(writer_guid);
        returned_value = false;

        if (local_writer != writer_handles_.end())
        {
            if (remote_participant_crypto_handle != nullptr)
            {
                DatareaderCryptoHandle* remote_reader_handle =
                        crypto_plugin_->cryptokeyfactory()->register_matched_remote_datareader(
                    *local_writer->second.writer_handle, *remote_participant_crypto_handle,
                    *shared_secret_handle, relay_only, exception);

                if (remote_reader_handle != nullptr && !remote_reader_handle->nil())
                {
                    if (is_key_exchange)
                    {
                        EPROSIMA_LOG_INFO(SECURITY,
                                "Process successful discovering local reader " << remote_reader_data.guid);
                        local_writer->second.associated_readers.emplace(remote_reader_data.guid,
                                std::make_tuple(remote_reader_data, remote_reader_handle));
                        lock.unlock();
                        participant_->pairing_remote_reader_with_local_writer_after_security(
                            writer_guid, remote_reader_data);
                    }
                    else
                    {
                        // Check pending reader crypto messages.
                        auto pending = remote_reader_pending_messages_.find(
                            std::make_pair(remote_reader_data.guid, writer_guid));
                        bool pairing_cause_pending_message = false;

                        if (pending != remote_reader_pending_messages_.end())
                        {
                            if (crypto_plugin_->cryptokeyexchange()->set_remote_datareader_crypto_tokens(
                                        *local_writer->second.writer_handle,
                                        *remote_reader_handle,
                                        pending->second,
                                        exception))
                            {
                                pairing_cause_pending_message = true;
                            }
                            else
                            {
                                EPROSIMA_LOG_ERROR(SECURITY, "Cannot set remote reader crypto tokens ("
                                        << remote_reader_data.guid << ") - (" << exception.what() << ")");
                            }

                            remote_reader_pending_messages_.erase(pending);
                        }

                        GUID_t local_reader_guid;
                        WriterProxyData* writer_data = nullptr;

                        // Get local writer crypto tokens.
                        DatawriterCryptoTokenSeq local_writer_crypto_tokens;
                        if (crypto_plugin_->cryptokeyexchange()->create_local_datawriter_crypto_tokens(
                                    local_writer_crypto_tokens,
                                    *local_writer->second.writer_handle,
                                    *remote_reader_handle, exception))
                        {
                            if (remote_participant_key == participant_->getGuid())
                            {
                                EPROSIMA_LOG_INFO(SECURITY, "Process successful discovering local reader "
                                        << remote_reader_data.guid);
                                local_writer->second.associated_readers.emplace(remote_reader_data.guid,
                                        std::make_tuple(remote_reader_data, remote_reader_handle));

                                // Search local reader.
                                auto local_reader = reader_handles_.find(remote_reader_data.guid);

                                if (local_reader != reader_handles_.end())
                                {
                                    returned_value = true;
                                    auto remote_writer = local_reader->second.associated_writers.find(writer_guid);

                                    if (remote_writer != local_reader->second.associated_writers.end())
                                    {
                                        if (crypto_plugin_->cryptokeyexchange()->set_remote_datawriter_crypto_tokens(
                                                    *local_reader->second.reader_handle,
                                                    *std::get<1>(remote_writer->second),
                                                    local_writer_crypto_tokens,
                                                    exception))
                                        {
                                            local_reader_guid = local_reader->first;
                                            writer_data = &(std::get<0>(remote_writer->second));
                                        }
                                        else
                                        {
                                            EPROSIMA_LOG_ERROR(SECURITY, "Cannot set local reader crypto tokens ("
                                                    << remote_reader_data.guid << ") - (" << exception.what() << ")");
                                        }
                                    }
                                    else
                                    {
                                        // Store in pendings.
                                        remote_writer_pending_messages_.emplace(
                                            std::make_pair(writer_guid, local_reader->first),
                                            std::move(local_writer_crypto_tokens));
                                    }
                                }
                                else
                                {
                                    EPROSIMA_LOG_ERROR(SECURITY, "Cannot find local reader ("
                                            << remote_reader_data.guid << ") - (" << exception.what() << ")");
                                }
                            }
                            else
                            {
                                ParticipantGenericMessage message =
                                        generate_writer_crypto_token_message(remote_participant_key,
                                                remote_reader_data.guid, writer_guid, local_writer_crypto_tokens);

                                local_writer->second.associated_readers.emplace(remote_reader_data.guid,
                                        std::make_tuple(remote_reader_data, remote_reader_handle));
                                lock.unlock();

                                CacheChange_t* change = create_change_for_message(
                                    message,
                                    participant_volatile_message_secure_writer_history_);

                                if (change != nullptr)
                                {
                                    // Serialize message
                                    CDRMessage_t aux_msg(0);
                                    aux_msg.wraps = true;
                                    aux_msg.buffer = change->serializedPayload.data;
                                    aux_msg.length = change->serializedPayload.length;
                                    aux_msg.max_size = change->serializedPayload.max_size;

                                    // Serialize encapsulation
                                    CDRMessage::addOctet(&aux_msg, 0);
                                    aux_msg.msg_endian = DEFAULT_ENDIAN;
                                    change->serializedPayload.encapsulation = PL_DEFAULT_ENCAPSULATION;
                                    CDRMessage::addOctet(&aux_msg, DEFAULT_ENCAPSULATION);
                                    CDRMessage::addUInt16(&aux_msg, 0);

                                    if (CDRMessage::addParticipantGenericMessage(&aux_msg, message))
                                    {
                                        change->serializedPayload.length = aux_msg.length;

                                        // Send
                                        if (participant_volatile_message_secure_writer_history_->add_change(change))
                                        {
                                            EPROSIMA_LOG_INFO(SECURITY, "Process successful discovering remote reader "
                                                    << remote_reader_data.guid);
                                            returned_value = true;
                                        }
                                        else
                                        {
                                            participant_volatile_message_secure_writer_history_->release_change(change);
                                            EPROSIMA_LOG_ERROR(SECURITY, "WriterHistory cannot add the CacheChange_t");
                                        }
                                    }
                                    else
                                    {
                                        participant_volatile_message_secure_writer_history_->release_change(change);
                                        EPROSIMA_LOG_ERROR(SECURITY, "Cannot serialize ParticipantGenericMessage");
                                    }
                                }
                                else
                                {
                                    EPROSIMA_LOG_ERROR(SECURITY, "WriterHistory cannot retrieve a CacheChange_t");
                                }
                            }
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(SECURITY, "Error generating crypto token. (" << exception.what() << ")");
                        }

                        // Here the mutex has to be unlock.
                        if (lock)
                        {
                            lock.unlock();
                        }

                        // If reader was found and setting of crypto tokens works,
                        // then tell core to match reader and writer.
                        if (local_reader_guid != GUID_t::unknown())
                        {
                            participant_->pairing_remote_writer_with_local_reader_after_security(
                                local_reader_guid, *writer_data);
                        }

                        // If writer was found and setting of crypto tokens works,
                        // then tell core to match writer and reader.
                        if (pairing_cause_pending_message)
                        {
                            participant_->pairing_remote_reader_with_local_writer_after_security(
                                writer_guid, remote_reader_data);
                        }
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(SECURITY,
                            "Crypto plugin fails registering remote reader " << remote_reader_data.guid <<
                            " of participant " << remote_participant_key);
                }
            }
            else
            {
                EPROSIMA_LOG_INFO(SECURITY, "Storing remote reader << " << remote_reader_data.guid <<
                        " of participant " << remote_participant_key << " on pendings");

                remote_reader_pending_discovery_messages_.push_back(std::make_tuple(remote_reader_data,
                        remote_participant_key, writer_guid));

                returned_value = true;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(SECURITY, "Cannot find local writer " << writer_guid << std::endl);
        }
    }
    else if (returned_value)
    {
        lock.unlock();
        participant_->pairing_remote_reader_with_local_writer_after_security(
            writer_guid, remote_reader_data);
    }

    return returned_value;
}

bool SecurityManager::discovered_writer(
        const GUID_t& reader_guid,
        const GUID_t& remote_participant_key,
        WriterProxyData& remote_writer_data,
        const EndpointSecurityAttributes& security_attributes)
{
    return discovered_writer(reader_guid, remote_participant_key, remote_writer_data, security_attributes, false);
}

void SecurityManager::remove_writer(
        const GUID_t& reader_guid,
        const GUID_t& /*remote_participant_key*/,
        const GUID_t& remote_writer_guid)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return;
    }

    if (crypto_plugin_ == nullptr)
    {
        return;
    }

    std::lock_guard<shared_mutex> _(mutex_);

    auto local_reader = reader_handles_.find(reader_guid);

    if (local_reader != reader_handles_.end())
    {
        SecurityException exception;

        auto wit = local_reader->second.associated_writers.find(remote_writer_guid);

        if (wit != local_reader->second.associated_writers.end())
        {
            crypto_plugin_->cryptokeyfactory()->unregister_datawriter(std::get<1>(wit->second), exception);
            local_reader->second.associated_writers.erase(wit);
        }
        else
        {
            EPROSIMA_LOG_INFO(SECURITY, "Cannot find remote writer " << remote_writer_guid << std::endl);
        }
    }
}

bool SecurityManager::discovered_builtin_writer(
        const GUID_t& reader_guid,
        const GUID_t& remote_participant_key,
        WriterProxyData& remote_writer_data,
        const EndpointSecurityAttributes& security_attributes)
{
    return discovered_writer(reader_guid, remote_participant_key, remote_writer_data, security_attributes, true);
}

bool SecurityManager::discovered_writer(
        const GUID_t& reader_guid,
        const GUID_t& remote_participant_key,
        WriterProxyData& remote_writer_data,
        const EndpointSecurityAttributes& security_attributes,
        bool is_builtin)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    std::unique_lock<shared_mutex> lock(mutex_);

    PermissionsHandle* remote_permissions = nullptr;
    std::shared_ptr<ParticipantCryptoHandle> remote_participant_crypto_handle;
    std::shared_ptr<SecretHandle> shared_secret_handle;
    AuthenticationStatus auth_status(AUTHENTICATION_INIT);

    if (!security_attributes.match(remote_writer_data.security_attributes_,
            remote_writer_data.plugin_security_attributes_))
    {
        return false;
    }

    if (remote_participant_key == participant_->getGuid())
    {
        remote_participant_crypto_handle = local_participant_crypto_handle_;
    }
    else
    {
        auto dp_it = discovered_participants_.find(remote_participant_key);

        if (dp_it != discovered_participants_.end())
        {
            remote_permissions = dp_it->second->get_permissions_handle();
            remote_participant_crypto_handle = dp_it->second->get_participant_crypto();
            shared_secret_handle = dp_it->second->get_shared_secret();
            auth_status = dp_it->second->get_auth_status();
        }
    }

    // assert(access_plugin_ == nullptr || remote_permissions != nullptr);
    // assert(crypto_plugin_ == nullptr || remote_participant_crypto_handle != nullptr);

    bool returned_value = true;
    SecurityException exception;

    if (!is_builtin)
    {
        //! Check if it is an unathenticated participant
        if (participant_->security_attributes().allow_unauthenticated_participants &&
                auth_status != AUTHENTICATION_NOT_AVAILABLE && auth_status != AUTHENTICATION_OK &&
                (security_attributes.is_write_protected || security_attributes.is_read_protected))
        {
            //!Do not match if read or write protection is enabled for this local endpoint
            return false;
        }

        if (access_plugin_ != nullptr && remote_permissions != nullptr)
        {
            if ((returned_value = access_plugin_->check_remote_datawriter(
                        *remote_permissions, domain_id_, remote_writer_data, exception)) == false)
            {
                EPROSIMA_LOG_ERROR(SECURITY, "Error checking create remote writer " << remote_writer_data.guid
                                                                                    << " (" << exception.what() << ")");
            }
        }
    }

    if (returned_value && crypto_plugin_ != nullptr && (security_attributes.is_submessage_protected ||
            security_attributes.is_payload_protected))
    {
        bool is_key_exchange = (remote_writer_data.guid.entityId
                == participant_volatile_message_secure_writer_entity_id);
        auto local_reader = reader_handles_.find(reader_guid);
        returned_value = false;

        if (local_reader != reader_handles_.end())
        {
            if (remote_participant_crypto_handle != nullptr)
            {
                DatawriterCryptoHandle* remote_writer_handle =
                        crypto_plugin_->cryptokeyfactory()->register_matched_remote_datawriter(
                    *local_reader->second.reader_handle, *remote_participant_crypto_handle,
                    *shared_secret_handle, exception);

                if (remote_writer_handle != nullptr && !remote_writer_handle->nil())
                {
                    if (is_key_exchange)
                    {
                        EPROSIMA_LOG_INFO(SECURITY,
                                "Process successful discovering local writer " << remote_writer_data.guid);
                        local_reader->second.associated_writers.emplace(remote_writer_data.guid,
                                std::make_tuple(remote_writer_data, remote_writer_handle));
                        lock.unlock();
                        participant_->pairing_remote_writer_with_local_reader_after_security(
                            reader_guid, remote_writer_data);
                    }
                    else
                    {
                        // Check pending writer crypto messages.
                        auto pending = remote_writer_pending_messages_.find(
                            std::make_pair(remote_writer_data.guid, reader_guid));
                        bool pairing_cause_pending_message = false;

                        if (pending != remote_writer_pending_messages_.end())
                        {
                            if (crypto_plugin_->cryptokeyexchange()->set_remote_datawriter_crypto_tokens(
                                        *local_reader->second.reader_handle,
                                        *remote_writer_handle,
                                        pending->second,
                                        exception))
                            {
                                pairing_cause_pending_message = true;
                            }
                            else
                            {
                                EPROSIMA_LOG_ERROR(SECURITY, "Cannot set remote writer crypto tokens ("
                                        << remote_writer_data.guid << ") - (" << exception.what() << ")");
                            }

                            remote_writer_pending_messages_.erase(pending);
                        }

                        GUID_t local_writer_guid;
                        ReaderProxyData* reader_data = nullptr;

                        // Get local reader crypto tokens.
                        DatareaderCryptoTokenSeq local_reader_crypto_tokens;
                        if (crypto_plugin_->cryptokeyexchange()->create_local_datareader_crypto_tokens(
                                    local_reader_crypto_tokens,
                                    *local_reader->second.reader_handle,
                                    *remote_writer_handle,
                                    exception))
                        {
                            if (remote_participant_key == participant_->getGuid())
                            {
                                EPROSIMA_LOG_INFO(SECURITY, "Process successful discovering local writer "
                                        << remote_writer_data.guid);
                                local_reader->second.associated_writers.emplace(remote_writer_data.guid,
                                        std::make_tuple(remote_writer_data, remote_writer_handle));

                                // Search local writer.
                                auto local_writer = writer_handles_.find(remote_writer_data.guid);

                                if (local_writer != writer_handles_.end())
                                {
                                    returned_value = true;
                                    auto remote_reader = local_writer->second.associated_readers.find(reader_guid);

                                    if (remote_reader != local_writer->second.associated_readers.end())
                                    {
                                        if (crypto_plugin_->cryptokeyexchange()->set_remote_datareader_crypto_tokens(
                                                    *local_writer->second.writer_handle,
                                                    *std::get<1>(remote_reader->second),
                                                    local_reader_crypto_tokens,
                                                    exception))
                                        {
                                            local_writer_guid = local_writer->first;
                                            reader_data = &(std::get<0>(remote_reader->second));
                                        }
                                        else
                                        {
                                            EPROSIMA_LOG_ERROR(SECURITY, "Cannot set local writer crypto tokens ("
                                                    << remote_writer_data.guid << ") - (" << exception.what() << ")");
                                        }
                                    }
                                    else
                                    {
                                        // Store in pendings.
                                        remote_reader_pending_messages_.emplace(
                                            std::make_pair(reader_guid, local_writer->first),
                                            std::move(local_reader_crypto_tokens));
                                    }
                                }
                                else
                                {
                                    EPROSIMA_LOG_ERROR(SECURITY, "Cannot find local writer ("
                                            << remote_writer_data.guid << ") - (" << exception.what() << ")");
                                }
                            }
                            else
                            {
                                ParticipantGenericMessage message =
                                        generate_reader_crypto_token_message(remote_participant_key,
                                                remote_writer_data.guid, reader_guid, local_reader_crypto_tokens);

                                local_reader->second.associated_writers.emplace(remote_writer_data.guid,
                                        std::make_tuple(remote_writer_data, remote_writer_handle));
                                lock.unlock();

                                CacheChange_t* change = create_change_for_message(
                                    message,
                                    participant_volatile_message_secure_writer_history_);

                                if (change != nullptr)
                                {
                                    // Serialize message
                                    CDRMessage_t aux_msg(0);
                                    aux_msg.wraps = true;
                                    aux_msg.buffer = change->serializedPayload.data;
                                    aux_msg.length = change->serializedPayload.length;
                                    aux_msg.max_size = change->serializedPayload.max_size;

                                    // Serialize encapsulation
                                    CDRMessage::addOctet(&aux_msg, 0);
                                    aux_msg.msg_endian = DEFAULT_ENDIAN;
                                    change->serializedPayload.encapsulation = PL_DEFAULT_ENCAPSULATION;
                                    CDRMessage::addOctet(&aux_msg, DEFAULT_ENCAPSULATION);
                                    CDRMessage::addUInt16(&aux_msg, 0);

                                    if (CDRMessage::addParticipantGenericMessage(&aux_msg, message))
                                    {
                                        change->serializedPayload.length = aux_msg.length;

                                        // Send
                                        if (participant_volatile_message_secure_writer_history_->add_change(change))
                                        {
                                            EPROSIMA_LOG_INFO(SECURITY, "Process successful discovering remote writer "
                                                    << remote_writer_data.guid);
                                            returned_value = true;
                                        }
                                        else
                                        {
                                            participant_volatile_message_secure_writer_history_->release_change(change);
                                            EPROSIMA_LOG_ERROR(SECURITY, "WriterHistory cannot add the CacheChange_t");
                                        }
                                    }
                                    else
                                    {
                                        participant_volatile_message_secure_writer_history_->release_change(change);
                                        EPROSIMA_LOG_ERROR(SECURITY, "Cannot serialize ParticipantGenericMessage");
                                    }
                                }
                                else
                                {
                                    EPROSIMA_LOG_ERROR(SECURITY, "WriterHistory cannot retrieve a CacheChange_t");
                                }

                            }
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(SECURITY, "Error generating crypto token. (" << exception.what() << ")");
                        }

                        // Here the mutex has to be unlock.
                        if (lock)
                        {
                            lock.unlock();
                        }

                        // If writer was found and setting of crypto tokens works,
                        // then tell core to match writer and reader.
                        if (local_writer_guid != GUID_t::unknown())
                        {
                            participant_->pairing_remote_reader_with_local_writer_after_security(
                                local_writer_guid, *reader_data);
                        }

                        // If reader was found and setting of crypto tokens works,
                        // then tell core to match reader and writer.
                        if (pairing_cause_pending_message)
                        {
                            participant_->pairing_remote_writer_with_local_reader_after_security(
                                reader_guid, remote_writer_data);
                        }
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(SECURITY,
                            "Crypto plugin fails registering remote writer " << remote_writer_data.guid <<
                            " of participant " << remote_participant_key);
                }
            }
            else
            {
                EPROSIMA_LOG_INFO(SECURITY, "Storing remote writer << " << remote_writer_data.guid <<
                        " of participant " << remote_participant_key << "on pendings");

                remote_writer_pending_discovery_messages_.push_back(std::make_tuple(remote_writer_data,
                        remote_participant_key, reader_guid));

                returned_value = true;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(SECURITY, "Cannot find local reader " << reader_guid << std::endl);
        }
    }
    else if (returned_value)
    {
        lock.unlock();
        participant_->pairing_remote_writer_with_local_reader_after_security(
            reader_guid, remote_writer_data);
    }

    return returned_value;
}

bool SecurityManager::encode_writer_submessage(
        const CDRMessage_t& input_message,
        CDRMessage_t& output_message,
        const GUID_t& writer_guid,
        const std::vector<GUID_t>& receiving_list) const
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    if (crypto_plugin_ == nullptr)
    {
        return false;
    }

    shared_lock<shared_mutex> _(mutex_);

    if (writer_guid.entityId == participant_volatile_message_secure_writer_entity_id)
    {
        bool ret_val = false;
        if (receiving_list.size() == 1)
        {
            GUID_t participant_guid(receiving_list.at(0).guidPrefix, c_EntityId_RTPSParticipant);
            auto part = discovered_participants_.find(participant_guid);
            if (part != discovered_participants_.end())
            {
                auto pCrypto = part->second->get_participant_crypto();

                if (pCrypto)
                {
                    EndpointSecurityAttributes attr;
                    SecurityException exception;
                    PropertySeq auxProps;
                    auxProps.emplace_back(
                        Property("dds.sec.builtin_endpoint_name", "BuiltinParticipantVolatileMessageSecureWriter"));
                    auto wHandle =
                            crypto_plugin_->cryptokeyfactory()->register_local_datawriter(
                        *pCrypto, auxProps, attr, exception);
                    std::vector<std::shared_ptr<DatareaderCryptoHandle>> receiving_crypto_list;
                    if (wHandle != nullptr)
                    {
                        ret_val = crypto_plugin_->cryptotransform()->encode_datawriter_submessage(output_message,
                                        input_message, *wHandle, receiving_crypto_list, exception);
                    }
                }
            }
        }

        return ret_val;
    }

    const auto& wr_it = writer_handles_.find(writer_guid);

    if (wr_it != writer_handles_.end())
    {
        std::vector<std::shared_ptr<DatareaderCryptoHandle>> receiving_datareader_crypto_list;

        for (const auto& rd_it : receiving_list)
        {
            const auto rd_it_handle = wr_it->second.associated_readers.find(rd_it);

            if (rd_it_handle != wr_it->second.associated_readers.end())
            {
                receiving_datareader_crypto_list.emplace_back(std::get<1>(rd_it_handle->second)->shared_from_this());
            }
            else
            {
                EPROSIMA_LOG_ERROR(SECURITY, "Cannot find remote reader " << rd_it);
            }
        }

        if (receiving_datareader_crypto_list.size() > 0)
        {
            SecurityException exception;

            if (crypto_plugin_->cryptotransform()->encode_datawriter_submessage(output_message,
                    input_message,
                    *wr_it->second.writer_handle,
                    receiving_datareader_crypto_list,
                    exception))
            {
                return true;
            }
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(SECURITY, "Cannot find local writer " << writer_guid);
    }

    return false;
}

bool SecurityManager::encode_reader_submessage(
        const CDRMessage_t& input_message,
        CDRMessage_t& output_message,
        const GUID_t& reader_guid,
        const std::vector<GUID_t>& receiving_list) const
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    if (crypto_plugin_ == nullptr)
    {
        return false;
    }

    shared_lock<shared_mutex> _(mutex_);

    if (reader_guid.entityId == participant_volatile_message_secure_reader_entity_id)
    {
        bool ret_val = false;

        if (receiving_list.size() == 1)
        {
            GUID_t participant_guid(receiving_list.at(0).guidPrefix, c_EntityId_RTPSParticipant);
            auto part = discovered_participants_.find(participant_guid);
            if (part != discovered_participants_.end())
            {
                auto pCrypto = part->second->get_participant_crypto();

                if (pCrypto)
                {
                    EndpointSecurityAttributes attr;
                    SecurityException exception;
                    PropertySeq auxProps;
                    auxProps.emplace_back(
                        Property("dds.sec.builtin_endpoint_name", "BuiltinParticipantVolatileMessageSecureReader"));
                    auto rHandle =
                            crypto_plugin_->cryptokeyfactory()->register_local_datareader(
                        *pCrypto, auxProps, attr, exception);
                    std::vector<std::shared_ptr<DatawriterCryptoHandle>> receiving_crypto_list;
                    if (rHandle != nullptr)
                    {
                        ret_val = crypto_plugin_->cryptotransform()->encode_datareader_submessage(output_message,
                                        input_message, *rHandle, receiving_crypto_list, exception);
                    }
                }
            }
        }

        return ret_val;
    }

    const auto& rd_it = reader_handles_.find(reader_guid);

    if (rd_it != reader_handles_.end())
    {
        std::vector<std::shared_ptr<DatawriterCryptoHandle>> receiving_datawriter_crypto_list;

        for (const auto& wr_it : receiving_list)
        {
            const auto wr_it_handle = rd_it->second.associated_writers.find(wr_it);

            if (wr_it_handle != rd_it->second.associated_writers.end())
            {
                receiving_datawriter_crypto_list.push_back(std::get<1>(wr_it_handle->second)->shared_from_this());
            }
            else
            {
                EPROSIMA_LOG_ERROR(SECURITY, "Cannot find remote writer " << wr_it);
            }
        }

        if (receiving_datawriter_crypto_list.size() > 0)
        {
            SecurityException exception;

            if (crypto_plugin_->cryptotransform()->encode_datareader_submessage(output_message,
                    input_message,
                    *rd_it->second.reader_handle,
                    receiving_datawriter_crypto_list,
                    exception))
            {
                return true;
            }
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(SECURITY, "Cannot find local reader " << reader_guid);
    }

    return false;
}

int SecurityManager::decode_rtps_submessage(
        CDRMessage_t& message,
        CDRMessage_t& out_message,
        const GuidPrefix_t& sending_participant) const
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return -1;
    }

    if (message.buffer[message.pos] != SEC_PREFIX)
    {
        return 1;
    }

    if (crypto_plugin_ == nullptr)
    {
        return -1;
    }

    CDRMessage::initCDRMsg(&out_message);

    shared_lock<shared_mutex> _(mutex_);

    const GUID_t remote_participant_key(sending_participant, c_EntityId_RTPSParticipant);
    std::shared_ptr<ParticipantCryptoHandle> remote_participant_crypto_handle;

    if (remote_participant_key == participant_->getGuid())
    {
        remote_participant_crypto_handle = local_participant_crypto_handle_;
    }
    else
    {
        auto dp_it = discovered_participants_.find(remote_participant_key);

        if (dp_it != discovered_participants_.end())
        {
            remote_participant_crypto_handle = dp_it->second->get_participant_crypto();
        }
    }

    if (remote_participant_crypto_handle != nullptr)
    {
        DatawriterCryptoHandle* writer_handle = nullptr;
        DatareaderCryptoHandle* reader_handle = nullptr;
        SecureSubmessageCategory_t category = INFO_SUBMESSAGE;
        SecurityException exception;

        // encoding requires modifying the handle but it takes care of its own synchronization
        if (crypto_plugin_->cryptotransform()->preprocess_secure_submsg(&writer_handle, &reader_handle,
                category, message, *local_participant_crypto_handle_,
                *remote_participant_crypto_handle, exception))
        {
            // TODO (Ricardo) Category INFO
            if (category == DATAWRITER_SUBMESSAGE)
            {
                if (crypto_plugin_->cryptotransform()->decode_datawriter_submessage(out_message, message,
                        *reader_handle, *writer_handle, exception))
                {
                    return 0;
                }
                else
                {
                    EPROSIMA_LOG_WARNING(SECURITY, "Cannot decode writer RTPS submessage (" << exception.what() << ")");
                }
            }
            else if (category == DATAREADER_SUBMESSAGE)
            {
                if (crypto_plugin_->cryptotransform()->decode_datareader_submessage(out_message, message,
                        *writer_handle, *reader_handle, exception))
                {
                    return 0;
                }
                else
                {
                    EPROSIMA_LOG_WARNING(SECURITY, "Cannot decode reader RTPS submessage (" << exception.what() << ")");
                }
            }
        }
        else
        {
            EPROSIMA_LOG_INFO(SECURITY, "Cannot preprocess RTPS submessage (" << exception.what() << ")");
        }
    }
    else
    {
        EPROSIMA_LOG_INFO(SECURITY, "Cannot decode RTPS submessage for participant " << remote_participant_key);
    }

    return -1;
}

bool SecurityManager::encode_serialized_payload(
        const SerializedPayload_t& payload,
        SerializedPayload_t& output_payload,
        const GUID_t& writer_guid) const
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    if (crypto_plugin_ == nullptr)
    {
        return false;
    }

    shared_lock<shared_mutex> _(mutex_);

    const auto& wr_it = writer_handles_.find(writer_guid);

    if (wr_it != writer_handles_.end())
    {
        SecurityException exception;
        std::vector<uint8_t> extra_inline_qos;

        if (crypto_plugin_->cryptotransform()->encode_serialized_payload(output_payload,
                extra_inline_qos,
                payload,
                *wr_it->second.writer_handle,
                exception))
        {
            return true;
        }
        else
        {
            EPROSIMA_LOG_ERROR(SECURITY, "Error encoding payload failed");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(SECURITY, "Cannot find local writer " << writer_guid);
    }

    return false;
}

bool SecurityManager::decode_serialized_payload(
        const SerializedPayload_t& secure_payload,
        SerializedPayload_t& payload,
        const GUID_t& reader_guid,
        const GUID_t& writer_guid) const
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    if (crypto_plugin_ == nullptr)
    {
        return false;
    }

    shared_lock<shared_mutex> _(mutex_);

    const auto& rd_it = reader_handles_.find(reader_guid);

    if (rd_it != reader_handles_.end())
    {
        const auto wr_it_handle = rd_it->second.associated_writers.find(writer_guid);

        if (wr_it_handle != rd_it->second.associated_writers.end())
        {
            std::vector<uint8_t> inline_qos;
            SecurityException exception;

            if (crypto_plugin_->cryptotransform()->decode_serialized_payload(payload,
                    secure_payload, inline_qos, *rd_it->second.reader_handle,
                    *std::get<1>(wr_it_handle->second), exception))
            {
                return true;
            }
            else
            {
                EPROSIMA_LOG_ERROR(SECURITY, "Error decoding encoded payload (" << exception.what() << ")");
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(SECURITY, "Cannot find remote writer " << writer_guid);
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(SECURITY, "Cannot find local reader " << reader_guid);
    }

    return false;
}

bool SecurityManager::participant_authorized(
        const ParticipantProxyData& participant_data,
        const DiscoveredParticipantInfo::AuthUniquePtr& remote_participant_info,
        std::shared_ptr<SecretHandle>& shared_secret_handle)
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return false;
    }

    EPROSIMA_LOG_INFO(SECURITY, "Authorized participant " << participant_data.guid);

    SecurityException exception;
    PermissionsHandle* remote_permissions = nullptr;

    if (access_plugin_ != nullptr)
    {
        PermissionsCredentialToken* credential_token = nullptr;
        if (authentication_plugin_->get_authenticated_peer_credential_token(&credential_token,
                *remote_participant_info->identity_handle_, exception))
        {
            remote_permissions =
                    access_plugin_->validate_remote_permissions(*authentication_plugin_,
                            *local_identity_handle_,
                            *local_permissions_handle_,
                            *remote_participant_info->identity_handle_,
                            participant_data.permissions_token_,
                            *credential_token, exception);

            if (remote_permissions != nullptr && !remote_permissions->nil())
            {
                if (!access_plugin_->check_remote_participant(*remote_permissions, domain_id_,
                        participant_data, exception))
                {
                    EPROSIMA_LOG_ERROR(SECURITY, "Error checking remote participant  " <<
                            participant_data.guid << " (" << exception.what() << ").");
                    access_plugin_->return_permissions_handle(remote_permissions, exception);
                    remote_permissions = nullptr;
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(SECURITY, "Error validating remote permissions for " <<
                        participant_data.guid << " (" << exception.what() << ").");

                if (remote_permissions != nullptr)
                {
                    access_plugin_->return_permissions_handle(remote_permissions, exception);
                    remote_permissions = nullptr;
                }
            }

            authentication_plugin_->return_authenticated_peer_credential_token(credential_token,
                    exception);
        }
        else
        {
            EPROSIMA_LOG_ERROR(SECURITY, "Not receive remote permissions of participant " <<
                    participant_data.guid << " (" << exception.what() << ").");
        }
    }

    if (access_plugin_ == nullptr || remote_permissions != nullptr)
    {

        std::list<std::pair<ReaderProxyData, GUID_t>> temp_readers;
        std::list<std::pair<WriterProxyData, GUID_t>> temp_writers;

        if (crypto_plugin_ != nullptr)
        {
            // TODO(Ricardo) Study cryptography without sharedsecret
            if (!shared_secret_handle)
            {
                EPROSIMA_LOG_ERROR(SECURITY, "Not shared secret for participant " << participant_data.guid);
                return false;
            }

            // Starts cryptography mechanism
            std::shared_ptr<ParticipantCryptoHandle> participant_crypto_handle =
                    register_and_match_crypto_endpoint(*remote_participant_info->identity_handle_,
                            *shared_secret_handle);

            // Store cryptography info
            if (participant_crypto_handle && !participant_crypto_handle->nil())
            {
                std::lock_guard<shared_mutex> _(mutex_);

                // Check there is a pending crypto message.
                auto pending = remote_participant_pending_messages_.find(participant_data.guid);

                if (pending != remote_participant_pending_messages_.end())
                {
                    if (!crypto_plugin_->cryptokeyexchange()->set_remote_participant_crypto_tokens(
                                *local_participant_crypto_handle_,
                                *participant_crypto_handle,
                                pending->second,
                                exception))
                    {
                        EPROSIMA_LOG_ERROR(SECURITY, "Cannot set remote participant crypto tokens ("
                                << participant_data.guid << ") - (" << exception.what() << ")");
                    }

                    remote_participant_pending_messages_.erase(pending);
                }

                // Search in pendings readers and writers
                auto rit = remote_reader_pending_discovery_messages_.begin();
                while (rit != remote_reader_pending_discovery_messages_.end())
                {
                    if (std::get<1>(*rit) == participant_data.guid)
                    {
                        temp_readers.push_back(std::make_pair(std::get<0>(*rit), std::get<2>(*rit)));
                        rit = remote_reader_pending_discovery_messages_.erase(rit);
                        continue;
                    }

                    ++rit;
                }

                auto wit = remote_writer_pending_discovery_messages_.begin();
                while (wit != remote_writer_pending_discovery_messages_.end())
                {
                    if (std::get<1>(*wit) == participant_data.guid)
                    {
                        temp_writers.push_back(std::make_pair(std::get<0>(*wit), std::get<2>(*wit)));
                        wit = remote_writer_pending_discovery_messages_.erase(wit);
                        continue;
                    }

                    ++wit;
                }

                auto dp_it = discovered_participants_.find(participant_data.guid);

                if (dp_it != discovered_participants_.end())
                {
                    dp_it->second->set_participant_crypto(participant_crypto_handle);
                    dp_it->second->set_shared_secret(shared_secret_handle);
                    dp_it->second->set_permissions_handle(remote_permissions);
                }
                else
                {
                    crypto_plugin_->cryptokeyfactory()->unregister_participant(participant_crypto_handle, exception);
                    EPROSIMA_LOG_ERROR(SECURITY, "Cannot find remote participant " << participant_data.guid);
                    return false;
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(SECURITY, "Cannot register remote participant in crypto plugin ("
                        << participant_data.guid << ")");
                return false;
            }

            match_builtin_key_exchange_endpoints(participant_data);
            exchange_participant_crypto(participant_crypto_handle, participant_data.guid);
        }
        else
        {
            {
                shared_lock<shared_mutex> _(mutex_);

                // Store shared_secret.
                auto dp_it = discovered_participants_.find(participant_data.guid);

                if (dp_it != discovered_participants_.end())
                {
                    dp_it->second->set_shared_secret(shared_secret_handle);
                    dp_it->second->set_permissions_handle(remote_permissions);
                }
            }

            match_builtin_key_exchange_endpoints(participant_data);
        }

        for (auto& remote_reader : temp_readers)
        {
            participant_->pdp()->get_edp()->pairing_reader_proxy_with_local_writer(remote_reader.second,
                    participant_data.guid, remote_reader.first);
        }

        for (auto& remote_writer : temp_writers)
        {
            participant_->pdp()->get_edp()->pairing_writer_proxy_with_local_reader(remote_writer.second,
                    participant_data.guid, remote_writer.first);
        }

        return true;
    }

    return false;
}

void SecurityManager::notify_participant_authorized(
        const ParticipantProxyData& participant_data)
{
    participant_->pdp()->notifyAboveRemoteEndpoints(participant_data, true);

    EPROSIMA_LOG_INFO(SECURITY, "Participant " << participant_data.guid << " authenticated");

    // Inform user about authenticated remote participant.
    if (participant_->getListener() != nullptr)
    {
        ParticipantAuthenticationInfo info;
        info.status = ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT;
        info.guid = participant_data.guid;
        participant_->getListener()->onParticipantAuthentication(
            participant_->getUserRTPSParticipant(), std::move(info));
    }
}

uint32_t SecurityManager::calculate_extra_size_for_rtps_message() const
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return 0;
    }

    if (crypto_plugin_ == nullptr)
    {
        return 0;
    }

    shared_lock<shared_mutex> _(mutex_);

    return crypto_plugin_->cryptotransform()->calculate_extra_size_for_rtps_message(
        static_cast<uint32_t>(discovered_participants_.size()));
}

uint32_t SecurityManager::calculate_extra_size_for_rtps_submessage(
        const GUID_t& writer_guid) const
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return 0;
    }

    if (crypto_plugin_ == nullptr)
    {
        return 0;
    }

    shared_lock<shared_mutex> _(mutex_);

    auto wr_it = writer_handles_.find(writer_guid);

    if (wr_it != writer_handles_.end())
    {
        return crypto_plugin_->cryptotransform()->calculate_extra_size_for_rtps_submessage(
            static_cast<uint32_t>(wr_it->second.associated_readers.size()));
    }
    else
    {
        EPROSIMA_LOG_INFO(SECURITY, "Not found local writer " << writer_guid);
    }

    return 0;
}

uint32_t SecurityManager::calculate_extra_size_for_encoded_payload(
        const GUID_t& writer_guid) const
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return 0;
    }

    if (crypto_plugin_ == nullptr)
    {
        return 0;
    }

    shared_lock<shared_mutex> _(mutex_);

    auto wr_it = writer_handles_.find(writer_guid);

    if (wr_it != writer_handles_.end())
    {
        return crypto_plugin_->cryptotransform()->calculate_extra_size_for_encoded_payload(
            static_cast<uint32_t>(wr_it->second.associated_readers.size()));
    }
    else
    {
        EPROSIMA_LOG_INFO(SECURITY, "Not found local writer " << writer_guid);
    }

    return 0;
}

void SecurityManager::resend_handshake_message_token(
        const GUID_t& remote_participant_key) const
{
    auto sentry = is_security_manager_initialized();
    if (!sentry)
    {
        return;
    }

    shared_lock<shared_mutex> _(mutex_);

    auto dp_it = discovered_participants_.find(remote_participant_key);

    if (dp_it != discovered_participants_.end())
    {
        SecurityManager::DiscoveredParticipantInfo::AuthUniquePtr remote_participant_info = dp_it->second->get_auth();

        if (remote_participant_info)
        {
            if (remote_participant_info->handshake_requests_sent_ >= auth_handshake_props_.max_handshake_requests_)
            {
                if (remote_participant_info->auth_status_ != AUTHENTICATION_FAILED)
                {
                    SecurityException exception;
                    remote_participant_info->event_->cancel_timer();
                    remote_participant_info->auth_status_ = AUTHENTICATION_FAILED;
                    on_validation_failed(dp_it->second->participant_data(), exception);
                }
            }
            else
            {
                if (remote_participant_info->change_sequence_number_ != SequenceNumber_t::unknown())
                {
                    CacheChange_t* p_change = participant_stateless_message_writer_history_->remove_change_and_reuse(
                        remote_participant_info->change_sequence_number_);
                    remote_participant_info->change_sequence_number_ = SequenceNumber_t::unknown();

                    if (p_change != nullptr)
                    {
                        EPROSIMA_LOG_INFO(SECURITY, "Authentication handshake resent to participant " <<
                                remote_participant_key);
                        if (participant_stateless_message_writer_history_->add_change(p_change))
                        {
                            remote_participant_info->change_sequence_number_ = p_change->sequenceNumber;
                            remote_participant_info->handshake_requests_sent_++;
                        }
                        //TODO (Ricardo) What to do if not added?
                    }
                }

                if (remote_participant_info->auth_status_ == AUTHENTICATION_WAITING_REPLY)
                {
                    // Avoid DoS attack by exponentially increasing event interval
                    auto time_ms = remote_participant_info->event_->getIntervalMilliSec();
                    remote_participant_info->event_->update_interval_millisec(
                        time_ms * auth_handshake_props_.handshake_resend_period_gain_);
                    remote_participant_info->event_->restart_timer();
                }
            }

            dp_it->second->set_auth(remote_participant_info);
        }
    }
}

void SecurityManager::on_validation_failed(
        const ParticipantProxyData& participant_data,
        const SecurityException& exception) const
{
    if (participant_->security_attributes().allow_unauthenticated_participants)
    {
        participant_->pdp()->notifyAboveRemoteEndpoints(participant_data, false);
    }

    if (strlen(exception.what()) > 0)
    {
        EPROSIMA_LOG_ERROR(SECURITY_AUTHENTICATION, exception.what());
    }

    EPROSIMA_LOG_INFO(SECURITY, "Authentication failed for participant " <<
            participant_data.guid);

    // Inform user about authenticated remote participant.
    if (participant_->getListener() != nullptr)
    {
        ParticipantAuthenticationInfo info;
        info.status = ParticipantAuthenticationInfo::UNAUTHORIZED_PARTICIPANT;
        info.guid = participant_data.guid;
        participant_->getListener()->onParticipantAuthentication(
            participant_->getUserRTPSParticipant(), std::move(info));
    }
}

SecurityManager::AuthenticationHandshakeProperties::AuthenticationHandshakeProperties()
    : max_handshake_requests_(10)
    , initial_handshake_resend_period_ms_(125)
    , handshake_resend_period_gain_(1.5)
{

}

void SecurityManager::AuthenticationHandshakeProperties::parse_from_property_policy(
        const PropertyPolicy& auth_handshake_properties)
{
    const Property* const max_handshake_requests =
            PropertyPolicyHelper::get_property(auth_handshake_properties, "max_handshake_requests");

    if (max_handshake_requests != nullptr)
    {
        max_handshake_requests_ = (int32_t)PropertyParser::as_int(
            *max_handshake_requests,
            false, 0,
            true, 1,
            SecurityException("Error parsing max_handshake_requests property value."));
    }

    const Property* const initial_handshake_resend_period =
            PropertyPolicyHelper::get_property(auth_handshake_properties, "initial_handshake_resend_period");

    if (initial_handshake_resend_period != nullptr)
    {
        initial_handshake_resend_period_ms_ = (int32_t)PropertyParser::as_int(
            *initial_handshake_resend_period,
            false, 0,
            true, 1,
            SecurityException("Error parsing initial_handshake_resend_period property value."));
    }

    const Property* const handshake_resend_period_gain =
            PropertyPolicyHelper::get_property(auth_handshake_properties, "handshake_resend_period_gain");

    if (handshake_resend_period_gain != nullptr)
    {
        handshake_resend_period_gain_ = PropertyParser::as_double(
            *handshake_resend_period_gain,
            false, 0.0,
            true, 1.0,
            SecurityException("Error parsing handshake_resend_period_gain property value."));
    }
}

bool SecurityManager::DiscoveredParticipantInfo::check_guid_comes_from(
        Authentication* const auth_plugin,
        const GUID_t& adjusted,
        const GUID_t& original)
{
    bool ret = false;
    if (auth_plugin != nullptr)
    {
        std::lock_guard<std::mutex> g(mtx_);

        if (nullptr != auth_ && AuthenticationStatus::AUTHENTICATION_OK == auth_->auth_status_)
        {
            ret = auth_plugin->check_guid_comes_from(auth_->identity_handle_, adjusted, original);
        }
    }
    return ret;
}

void SecurityManager::on_writer_change_received_by_all(
        RTPSWriter* writer,
        CacheChange_t* change)
{
    static_cast<void>(writer);
    assert(writer == participant_volatile_message_secure_writer_);

    if (nullptr != participant_volatile_message_secure_writer_history_)
    {
        participant_volatile_message_secure_writer_history_->remove_change(change);
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
