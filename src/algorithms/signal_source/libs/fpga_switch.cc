/*!
 * \file fpga_switch.cc
 * \brief Switch that connects the HW accelerator queues to the analog front end or the DMA.
 * \authors <ul>
 *    <li> Marc Majoral, 2017. mmajoral(at)cttc.cat
 *    <li> Javier Arribas, 2015. jarribas(at)cttc.es
 * </ul>
 *
 * Class that controls a switch in the FPGA
 *
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */

#include "fpga_switch.h"

#include <cmath>

// FPGA stuff
#include <new>

// libraries used by DMA test code and GIPO test code
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// libraries used by DMA test code
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

// libraries used by GPIO test code
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>

// logging
#include <glog/logging.h>

// string manipulation
#include <string>

// constants
#define PAGE_SIZE 0x10000
#define TEST_REGISTER_TRACK_WRITEVAL 0x55AA

fpga_switch::fpga_switch(std::string device_name)
{
    if ((d_device_descriptor = open(device_name.c_str(), O_RDWR | O_SYNC)) == -1)
        {
            LOG(WARNING) << "Cannot open deviceio" << device_name;
            printf("switch memory successfully mapped\n");
        }
    d_map_base = reinterpret_cast<volatile unsigned *>(mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, d_device_descriptor, 0));

    if (d_map_base == reinterpret_cast<void*>(-1))
        {
            LOG(WARNING) << "Cannot map the FPGA switch module into tracking memory";
            printf("could not map switch memory\n");
        }

    // sanity check : check test register
    unsigned writeval = TEST_REGISTER_TRACK_WRITEVAL;
    unsigned readval;
    readval = fpga_switch::fpga_switch_test_register(writeval);
    if (writeval != readval)
        {
            LOG(WARNING) << "Test register sanity check failed";
        }
    else
        {
            LOG(INFO) << "Test register sanity check success !";
        }

    DLOG(INFO) << "Switch FPGA class created";
}

fpga_switch::~fpga_switch()
{
    close_device();
}

void fpga_switch::set_switch_position(int switch_position)
{
    d_map_base[0] = switch_position;
}

unsigned fpga_switch::fpga_switch_test_register(
        unsigned writeval)
{
    unsigned readval;
    // write value to test register
    d_map_base[3] = writeval;
    // read value from test register
    readval = d_map_base[3];
    // return read value
    return readval;
}

void fpga_switch::close_device()
{
    unsigned * aux = const_cast<unsigned*>(d_map_base);
    if (munmap(static_cast<void*>(aux), PAGE_SIZE) == -1)
        {
            printf("Failed to unmap memory uio\n");
        }

    close(d_device_descriptor);
}
    
