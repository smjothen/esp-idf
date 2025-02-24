/*
 * SPDX-FileCopyrightText: 2015-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*******************************************************************************
 * NOTICE
 * The hal is not public api, don't use in application code.
 * See readme.md in hal/include/hal/readme.md
 ******************************************************************************/

// The LL layer for ESP32 GPIO register operations

#pragma once

#include <stdbool.h>
#include "soc/soc.h"
#include "soc/gpio_periph.h"
#include "soc/gpio_struct.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "hal/gpio_types.h"
#include "hal/misc.h"

#ifdef __cplusplus
extern "C" {
#endif

// the address of esp32's IO_MUX_GPIOx_REGs are not incremented as the gpio number increments(address are out of order)
extern const uint8_t GPIO_PIN_MUX_REG_OFFSET[SOC_GPIO_PIN_COUNT];

// Get GPIO hardware instance with giving gpio num
#define GPIO_LL_GET_HW(num) (((num) == 0) ? (&GPIO) : NULL)

// the gpio matrix signal routing const time
#define GPIO_LL_MATRIX_DELAY_NS       25

#define GPIO_LL_APP_CPU_INTR_ENA      (BIT(0))
#define GPIO_LL_APP_CPU_NMI_INTR_ENA  (BIT(1))
#define GPIO_LL_PRO_CPU_INTR_ENA      (BIT(2))
#define GPIO_LL_PRO_CPU_NMI_INTR_ENA  (BIT(3))
#define GPIO_LL_SDIO_EXT_INTR_ENA     (BIT(4))

/**
 * @brief Get the configuration for an IO
 *
 * @param hw Peripheral GPIO hardware instance address.
 * @param gpio_num GPIO number
 * @param[out] io_config Pointer to the structure that saves the specific IO configuration
 */
static inline void gpio_ll_get_io_config(gpio_dev_t *hw, uint32_t gpio_num, gpio_io_config_t *io_config)
{
    uint32_t bit_shift = (gpio_num < 32) ? gpio_num : (gpio_num - 32);
    uint32_t bit_mask = 1 << bit_shift;
    uint32_t iomux_reg_val = REG_READ(GPIO_PIN_MUX_REG[gpio_num]);
    io_config->pu = (iomux_reg_val & FUN_PU_M) >> FUN_PU_S;
    io_config->pd = (iomux_reg_val & FUN_PD_M) >> FUN_PD_S;
    io_config->ie = (iomux_reg_val & FUN_IE_M) >> FUN_IE_S;
    io_config->oe = (((gpio_num < 32) ? hw->enable : hw->enable1.val) & bit_mask) >> bit_shift;
    io_config->oe_ctrl_by_periph = !(hw->func_out_sel_cfg[gpio_num].oen_sel);
    io_config->oe_inv = hw->func_out_sel_cfg[gpio_num].oen_inv_sel;
    io_config->od = hw->pin[gpio_num].pad_driver;
    io_config->drv = (gpio_drive_cap_t)((iomux_reg_val & FUN_DRV_M) >> FUN_DRV_S);
    io_config->fun_sel = (iomux_reg_val & MCU_SEL_M) >> MCU_SEL_S;
    io_config->sig_out = hw->func_out_sel_cfg[gpio_num].func_sel;
    io_config->slp_sel = (iomux_reg_val & SLP_SEL_M) >> SLP_SEL_S;
}

/**
  * @brief Enable pull-up on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
static inline void gpio_ll_pullup_en(gpio_dev_t *hw, uint32_t gpio_num)
{
    REG_SET_BIT(GPIO_PIN_MUX_REG[gpio_num], FUN_PU);
}

/**
  * @brief Disable pull-up on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_pullup_dis(gpio_dev_t *hw, uint32_t gpio_num)
{
    REG_CLR_BIT(DR_REG_IO_MUX_BASE + GPIO_PIN_MUX_REG_OFFSET[gpio_num], FUN_PU);
}

/**
  * @brief Return pull-up status on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  * @return if GPIO gpio_num`s FUN_PU is true
  */
static inline bool gpio_ll_pullup_is_enabled(gpio_dev_t *hw, uint32_t gpio_num)
{
    return REG_GET_BIT(GPIO_PIN_MUX_REG[gpio_num], FUN_PU) ? true : false;
}

/**
  * @brief Enable pull-down on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
static inline void gpio_ll_pulldown_en(gpio_dev_t *hw, uint32_t gpio_num)
{
    REG_SET_BIT(GPIO_PIN_MUX_REG[gpio_num], FUN_PD);
}

/**
  * @brief Disable pull-down on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_pulldown_dis(gpio_dev_t *hw, uint32_t gpio_num)
{
    REG_CLR_BIT(DR_REG_IO_MUX_BASE + GPIO_PIN_MUX_REG_OFFSET[gpio_num], FUN_PD);
}

/**
  * @brief Return pull-down status on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  * @return if GPIO gpio_num`s FUN_PD is true
  */
static inline bool gpio_ll_pulldown_is_enabled(gpio_dev_t *hw, uint32_t gpio_num)
{
    return REG_GET_BIT(GPIO_PIN_MUX_REG[gpio_num], FUN_PD) ? true : false;
}

/**
  * @brief Enable GPIO pin used for wakeup from sleep.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_sleep_sel_en(gpio_dev_t *hw, uint32_t gpio_num)
{
    PIN_SLP_SEL_ENABLE(GPIO_PIN_MUX_REG[gpio_num]);
}

/**
  * @brief Disable GPIO pin used for wakeup from sleep.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_sleep_sel_dis(gpio_dev_t *hw, uint32_t gpio_num)
{
    PIN_SLP_SEL_DISABLE(GPIO_PIN_MUX_REG[gpio_num]);
}

/**
  * @brief Return slp-sel status on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  * @return if GPIO gpio_num`s SLP_SEL is true
  */
__attribute__((always_inline))
static inline bool gpio_ll_sleep_sel_is_enabled(gpio_dev_t *hw, uint32_t gpio_num)
{
    return REG_GET_BIT(GPIO_PIN_MUX_REG[gpio_num], SLP_SEL) ? true : false;
}

/**
  * @brief Disable GPIO pull-up in sleep mode.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_sleep_pullup_dis(gpio_dev_t *hw, uint32_t gpio_num)
{
    PIN_SLP_PULLUP_DISABLE(GPIO_PIN_MUX_REG[gpio_num]);
}

/**
  * @brief Enable GPIO pull-up in sleep mode.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_sleep_pullup_en(gpio_dev_t *hw, uint32_t gpio_num)
{
    PIN_SLP_PULLUP_ENABLE(GPIO_PIN_MUX_REG[gpio_num]);
}

/**
  * @brief Return slp-pull-up status on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  * @return if GPIO gpio_num`s SLP_PU is true
  */
__attribute__((always_inline))
static inline bool gpio_ll_sleep_pullup_is_enabled(gpio_dev_t *hw, uint32_t gpio_num)
{
    return REG_GET_BIT(GPIO_PIN_MUX_REG[gpio_num], SLP_PU) ? true : false;
}

/**
  * @brief Enable GPIO pull-down in sleep mode.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_sleep_pulldown_en(gpio_dev_t *hw, uint32_t gpio_num)
{
    PIN_SLP_PULLDOWN_ENABLE(GPIO_PIN_MUX_REG[gpio_num]);
}

/**
  * @brief Disable GPIO pull-down in sleep mode.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_sleep_pulldown_dis(gpio_dev_t *hw, uint32_t gpio_num)
{
    PIN_SLP_PULLDOWN_DISABLE(GPIO_PIN_MUX_REG[gpio_num]);
}

/**
  * @brief Return slp-pull-down status on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  * @return if GPIO gpio_num`s SLP_PD is true
  */
__attribute__((always_inline))
static inline bool gpio_ll_sleep_pulldown_is_enabled(gpio_dev_t *hw, uint32_t gpio_num)
{
    return REG_GET_BIT(GPIO_PIN_MUX_REG[gpio_num], SLP_PD) ? true : false;
}

/**
 * @brief  GPIO set interrupt trigger type
 *
 * @param  hw Peripheral GPIO hardware instance address.
 * @param  gpio_num GPIO number. If you want to set the trigger type of e.g. of GPIO16, gpio_num should be GPIO_NUM_16 (16);
 * @param  intr_type Interrupt type, select from gpio_int_type_t
 */
static inline void gpio_ll_set_intr_type(gpio_dev_t *hw, uint32_t gpio_num, gpio_int_type_t intr_type)
{
    hw->pin[gpio_num].int_type = intr_type;
}

/**
  * @brief Get GPIO interrupt status
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param core_id interrupt core id
  * @param status interrupt status
  */
__attribute__((always_inline))
static inline void gpio_ll_get_intr_status(gpio_dev_t *hw, uint32_t core_id, uint32_t *status)
{
    *status = (core_id == 0) ? hw->pcpu_int : hw->acpu_int;
}

/**
  * @brief Get GPIO interrupt status high
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param core_id interrupt core id
  * @param status interrupt status high
  */
__attribute__((always_inline))
static inline void gpio_ll_get_intr_status_high(gpio_dev_t *hw, uint32_t core_id, uint32_t *status)
{
    *status = (core_id == 0) ? HAL_FORCE_READ_U32_REG_FIELD(hw->pcpu_int1, intr) : HAL_FORCE_READ_U32_REG_FIELD(hw->acpu_int1, intr);
}

/**
  * @brief Clear GPIO interrupt status
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param mask interrupt status clear mask
  */
__attribute__((always_inline))
static inline void gpio_ll_clear_intr_status(gpio_dev_t *hw, uint32_t mask)
{
    hw->status_w1tc = mask;
}

/**
  * @brief Clear GPIO interrupt status high
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param mask interrupt status high clear mask
  */
__attribute__((always_inline))
static inline void gpio_ll_clear_intr_status_high(gpio_dev_t *hw, uint32_t mask)
{
    HAL_FORCE_MODIFY_U32_REG_FIELD(hw->status1_w1tc, intr_st, mask);
}

/**
 * @brief  Enable GPIO module interrupt signal
 *
 * @param  hw Peripheral GPIO hardware instance address.
 * @param  core_id Interrupt enabled CPU to corresponding ID
 * @param  gpio_num GPIO number. If you want to enable the interrupt of e.g. GPIO16, gpio_num should be GPIO_NUM_16 (16);
 */
__attribute__((always_inline))
static inline void gpio_ll_intr_enable_on_core(gpio_dev_t *hw, uint32_t core_id, uint32_t gpio_num)
{
    if (core_id == 0) {
        hw->pin[gpio_num].int_ena = GPIO_LL_PRO_CPU_INTR_ENA;     //enable pro cpu intr
    } else {
        hw->pin[gpio_num].int_ena = GPIO_LL_APP_CPU_INTR_ENA;     //enable app cpu intr
    }
}

/**
 * @brief  Disable GPIO module interrupt signal
 *
 * @param  hw Peripheral GPIO hardware instance address.
 * @param  gpio_num GPIO number. If you want to disable the interrupt of e.g. GPIO16, gpio_num should be GPIO_NUM_16 (16);
 */
__attribute__((always_inline))
static inline void gpio_ll_intr_disable(gpio_dev_t *hw, uint32_t gpio_num)
{
    hw->pin[gpio_num].int_ena = 0;                             //disable GPIO intr
}

/**
  * @brief Disable input mode on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_input_disable(gpio_dev_t *hw, uint32_t gpio_num)
{
    PIN_INPUT_DISABLE(DR_REG_IO_MUX_BASE + GPIO_PIN_MUX_REG_OFFSET[gpio_num]);
}

/**
  * @brief Enable input mode on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_input_enable(gpio_dev_t *hw, uint32_t gpio_num)
{
    PIN_INPUT_ENABLE(DR_REG_IO_MUX_BASE + GPIO_PIN_MUX_REG_OFFSET[gpio_num]);
}

/**
  * @brief Disable output mode on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_output_disable(gpio_dev_t *hw, uint32_t gpio_num)
{
    if (gpio_num < 32) {
        hw->enable_w1tc = (0x1 << gpio_num);
    } else {
        HAL_FORCE_MODIFY_U32_REG_FIELD(hw->enable1_w1tc, data, (0x1 << (gpio_num - 32)));
    }
}

/**
  * @brief Enable output mode on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_output_enable(gpio_dev_t *hw, uint32_t gpio_num)
{
    if (gpio_num < 32) {
        hw->enable_w1ts = (0x1 << gpio_num);
    } else {
        HAL_FORCE_MODIFY_U32_REG_FIELD(hw->enable1_w1ts, data, (0x1 << (gpio_num - 32)));
    }
}

/**
  * @brief Disable GPIO input in sleep mode.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_sleep_input_disable(gpio_dev_t *hw, uint32_t gpio_num)
{
    PIN_SLP_INPUT_DISABLE(GPIO_PIN_MUX_REG[gpio_num]);
}

/**
  * @brief Enable GPIO input in sleep mode.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_sleep_input_enable(gpio_dev_t *hw, uint32_t gpio_num)
{
    PIN_SLP_INPUT_ENABLE(GPIO_PIN_MUX_REG[gpio_num]);
}

/**
  * @brief Disable GPIO output in sleep mode.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_sleep_output_disable(gpio_dev_t *hw, uint32_t gpio_num)
{
    PIN_SLP_OUTPUT_DISABLE(GPIO_PIN_MUX_REG[gpio_num]);
}

/**
  * @brief Enable GPIO output in sleep mode.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_sleep_output_enable(gpio_dev_t *hw, uint32_t gpio_num)
{
    PIN_SLP_OUTPUT_ENABLE(GPIO_PIN_MUX_REG[gpio_num]);
}

/**
  * @brief Disable open-drain mode on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
__attribute__((always_inline))
static inline void gpio_ll_od_disable(gpio_dev_t *hw, uint32_t gpio_num)
{
    hw->pin[gpio_num].pad_driver = 0;
}

/**
  * @brief Enable open-drain mode on GPIO.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number
  */
static inline void gpio_ll_od_enable(gpio_dev_t *hw, uint32_t gpio_num)
{
    hw->pin[gpio_num].pad_driver = 1;
}

/**
 * @brief Disconnect any peripheral output signal routed via GPIO matrix to the pin
 *
 * @param  hw Peripheral GPIO hardware instance address.
 * @param  gpio_num GPIO number
 */
__attribute__((always_inline))
static inline void gpio_ll_matrix_out_default(gpio_dev_t *hw, uint32_t gpio_num)
{
    REG_WRITE(GPIO_FUNC0_OUT_SEL_CFG_REG + (gpio_num * 4), SIG_GPIO_OUT_IDX);
}

/**
 * @brief  Select a function for the pin in the IOMUX
 *
 * @param  hw Peripheral GPIO hardware instance address.
 * @param  gpio_num GPIO number
 * @param  func Function to assign to the pin
 */
__attribute__((always_inline))
static inline void gpio_ll_func_sel(gpio_dev_t *hw, uint8_t gpio_num, uint32_t func)
{
    PIN_FUNC_SELECT(DR_REG_IO_MUX_BASE + GPIO_PIN_MUX_REG_OFFSET[gpio_num], func);
}

/**
 * @brief  GPIO set output level
 *
 * @param  hw Peripheral GPIO hardware instance address.
 * @param  gpio_num GPIO number. If you want to set the output level of e.g. GPIO16, gpio_num should be GPIO_NUM_16 (16);
 * @param  level Output level. 0: low ; 1: high
 */
__attribute__((always_inline))
static inline void gpio_ll_set_level(gpio_dev_t *hw, uint32_t gpio_num, uint32_t level)
{
    if (level) {
        if (gpio_num < 32) {
            hw->out_w1ts = 1 << gpio_num;
        } else {
            hw->out1_w1ts.val = 1 << (gpio_num - 32);
        }
    } else {
        if (gpio_num < 32) {
            hw->out_w1tc = 1 << gpio_num;
        } else {
            hw->out1_w1tc.val = 1 << (gpio_num - 32);
        }
    }
}

/**
 * @brief  GPIO get input level
 *
 * @warning If the pad is not configured for input (or input and output) the returned value is always 0.
 *
 * @param  hw Peripheral GPIO hardware instance address.
 * @param  gpio_num GPIO number. If you want to get the logic level of e.g. pin GPIO16, gpio_num should be GPIO_NUM_16 (16);
 *
 * @return
 *     - 0 the GPIO input level is 0
 *     - 1 the GPIO input level is 1
 */
__attribute__((always_inline))
static inline int gpio_ll_get_level(gpio_dev_t *hw, uint32_t gpio_num)
{
    if (gpio_num < 32) {
        return (hw->in >> gpio_num) & 0x1;
    } else {
        return (HAL_FORCE_READ_U32_REG_FIELD(hw->in1, data) >> (gpio_num - 32)) & 0x1;
    }
}

/**
 * @brief Enable GPIO wake-up function.
 *
 * @param hw Peripheral GPIO hardware instance address.
 * @param gpio_num GPIO number.
 */
static inline void gpio_ll_wakeup_enable(gpio_dev_t *hw, uint32_t gpio_num)
{
    hw->pin[gpio_num].wakeup_enable = 1;
}

/**
 * @brief Disable GPIO wake-up function.
 *
 * @param hw Peripheral GPIO hardware instance address.
 * @param gpio_num GPIO number
 */
static inline void gpio_ll_wakeup_disable(gpio_dev_t *hw, uint32_t gpio_num)
{
    hw->pin[gpio_num].wakeup_enable = 0;
}

/**
  * @brief Set GPIO pad drive capability
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number, only support output GPIOs
  * @param strength Drive capability of the pad
  */
static inline void gpio_ll_set_drive_capability(gpio_dev_t *hw, uint32_t gpio_num, gpio_drive_cap_t strength)
{
    SET_PERI_REG_BITS(GPIO_PIN_MUX_REG[gpio_num], FUN_DRV_V, strength, FUN_DRV_S);
}

/**
  * @brief Get GPIO pad drive capability
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number, only support output GPIOs
  * @param strength Pointer to accept drive capability of the pad
  */
static inline void gpio_ll_get_drive_capability(gpio_dev_t *hw, uint32_t gpio_num, gpio_drive_cap_t *strength)
{
    *strength = (gpio_drive_cap_t)GET_PERI_REG_BITS2(GPIO_PIN_MUX_REG[gpio_num], FUN_DRV_V, FUN_DRV_S);
}

/**
  * @brief Enable all digital gpio pad hold function during Deep-sleep.
  *
  * @param hw Peripheral GPIO hardware instance address.
  */
static inline void gpio_ll_deep_sleep_hold_en(gpio_dev_t *hw)
{
    CLEAR_PERI_REG_MASK(RTC_CNTL_DIG_ISO_REG, RTC_CNTL_DG_PAD_FORCE_UNHOLD);
    SET_PERI_REG_MASK(RTC_CNTL_DIG_ISO_REG, RTC_CNTL_DG_PAD_AUTOHOLD_EN_M);
}

/**
  * @brief Disable all digital gpio pad hold function during Deep-sleep.
  *
  * @param hw Peripheral GPIO hardware instance address.
  */
static inline void gpio_ll_deep_sleep_hold_dis(gpio_dev_t *hw)
{
    CLEAR_PERI_REG_MASK(RTC_CNTL_DIG_ISO_REG, RTC_CNTL_DG_PAD_AUTOHOLD_EN_M);
}

/**
 * @brief  Get deep sleep hold status
 *
 * @param  hw Peripheral GPIO hardware instance address.
 *
 * @return
 *     - true  deep sleep hold is enabled
 *     - false deep sleep hold is disabled
 */
__attribute__((always_inline))
static inline bool gpio_ll_deep_sleep_hold_is_en(gpio_dev_t *hw)
{
    return !GET_PERI_REG_MASK(RTC_CNTL_DIG_ISO_REG, RTC_CNTL_DG_PAD_FORCE_UNHOLD) && GET_PERI_REG_MASK(RTC_CNTL_DIG_ISO_REG, RTC_CNTL_DG_PAD_AUTOHOLD_EN_M);
}

/**
  * @brief Enable gpio pad hold function.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number, only support output GPIOs
  */
static inline void gpio_ll_hold_en(gpio_dev_t *hw, uint32_t gpio_num)
{
    SET_PERI_REG_MASK(RTC_IO_DIG_PAD_HOLD_REG, GPIO_HOLD_MASK[gpio_num]);
}

/**
  * @brief Disable gpio pad hold function.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number, only support output GPIOs
  */
static inline void gpio_ll_hold_dis(gpio_dev_t *hw, uint32_t gpio_num)
{
    CLEAR_PERI_REG_MASK(RTC_IO_DIG_PAD_HOLD_REG, GPIO_HOLD_MASK[gpio_num]);
}

/**
  * @brief Get digital gpio pad hold status.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number, only support output GPIOs
  *
  * @note caller must ensure that gpio_num is a digital io pad
  *
  * @return
  *     - true  digital gpio pad is held
  *     - false digital gpio pad is unheld
  */
__attribute__((always_inline))
static inline bool gpio_ll_is_digital_io_hold(gpio_dev_t *hw, uint32_t gpio_num)
{
    uint32_t mask = 0;

    switch (gpio_num) {
    case 1: mask = BIT(1); break;
    case 3: mask = BIT(0); break;
    case 5: mask = BIT(8); break;
    case 6 ... 11 : mask = BIT(gpio_num - 4); break;
    case 16 ... 19:
    case 21 ... 23: mask = BIT(gpio_num - 7); break;
    default: break;
    }

    return GET_PERI_REG_MASK(RTC_IO_DIG_PAD_HOLD_REG, mask);
}

/**
  * @brief Configure peripheral signal input whether to bypass GPIO matrix.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param signal_idx Peripheral signal id to input. One of the ``*_IN_IDX`` signals in ``soc/gpio_sig_map.h``.
  * @param from_gpio_matrix True if not to bypass GPIO matrix, otherwise False.
  */
__attribute__((always_inline))
static inline void gpio_ll_set_input_signal_from(gpio_dev_t *hw, uint32_t signal_idx, bool from_gpio_matrix)
{
    hw->func_in_sel_cfg[signal_idx].sig_in_sel = from_gpio_matrix;
}

/**
  * @brief Configure the source of output enable signal for the GPIO pin.
  *
  * @param hw Peripheral GPIO hardware instance address.
  * @param gpio_num GPIO number of the pad.
  * @param ctrl_by_periph True if use output enable signal from peripheral, false if force the output enable signal to be sourced from bit n of GPIO_ENABLE_REG
  * @param oen_inv True if the output enable needs to be inverted, otherwise False.
  */
static inline void gpio_ll_set_output_enable_ctrl(gpio_dev_t *hw, uint8_t gpio_num, bool ctrl_by_periph, bool oen_inv)
{
    hw->func_out_sel_cfg[gpio_num].oen_inv_sel = oen_inv;       // control valid only when using gpio matrix to route signal to the IO
    hw->func_out_sel_cfg[gpio_num].oen_sel = !ctrl_by_periph;
}

/**
 * @brief  Control the pin in the IOMUX
 *
 * @param  bmap   write mask of control value
 * @param  val    Control value
 * @param  shift  write mask shift of control value
 */
__attribute__((always_inline))
static inline void gpio_ll_set_pin_ctrl(uint32_t val, uint32_t bmap, uint32_t shift)
{
    SET_PERI_REG_BITS(PIN_CTRL, bmap, val, shift);
}

/**
 * @brief Get the GPIO number that is routed to the input peripheral signal through GPIO matrix.
 *
 * @param hw Peripheral GPIO hardware instance address.
 * @param in_sig_idx Peripheral signal index (tagged as input attribute).
 *
 * @return
 *    - -1     Signal bypassed GPIO matrix
 *    - Others GPIO number
 */
static inline int gpio_ll_get_in_signal_connected_io(gpio_dev_t *hw, uint32_t in_sig_idx)
{
    typeof(hw->func_in_sel_cfg[in_sig_idx]) reg;
    reg.val = hw->func_in_sel_cfg[in_sig_idx].val;
    return (reg.sig_in_sel ? reg.func_sel : -1);
}

#ifdef __cplusplus
}
#endif
