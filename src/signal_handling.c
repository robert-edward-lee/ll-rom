#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "service_printing.h"
#include "signal_handling.h"

#ifndef _WIN32
#include <linux/version.h>

static void signal_handler(int signal, siginfo_t *signalInfo, void *userContext) {
    (void)userContext;

    if(signal & SIGSEGV) {
        switch(signalInfo->si_code) {
            case SEGV_MAPERR:
                PrintWarn("Address not mapped to object(%s)", QUOTE(SEGV_MAPERR));
                break;
            case SEGV_ACCERR:
                PrintWarn("Invalid permissions for mapped object(%s)", QUOTE(SEGV_ACCERR));
                break;
            case SEGV_BNDERR:
                PrintWarn("Bounds checking failure(%s)", QUOTE(SEGV_BNDERR));
                break;
            case SEGV_PKUERR:
                PrintWarn("Protection key checking failure(%s)", QUOTE(SEGV_PKUERR));
                break;
            case SEGV_ACCADI:
                PrintWarn("ADI not enabled for mapped object(%s)", QUOTE(SEGV_ACCADI));
                break;
            case SEGV_ADIDERR:
                PrintWarn("Disrupting MCD error(%s)", QUOTE(SEGV_ADIDERR));
                break;
            case SEGV_ADIPERR:
                PrintWarn("Precise MCD exception(%s)", QUOTE(SEGV_ADIPERR));
                break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
            case SEGV_MTEAERR:
                PrintWarn("Async MTE fault(%s)", QUOTE(SEGV_MTEAERR));
                break;
            case SEGV_MTESERR:
                PrintWarn("Sync MTE tag check fault(%s)", QUOTE(SEGV_MTESERR));
                break;
#endif
            default:
                PrintWarn("Unknown error code: (%d)", signalInfo->si_code);
                break;
        }
        PrintWarn("Address of faulting memory reference: %p", signalInfo->si_addr);
    }
    exit(EXIT_FAILURE);
}
#else
static void signal_handler(int signal) {
    if(signal & SIGSEGV) {
        PrintWarn("Segmentation fault");
    }
    exit(EXIT_FAILURE);
}
#endif

void register_signal_handler(void) {
#ifdef _WIN32
    signal(SIGSEGV, signal_handler);
#else
    struct sigaction signal_action;

    memset(&signal_action, 0, sizeof(signal_action));
    sigaddset(&signal_action.sa_mask, SIGSEGV);
    signal_action.sa_sigaction = signal_handler;
    signal_action.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &signal_action, NULL);
#endif
}
