#include "led.h"
#include <string.h>
#include <switch.h>

#include "util.h"

// Breathing effect LED pattern
static const HidsysNotificationLedPattern breathing_pattern = {
    .baseMiniCycleDuration = 0x8, // 100ms.
    .totalMiniCycles = 0x2,       // 3 mini cycles. Last one 12.5ms.
    .totalFullCycles = 0x5,       // 5 full cycles.
    .startIntensity = 0x2,        // 13%.
    .miniCycles = {
        // First cycle
        {
            .ledIntensity = 0xF,      // 100%.
            .transitionSteps = 0xF,   // 15 steps. Transition time 1.5s.
            .finalStepDuration = 0x0, // 12.5ms.
        },
        // Second cycle
        {
            .ledIntensity = 0x2,      // 13%.
            .transitionSteps = 0xF,   // 15 steps. Transition time 1.5s.
            .finalStepDuration = 0x0, // 12.5ms.
        },
    },
};

static void send_led_pattern(const HidsysNotificationLedPattern* pattern)
{
    if (!serviceIsActive(hidsysGetServiceSession()))
        return;

    s32 total_entries = 0;
    HidsysUniquePadId uniquePadIds[2];

    const Result rc = hidsysGetUniquePadsFromNpad(isHidHandheld() ? HidNpadIdType_Handheld : HidNpadIdType_No1, uniquePadIds, 2, &total_entries);
    // A controller/LED service failure must not terminate the FTP server.
    if (R_FAILED(rc))
        return;

    for (int i = 0; i < total_entries && i < 2; i++)
        hidsysSetNotificationLedPattern(pattern, uniquePadIds[i]);
}

void flash_led_connect()
{
    send_led_pattern(&breathing_pattern);
}

void flash_led_disconnect()
{
    HidsysNotificationLedPattern pattern;
    memset(&pattern, 0, sizeof(pattern));

    send_led_pattern(&pattern);
}

