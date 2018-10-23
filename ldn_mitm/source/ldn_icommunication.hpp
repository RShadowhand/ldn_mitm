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

enum class LdnICommunicationCmd {
    Read = 0,
    Write = 1,
    Flush = 2,
    SetSize = 3,
    GetSize = 4,
    OperateRange = 5,
};

class ICommunicationInterface : public IServiceObject {
    private:
    public:
        ICommunicationInterface() {
            LogStr("ICommunicationInterface\n");
            /* ... */
        };
        
        ICommunicationInterface *clone() override {
            LogStr("ICommunicationInterface::clone\n");
            return new ICommunicationInterface();
        }
        
        ~ICommunicationInterface() {
            LogStr("~ICommunicationInterface\n");
            /* ... */
        };
        
        Result dispatch(IpcParsedCommand &r, IpcCommand &out_c, u64 cmd_id, u8 *pointer_buffer, size_t pointer_buffer_size) final {
            Result rc = 0xF601;
            char buf[128];
            sprintf(buf, "ICommunicationInterface::dispatch cmd_id: %" PRIu64 "\n", cmd_id);
            LogStr(buf);
            return rc;
        };
        
        Result handle_deferred() final {
            /* TODO: Panic, we can never defer. */
            return 0;
        };
    private:
};
