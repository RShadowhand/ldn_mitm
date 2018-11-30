/*
 * Copyright (c) 2018 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include "debug.hpp"
#include "lan_discovery.hpp"
#include "ldn_types.h"
#include "ipinfo.hpp"

enum LdnCommCmd {
    LdnCommCmd_GetState = 0,
    LdnCommCmd_GetNetworkInfo = 1,
    LdnCommCmd_GetIpv4Address = 2,
    LdnCommCmd_GetDisconnectReason = 3,
    LdnCommCmd_GetSecurityParameter = 4,
    LdnCommCmd_GetNetworkConfig = 5,
    LdnCommCmd_AttachStateChangeEvent = 100,
    LdnCommCmd_GetNetworkInfoLatestUpdate = 101,
    LdnCommCmd_Scan = 102,
    LdnCommCmd_ScanPrivate = 103,
    LdnCommCmd_SetWirelessControllerRestriction = 104,  // nyi. Not sure the name of 104. guessed from smash
    LdnCommCmd_OpenAccessPoint = 200,
    LdnCommCmd_CloseAccessPoint = 201,
    LdnCommCmd_CreateNetwork = 202,
    LdnCommCmd_CreateNetworkPrivate = 203,
    LdnCommCmd_DestroyNetwork = 204,
    LdnCommCmd_Reject = 205,                            // nyi
    LdnCommCmd_SetAdvertiseData = 206,
    LdnCommCmd_SetStationAcceptPolicy = 207,            // nyi
    LdnCommCmd_AddAcceptFilterEntry = 208,              // nyi
    LdnCommCmd_ClearAcceptFilter = 209,                 // nyi
    LdnCommCmd_OpenStation = 300,
    LdnCommCmd_CloseStation = 301,
    LdnCommCmd_Connect = 302,
    LdnCommCmd_ConnectPrivate = 303,
    LdnCommCmd_Disconnect = 304,
    LdnCommCmd_Initialize = 400,
    LdnCommCmd_Finalize = 401,
};

enum class CommState {
    None,
    Initialized,
    AccessPoint,
    AccessPointCreated,
    Station,
    StationConnected,
    Error
};

class ICommunicationInterface : public IServiceObject {
    private:
        LANDiscovery lanDiscovery;
        CommState state;
        IEvent *state_event;
    public:
        ICommunicationInterface(): state(CommState::None), state_event(nullptr) {
            LogStr("ICommunicationInterface\n");
            /* ... */
        };
        
        ~ICommunicationInterface() {
            LogStr("~ICommunicationInterface\n");
            /* ... */
            if (this->state_event) {
                delete this->state_event;
            }
        };
    private:
        void set_state(CommState new_state) {
            this->state = new_state;
            if (this->state_event) {
                LogStr("state_event signal_event\n");
                this->state_event->Signal();
            }
        }
        void onNodeChanged();
    private:
        Result Initialize(u64 unk, PidDescriptor pid);
        Result Finalize();
        Result GetState(Out<u32> state);
        Result GetNetworkInfo(OutPointerWithServerSize<NetworkInfo, 1> buffer);
        Result GetIpv4Address(Out<u32> address, Out<u32> mask);
        Result GetDisconnectReason(Out<u32> reason);
        Result GetSecurityParameter(Out<SecurityParameter> out);
        Result GetNetworkConfig(Out<NetworkConfig> out);
        Result OpenAccessPoint();
        Result CloseAccessPoint();
        Result DestroyNetwork();
        Result CreateNetwork(CreateNetworkConfig data);
        Result OpenStation();
        Result CloseStation();
        Result Disconnect();
        Result SetAdvertiseData(InPointer<u8> data1, InBuffer<u8> data2);
        Result SetStationAcceptPolicy(u8 policy);
        Result AttachStateChangeEvent(Out<CopiedHandle> handle);
        Result Scan(Out<u32> count, OutBuffer<NetworkInfo> buffer, OutPointerWithServerSize<u8, 0> _);
        Result Connect(ConnectNetworkData dat, InPointer<NetworkInfo> data);
        Result GetNetworkInfoLatestUpdate(OutPointerWithServerSize<NetworkInfo, 1> buffer1, OutPointerWithServerSize<NodeLatestUpdate, 1> buffer2);
        Result SetWirelessControllerRestriction();

        Result CreateNetworkPrivate(SecurityConfig securityConfig, PrivateData priData, UserConfig userConfig, NetworkConfig networkConfig, InPointer<u8> buffer);
        Result ConnectPrivate(SecurityConfig securityConfig, PrivateData priData, UserConfig userConfig, uint32_t localVersion, uint32_t option, PrivateData priData2);
        Result ScanPrivate(Out<u32> count, OutBuffer<NetworkInfo> buffer, OutPointerWithServerSize<u8, 0> _);
    public:
        DEFINE_SERVICE_DISPATCH_TABLE {
            MakeServiceCommandMeta<LdnCommCmd_GetState, &ICommunicationInterface::GetState>(),
            MakeServiceCommandMeta<LdnCommCmd_GetNetworkInfo, &ICommunicationInterface::GetNetworkInfo>(),
            MakeServiceCommandMeta<LdnCommCmd_GetIpv4Address, &ICommunicationInterface::GetIpv4Address>(),
            MakeServiceCommandMeta<LdnCommCmd_GetDisconnectReason, &ICommunicationInterface::GetDisconnectReason>(),
            MakeServiceCommandMeta<LdnCommCmd_GetSecurityParameter, &ICommunicationInterface::GetSecurityParameter>(),
            MakeServiceCommandMeta<LdnCommCmd_GetNetworkConfig, &ICommunicationInterface::GetNetworkConfig>(),
            MakeServiceCommandMeta<LdnCommCmd_AttachStateChangeEvent, &ICommunicationInterface::AttachStateChangeEvent>(),
            MakeServiceCommandMeta<LdnCommCmd_GetNetworkInfoLatestUpdate, &ICommunicationInterface::GetNetworkInfoLatestUpdate>(),
            MakeServiceCommandMeta<LdnCommCmd_Scan, &ICommunicationInterface::Scan>(),
            MakeServiceCommandMeta<LdnCommCmd_SetWirelessControllerRestriction, &ICommunicationInterface::SetWirelessControllerRestriction>(),
            MakeServiceCommandMeta<LdnCommCmd_OpenAccessPoint, &ICommunicationInterface::OpenAccessPoint>(),
            MakeServiceCommandMeta<LdnCommCmd_CloseAccessPoint, &ICommunicationInterface::CloseAccessPoint>(),
            MakeServiceCommandMeta<LdnCommCmd_CreateNetwork, &ICommunicationInterface::CreateNetwork>(),
            MakeServiceCommandMeta<LdnCommCmd_DestroyNetwork, &ICommunicationInterface::DestroyNetwork>(),
            MakeServiceCommandMeta<LdnCommCmd_OpenStation, &ICommunicationInterface::OpenStation>(),
            MakeServiceCommandMeta<LdnCommCmd_CloseStation, &ICommunicationInterface::CloseStation>(),
            MakeServiceCommandMeta<LdnCommCmd_Connect, &ICommunicationInterface::Connect>(),
            MakeServiceCommandMeta<LdnCommCmd_Disconnect, &ICommunicationInterface::Disconnect>(),
            MakeServiceCommandMeta<LdnCommCmd_SetAdvertiseData, &ICommunicationInterface::SetAdvertiseData>(),
            MakeServiceCommandMeta<LdnCommCmd_SetStationAcceptPolicy, &ICommunicationInterface::SetStationAcceptPolicy>(),
            MakeServiceCommandMeta<LdnCommCmd_Initialize, &ICommunicationInterface::Initialize>(),
            MakeServiceCommandMeta<LdnCommCmd_Finalize, &ICommunicationInterface::Finalize>(),

            MakeServiceCommandMeta<LdnCommCmd_CreateNetworkPrivate, &ICommunicationInterface::CreateNetworkPrivate>(),
            MakeServiceCommandMeta<LdnCommCmd_ConnectPrivate, &ICommunicationInterface::ConnectPrivate>(),
            MakeServiceCommandMeta<LdnCommCmd_ScanPrivate, &ICommunicationInterface::ScanPrivate>(),
        };
};
