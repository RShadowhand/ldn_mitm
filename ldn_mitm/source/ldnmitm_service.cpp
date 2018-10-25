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
 
#include <switch.h>
#include "ldnmitm_service.hpp"

#include "mitm_query_service.hpp"
#include "debug.hpp"
#include "ldnmitm_worker.hpp"

Result LdnMitMService::dispatch(IpcParsedCommand &r, IpcCommand &out_c, u64 cmd_id, u8 *pointer_buffer, size_t pointer_buffer_size) {
    char buf[128];
    sprintf(buf, "LdnMitMService::dispatch cmd_id: %" PRIu64 " is domain message %d\n", cmd_id, r.IsDomainRequest);
    LogStr(buf);
    Result rc = 0xF601;
    switch (static_cast<LdnSrvCmd>(cmd_id)) {
        case LdnSrvCmd::CreateUserLocalCommunicationService: 
            rc = WrapIpcCommandImpl<&LdnMitMService::create_user_local_communication_service>(this, r, out_c, pointer_buffer, pointer_buffer_size);
            break;
        default:
            break;
    }
    sprintf(buf, "LdnMitMService::dispatch rc: %u\n", rc);
    LogStr(buf);
    return rc;
}

void LdnMitMService::postprocess(IpcParsedCommand &r, IpcCommand &out_c, u64 cmd_id, u8 *pointer_buffer, size_t pointer_buffer_size) {
    return;
}

std::tuple<Result, OutSession<IMitMCommunicationInterface>> LdnMitMService::create_user_local_communication_service() {
    Result rc = 0;
    // IPCSession<ICommunicationInterface> *out_session = new IPCSession<ICommunicationInterface>(communication);
    IPCSession<IMitMCommunicationInterface> *out_session = new IPCSession<IMitMCommunicationInterface>(
        std::make_shared<IMitMCommunicationInterface>(forward_service)
    );

    LdnMitMWorker::AddWaitable(out_session);

    OutSession out_s = OutSession(out_session);
    return {rc, out_s};
}

Result LdnMitMService::handle_deferred() {
    /* This service is never deferrable. */
    return 0;
}
