#include "config.h"

#include <stdio.h>
#include <string.h>
#include <pal.h>
#include <common.h>
#include "pal_base_private.h"
#include "devices/devices.h"
#if __epiphany__
#include "devices/posix/epiphany/ctrl.h"
#endif

__attribute__((constructor)) void __pal_init(void);
__attribute__((destructor)) void __pal_fini(void);

/* Defining the table this way statically compile time depends on devs being in
 * the right order.  If we We could do this in the constructor
 * too... */
#ifdef __epiphany__
struct pal_global __pal_global __attribute__ ((section (".data_bank0"))) = {
#else
struct pal_global __pal_global = {
#endif
    .devs = {
        NULL,
#if ENABLE_DEV_EPIPHANY
        &__pal_dev_epiphany.dev,
#else
        NULL,
#endif
#if ENABLE_DEV_GRID
        &__pal_dev_grid.dev,
#else
        NULL,
#endif
#if ENABLE_DEV_SMP
        &__pal_dev_smp.dev,
#else
        NULL,
#endif
#if ENABLE_DEV_FPGA
        &__pal_dev_fpga.dev,
#else
        NULL,
#endif
#if ENABLE_DEV_GPU
        &__pal_dev_gpu.dev,
#else
        NULL,
#endif
#if ENABLE_DEV_DEMO
        &__pal_dev_demo.dev,
#else
        NULL,
#endif
    },
    .teams_head = NULL,
    .teams_tail = NULL,
    .progs_head = NULL,
    .progs_tail = NULL,

/* We might want to use another PRNG for some platforms */
#ifdef TINYMT32_H
    .random = {
        .status = {0},
        .mat1 = 0,
        .mat2 = 0,
        .tmat = 0
    }
#endif
};

#ifdef __epiphany__
struct pal_epiphany_coords __pal_epiphany_coords = {
    .default_dev_start = { .row = 32, .col = 8 },
    .default_dev_size = { .row = 4, .col = 4 },
    .default_team_topology = P_TOPOLOGY_FLAT,
    .default_team_start = { .row = 0, .col = 0 },
    .default_team_size = { .row = 1, .col = 1 },
    .default_team_rank = { 0 },
    .device_rank = 0,
    .eram_base = 0x8e000000,
    .eram_size = 0x02000000,
};
#endif

static void early_device_init()
{
    for (int i = 0; i < ARRAY_SIZE(__pal_global.devs); i++) {
        struct dev *dev = __pal_global.devs[i];
        if (dev && dev->dev_ops->early_init)
            dev->dev_ops->early_init(dev);
    }
}

static void late_device_fini()
{
    for (int i = 0; i < ARRAY_SIZE(__pal_global.devs); i++) {
        struct dev *dev = __pal_global.devs[i];
        if (dev && dev->dev_ops->late_fini)
            dev->dev_ops->late_fini(dev);
    }
}

#ifdef ENABLE_DEV_EPIPHANY_SIM
static void detect_epiphany_simulator()
{
    char *p;
    bool simulator;

    p = getenv("PAL_EPIPHANY_TARGET");

    if (p && strncmp(p, "device", sizeof("device")) == 0)
        return;

    if (p && strncmp(p, "sim", sizeof("sim")) == 0)
        goto setup_simulator;

    /* E-HAL backwards compatibility */
    p = getenv("EHAL_TARGET");
    if (p && strncmp(p, "esim", sizeof("esim")) == 0)
        goto setup_simulator;

    if (p && strncmp(p, "sim", sizeof("sim")) == 0)
        goto setup_simulator;

    return;

setup_simulator:
    /* Patch device structure with simulator ops */
    __pal_dev_epiphany.dev.dev_ops = &__pal_dev_epiphany_sim_ops;
}
#else
static void detect_epiphany_simulator()
{
}
#endif

__attribute__((constructor))
void __pal_init()
{
#if __epiphany__
    /* Platform specifics should probably go into separate file ? */
    // Assume team is entire chip
    struct epiphany_ctrl_mem *ctrl =
        (struct epiphany_ctrl_mem *) CTRL_MEM_EADDR;

    __pal_dev_epiphany.dev.start = __pal_epiphany_coords.default_dev_start;
    __pal_dev_epiphany.dev.size = __pal_epiphany_coords.default_dev_size;
    __pal_dev_epiphany.eram_base = __pal_epiphany_coords.eram_base;
    __pal_dev_epiphany.eram_size = __pal_epiphany_coords.eram_size;
    __pal_global.default_team.start = __pal_epiphany_coords.default_team_start;
    __pal_global.default_team.size = __pal_epiphany_coords.default_team_size;
    __pal_global.default_team.rank = __pal_epiphany_coords.default_team_rank;
    __pal_global.default_team.topology = __pal_epiphany_coords.default_team_topology;
    __pal_global.default_team.dev = &__pal_dev_epiphany.dev;

    ctrl->status[__pal_epiphany_coords.device_rank] = STATUS_RUNNING;
#else
    detect_epiphany_simulator();
    early_device_init();
#endif
}

__attribute__((destructor))
void __pal_fini()
{
#if __epiphany__
    struct epiphany_ctrl_mem *ctrl =
        (struct epiphany_ctrl_mem *) CTRL_MEM_EADDR;
    ctrl->status[__pal_epiphany_coords.device_rank] = STATUS_DONE;
#else
    struct team *team, *next_team;
    struct prog *prog, *next_prog;

    team = __pal_global.teams_head;
    while (team) {
        next_team = team->next;
        // team_fini(team);
        team = next_team;
    }

    prog = __pal_global.progs_head;
    while (prog) {
        next_prog = prog->next;
        // prog_fini(prog);
        prog = next_prog;
    }

    late_device_fini();

    memset(&__pal_global, 0, sizeof(struct pal_global));
#endif
}

