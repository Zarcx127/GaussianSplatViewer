/**
 * Copyright (C) 2026  Zarcx127@github.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

#ifndef SPLAT_WORKGROUP_SCAN_GLSL_H
#define SPLAT_WORKGROUP_SCAN_GLSL_H

#ifdef WORKGROUP_SCAN_LOCAL_SIZE

shared uint scanValues[WORKGROUP_SCAN_LOCAL_SIZE];

void workgroup_scan_upsweep()
{
    uint localIndex = gl_LocalInvocationIndex;

    for(uint offset = 1; offset < WORKGROUP_SCAN_LOCAL_SIZE; offset <<= 1)
    {
        uint index = (((localIndex + 1) * offset * 2) - 1);
        if(index < WORKGROUP_SCAN_LOCAL_SIZE)
            scanValues[index] += scanValues[index - offset];

        barrier();
    }
}

void workgroup_scan_downsweep()
{
    uint localIndex = gl_LocalInvocationIndex;

    for(uint offset = (WORKGROUP_SCAN_LOCAL_SIZE >> 1); offset > 0; offset >>= 1) 
    {
        uint index = (((localIndex + 1) * offset * 2) - 1);
        if(index < WORKGROUP_SCAN_LOCAL_SIZE)
        {
            uint leftValue = scanValues[index - offset];

            scanValues[index - offset] = scanValues[index];
            scanValues[index] += leftValue;
        }

        barrier();
    }
}

#endif

#endif
