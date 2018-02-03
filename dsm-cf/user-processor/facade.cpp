/**
 * Partov is a simulation engine, supporting emulation as well,
 * making it possible to create virtual networks.
 *  
 * Copyright © 2009-2015 Behnam Momeni.
 * 
 * This file is part of the Partov.
 * 
 * Partov is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Partov is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Partov.  If not, see <http://www.gnu.org/licenses/>.
 *  
 */

#include "sm.h"
 
// These functions allow base folder to use user folder without compile-time dependencies

void SimulatedMachineParseArguments (int argc, char *argv[]) {
  SimulatedMachine::parseArguments (argc, argv);
}

Machine *instantiateSimulatedMachine (const ClientFramework *cf, int count) {
  return new SimulatedMachine (cf, count);
}

uint64_t readMainMemory (uint16_t address)
{
	uint64_t new_address = (uint64_t)address;
	uint64_t data = new_address * new_address * new_address * new_address;
	return data;
}

