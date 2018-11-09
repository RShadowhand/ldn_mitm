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
 
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <malloc.h>

#include <switch.h>
#include <stratosphere.hpp>

#include "ldnmitm_service.hpp"

#define TITLE_ID 0x4200000000000010

extern "C" {
    extern u32 __start__;

    u32 __nx_applet_type = AppletType_None;

    #define INNER_HEAP_SIZE 0x260000
    size_t nx_inner_heap_size = INNER_HEAP_SIZE;
    char   nx_inner_heap[INNER_HEAP_SIZE];
    
    void __libnx_initheap(void);
    void __appInit(void);
    void __appExit(void);
}


void __libnx_initheap(void) {
	void*  addr = nx_inner_heap;
	size_t size = nx_inner_heap_size;

	/* Newlib */
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = (char*)addr;
	fake_heap_end   = (char*)addr + size;
}

void registerFspLr()
{
    if (kernelAbove400())
        return;

    Result rc = fsprInitialize();
    if (R_FAILED(rc))
        fatalLater(rc);

    u64 pid;
    svcGetProcessId(&pid, CUR_PROCESS_HANDLE);

    rc = fsprRegisterProgram(pid, TITLE_ID, FsStorageId_NandSystem, NULL, 0, NULL, 0);
    if (R_FAILED(rc))
        fatalLater(rc);
    fsprExit();
}

void __appInit(void) {
    Result rc;
    svcSleepThread(10000000000L);
    
    rc = smInitialize();
    if (R_FAILED(rc)) {
        fatalLater(rc);
    }
    
    rc = fsInitialize();
    if (R_FAILED(rc)) {
        fatalLater(rc);
    }
    registerFspLr();
    
    rc = fsdevMountSdmc();
    if (R_FAILED(rc)) {
        fatalLater(rc);
    }

    rc = ipinfoInit();
    if (R_FAILED(rc)) {
        fatalLater(rc);
    }

    CheckAtmosphereVersion();
    LogStr("__appInit done\n");
}

void __appExit(void) {
    /* Cleanup services. */
    ipinfoExit();
    fsdevUnmountAll();
    fsExit();
    smExit();
}

struct LdnMitmManagerOptions {
    static const size_t PointerBufferSize = 0x1000;
    static const size_t MaxDomains = 0x10;
    static const size_t MaxDomainObjects = 0x4000;
};

class LogServiceSession : public ServiceSession {
    public:
        LogServiceSession(Handle s_h, size_t pbs, ServiceObjectHolder &&h) : ServiceSession(s_h, pbs, std::move(h)) { }
        virtual void PreProcessRequest(IpcResponseContext *ctx) override {
            char buf[64];
            sprintf(buf, "req %" PRIu64 "\n", ctx->cmd_id);
            LogStr(buf);
            LogHex(armGetTls(), 0x100);
        }
        virtual void PostProcessResponse(IpcResponseContext *ctx) override {
            char buf[64];
            sprintf(buf, "res %" PRIu64 " rc %d\n", ctx->cmd_id, ctx->rc);
            LogStr(buf);
            LogHex(armGetTls(), 0x100);
        }
};
template<typename ManagerOptions>
class LdnWaitableManager : public WaitableManager<ManagerOptions> {
    public:
        LdnWaitableManager(u32 n, u32 ss = 0x8000) : WaitableManager<ManagerOptions>(n, ss) {};
        virtual void AddSession(Handle server_h, ServiceObjectHolder &&service) override {
            LogStr("AddSession\n");
            this->AddWaitable(new LogServiceSession(server_h, ManagerOptions::PointerBufferSize, std::move(service)));
        }
};
using LdnMitmManager = LdnWaitableManager<LdnMitmManagerOptions>;

int main(int argc, char **argv)
{
    consoleDebugInit(debugDevice_SVC);
    LogStr("main\n");

    /* TODO: What's a good timeout value to use here? */
    auto server_manager = new LdnMitmManager(5);

    /* Create ldn:s mitm. */
    AddMitmServerToManager<LdnMitMService>(server_manager, "ldn:u", 61);

    LogStr("main process\n");
    /* Loop forever, servicing our services. */
    server_manager->Process();
    LogStr("main process done\n");

    return 0;
}
