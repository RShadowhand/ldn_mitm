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

#include "sm_mitm.h"

#include "mitm_server.hpp"
#include "ldnmitm_service.hpp"
#include "ldnmitm_worker.hpp"
#include "lan_discovery.hpp"

#include "mitm_query_service.hpp"

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
    
    rc = smMitMInitialize();
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

    rc = nifmInitialize();
    if (R_FAILED(rc)) {
        fatalLater(rc);
    }

    // rc = splInitialize();
    // if (R_FAILED(rc))  {
    //     fatalSimple(0xCAFE << 4 | 3);
    // }
    
    // /* Check for exosphere API compatibility. */
    // u64 exosphere_cfg;
    // if (R_SUCCEEDED(splGetConfig((SplConfigItem)65000, &exosphere_cfg))) {
    //     /* MitM requires Atmosphere API 0.1. */
    //     u16 api_version = (exosphere_cfg >> 16) & 0xFFFF;
    //     if (api_version < 0x0001) {
    //         fatalSimple(0xCAFE << 4 | 0xFE);
    //     }
    // } else {
    //     fatalSimple(0xCAFE << 4 | 0xFF);
    // }
    
    LogStr("__appInit done\n");
    //splExit();
}

void __appExit(void) {
    LogStr("__appExit\n");
    /* Cleanup services. */
    // splExit();
    nifmExit();
    fsdevUnmountAll();
    fsExit();
    smMitMExit();
    smExit();
}

int main(int argc, char **argv)
{
    Thread worker_thread = {0};
    Thread lan_thread = {0};
    consoleDebugInit(debugDevice_SVC);
    LogStr("main\n");

    if (R_FAILED(threadCreate(&worker_thread, &LdnMitMWorker::Main, NULL, 0x20000, 45, 0))) {
        /* TODO: Panic. */
        LogStr("Error LdnMitMWorker create\n");
    }
    if (R_FAILED(threadStart(&worker_thread))) {
        /* TODO: Panic. */
        LogStr("Error LdnMitMWorker start\n");
    }
    if (R_FAILED(threadCreate(&lan_thread, &LANDiscovery::Main, NULL, 0x20000, 45, 0))) {
        /* TODO: Panic. */
        LogStr("Error LANDiscovery create\n");
    }
    if (R_FAILED(threadStart(&lan_thread))) {
        /* TODO: Panic. */
        LogStr("Error LANDiscovery start\n");
    }

    /* TODO: What's a good timeout value to use here? */
    auto server_manager = std::make_unique<MultiThreadedWaitableManager>(1, U64_MAX, 0x20000);
    //auto server_manager = std::make_unique<WaitableManager>(U64_MAX);

    /* Create ldn:s mitm. */
    ISession<MitMQueryService<LdnMitMService>> *ldn_query_srv = NULL;
    MitMServer<LdnMitMService> *ldn_srv = new MitMServer<LdnMitMService>(&ldn_query_srv, "ldn:u", 61);
    server_manager->add_waitable(ldn_srv);
    server_manager->add_waitable(ldn_query_srv);

    LogStr("main process\n");
    /* Loop forever, servicing our services. */
    server_manager->process();
    LogStr("main process done\n");

    return 0;
}
