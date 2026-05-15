/*
 * test.c
 *
 *  Created on: 15. mai 2026
 *      Author: dagak
 */
#include "main.h"
void testGPIOA() {

}
void debug_PA2_configuration(void) {
    printf("=== PA2 & Timer 2 Channel 3 Configuration Debug ===\r\n");

    // Check GPIOA MODER for PA2 (bits 5:4)
    uint32_t moder = GPIOA->MODER;
    uint32_t moder_pa2 = (moder >> 4) & 0x3;
    printf("GPIOA->MODER (bits 5:4): 0x%lX\r\n", moder_pa2);
    if (moder_pa2 == 2) {
        printf("  ✓ PA2 is in AF (Alternate Function) mode\r\n");
    } else {
        printf("  ✗ ERROR: PA2 is NOT in AF mode (value: %lu)\r\n", moder_pa2);
    }

    // Check GPIOA AFRL for PA2 AF selection (bits 11:8)
    uint32_t afr = *GPIOA->AFR;
    uint32_t af_pa2 = (afr >> 8) & 0xF;
    printf("GPIOA->AFRL[11:8] (PA2 AF): 0x%lX\r\n", af_pa2);
    if (af_pa2 == 1) {
        printf("  ✓ PA2 is configured for AF1 (Timer 2)\r\n");
    } else {
        printf("  ✗ ERROR: PA2 AF is incorrect (value: %lu, expected 1)\r\n", af_pa2);
    }

    // Check Timer 2 Channel 3 PWM Mode (CCMR2 bits 6:4)
    uint32_t ccmr2 = TIM2->CCMR2;
    uint32_t pwm_mode = (ccmr2 >> 4) & 0x7;
    printf("TIM2->CCMR2[6:4] (PWM Mode): 0x%lX\r\n", pwm_mode);
    if (pwm_mode == 6) {
        printf("  ✓ Channel 3 is in PWM Mode 1\r\n");
    } else {
        printf("  ✗ ERROR: Channel 3 PWM mode incorrect (value: %lu, expected 6)\r\n", pwm_mode);
    }

    // Check Channel 3 Output Enable (CCER bit 8 = CC3E)
    uint32_t ccer = TIM2->CCER;
    uint32_t cc3e = (ccer >> 8) & 0x1;
    printf("TIM2->CCER[8] (CC3E - Channel 3 Enable): %lu\r\n", cc3e);
    if (cc3e == 1) {
        printf("  ✓ Channel 3 output is enabled\r\n");
    } else {
        printf("  ✗ ERROR: Channel 3 output is disabled\r\n");
        printf("CCER: %lu\r\n", ccer);
    }

    // Check Timer 2 Enable (CR1 bit 0 = CEN)
    uint32_t cr1 = TIM2->CR1;
    uint32_t cen = cr1 & 0x1;
    printf("TIM2->CR1[0] (CEN - Counter Enable): %lu\r\n", cen);
    if (cen == 1) {
        printf("  ✓ Timer 2 counter is running\r\n");
    } else {
        printf("  ✗ ERROR: Timer 2 counter is not running\r\n");
    }

    // PWM Parameters
    printf("\r\n=== PWM Parameters ===\r\n");
    printf("TIM2->PSC (Prescaler): %lu\r\n", TIM2->PSC);
    printf("TIM2->ARR (Auto-reload/Period): %lu\r\n", TIM2->ARR);
    printf("TIM2->CCR3 (Capture/Compare - Duty): %lu\r\n", TIM2->CCR3);

    uint32_t counter_freq = 84000000 / (TIM2->PSC + 1);
    uint32_t pwm_freq = counter_freq / (TIM2->ARR + 1);
    uint32_t duty_cycle = (TIM2->CCR3 * 100) / (TIM2->ARR + 1);

    printf("Counter Frequency: %lu Hz\r\n", counter_freq);
    printf("PWM Frequency: %lu Hz\r\n", pwm_freq);
    printf("Duty Cycle: %lu%%\r\n", duty_cycle);

    printf("\r\n=== Summary ===\r\n");
    if (moder_pa2 == 2 && af_pa2 == 1 && pwm_mode == 6 && cc3e == 1 && cen == 1) {
        printf("✓ All checks passed! PA2 is correctly configured for Timer 2 Channel 3 PWM\r\n");
    } else {
        printf("✗ Some checks failed. Review the errors above.\r\n");
    }
}
