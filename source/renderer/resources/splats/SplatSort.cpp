#include "renderer/resources/splats/SplatSort.hpp"

bool splat_sort_resources_are_valid(const SplatSortResources& resources) 
{
    return (
        resources.radixHistograms.buffer &&
        resources.radixScanBlockSums.buffer &&
        resources.radixBucketOffsets.buffer &&
        resources.dispatchCommands.buffer &&
        (resources.workgroupCapacity > 0) &&
        (resources.histogramCapacity > 0) &&
        (resources.histogramBlockCapacity > 0) &&
        (resources.scanBlockCapacityPerBucket > 0)
    );
}
