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
#include "debug.hpp"

#include "ldnmitm_service.hpp"
#include "ldn_icommunication.hpp"

Result LdnMitMService::CreateUserLocalCommunicationService(Out<std::shared_ptr<ICommunicationInterface>> out) {
    LogStr("CreateUserLocalCommunicationService\n");
    Result rc = 0;

    auto comm = std::make_shared<ICommunicationInterface>();
    out.SetValue(std::move(comm));

    return rc;
}
