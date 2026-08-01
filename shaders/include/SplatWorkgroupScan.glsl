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

    for(
        uint offset = (WORKGROUP_SCAN_LOCAL_SIZE >> 1);
        offset > 0;
        offset >>= 1
    ) {
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
