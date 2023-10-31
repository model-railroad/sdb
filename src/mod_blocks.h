/*
 * Project: Software Defined Blocks
 * Copyright (C) 2023 alf.labs gmail com.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INC_SDB_MOD_BLOCKS_H
#define INC_SDB_MOD_BLOCKS_H

#include "common.h"
#include "sdb_mod.h"
#include "sdb_block.h"
#include "sdb_data_store.h"

#define MOD_BLOCKS_NAME "bl"

class SdbModBlocks : public SdbModTask {
public:
    explicit SdbModBlocks(SdbModManager& manager) :
       SdbModTask(manager, MOD_BLOCKS_NAME, "TaskBlocks", SdbPriority::Logic)
    { }

    void onStart() override {
        // TBD: register 2 static blocks. This will be dynamic later.
        _manager.registerBlock(
            new SdbBlock(_manager,
                         "block0",
                         _manager.sensorByName("tof0"),
                         SdbKey::Block0NegateLong,
                         SdbKey::Block0JmriNameStr,
                         SdbKey::Block0MqttTopicStr));
        _manager.registerBlock(
            new SdbBlock(_manager,
                         "block1",
                         _manager.sensorByName("tof1"),
                         static_cast<SdbKey::SdbKey>(SdbKey::Block0NegateLong + 1),
                         static_cast<SdbKey::SdbKey>(SdbKey::Block0JmriNameStr + 1),
                         static_cast<SdbKey::SdbKey>(SdbKey::Block0MqttTopicStr + 1)));

        for (auto* b : _manager.blocks()) {
            b->onStart();
        }

        startTask();
    }

    long onLoop() override {
        return 2000;
    }

private:
    [[noreturn]] void onTaskRun() override {
        while(true) {
            // TBD synchronize when blocks() becomes dynamic.
            auto& vector = _manager.blocks();

            for (auto* block : vector) {
                if (block->update() || block->needsRefresh()) {
                    block->notify();
                }
            }

            rtDelay(50L);
        }
    }
};


#endif // INC_SDB_MOD_BLOCKS_H
