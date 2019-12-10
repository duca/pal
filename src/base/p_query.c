#include <pal.h>

#include "pal_base.h"
#include "pal_base_private.h"

/**
 *
 * Queries a PAL device,returning an integer
 * The object can be of type pal_dev_t, pal_mem_t, or pal_team_t.
 * Valid queries should never return '0'.
 *
 * @todo Currently we only support querying devices.
 *
 * @param dev  Device to query
 *
 * @param prop Property to query
 *
 *        TYPE     - Device type
 *                   1:EPIPHANY
 *                   2:GRID
 *                   3:SMP
 *                   4:FPGA
 *                   5:GPU
 *        ISA      - Instruction Set Architecture
 *                   1:EPIPHANY
 *                   2:x86
 *                   3:ARM
 *                   4:MIPS
 *        OS       - Operating System
 *                   1:Bare metal (CRT0)
 *                   2:Linux
 *        MEMARCH  - Memory architecture
 *                   1:32 bits
 *                   2:64 bits
 *        TOPOLOGY - Network topology of device
 *                   1:Point to point
 *                   2:Bus
 *                   3:Star
 *                   4:Ring
 *                   5:Mesh
 *                   6:Hypercube
 *                   7:Tree
 *        NODES    - Number of nodes in device
 *        ROWS     - Number of rows in topology
 *        COLS     - Number of cols in topology
 *        PLANES   - Number of planes in topology
 *        CHIPROWS - Number of rows on a single chip
 *        CHIPCOLS - Number of cols on a single chip
 *        SIMD     - Vector size at each node
 *        MEMSIZE  - Local memory size at each node
 *        MEMBASE  - Memory offset of first node
 *        VERSION  - Unique platform version "tag"
 *        WHOAMI   - Returns index of current node
 *
 * @return      Value of property being queried
 *
 */
int p_query(p_dev_t dev, int prop)
{
    struct dev *pdev;

    if (p_ref_is_err(dev))
        return -EINVAL;

    pdev = (struct dev *) dev;
    return pdev->dev_ops->query(pdev, prop);
}
