
#include "main.h"
#include "bsp_io.h"

/*
 *   //蓝牙输出
 *   PA1 BLE_RST                                            OUT
 *   PA2 UART2_TX                                           OUT
 *   PA3 UART3_TX                                           OUT
 *   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 *   //LCD屏
 *   PA4 SPI1_CS                                            OUT
 *   PA5 SPI1_CLK                                           OUT
 *   PA6 SPI1_MISO                                          未接
 *   PA7 SPI1_MOSI                                          OUT
 *   PB0 LCD_RST                                            OUT
 *   PB2 LCD_A0                                             OUT
 *   //////////////////////////
 *   //E2
 *   PB10 I2C_SCL                                           OUT
 *   PB11 I2C_SDA                                           OUT
 *   //LED
 *   PA11 LED1                                              OUT
 *   PA12 LED2                                              OUT
 *   //开机控制
 *   PB1 PWR                                                OUT
 *   //喂狗
 *   PB9 WDI                                                OUT
 *   //蜂鸣器
 *   PB8 BELL                                               OUT
 *   //KEY
 *   PB7 KEY9                                               IN
 *   PB6 KEY8                                               IN
 *   PB5 KEY7                                               IN
 *   PB4 KEY6                                               IN
 *   PB3 KEY5                                               IN
 *   PD3 KEY4                                               IN
 *   PD2 KEY3                                               IN
 *   PD1 KEY2                                               IN
 *   PD0 KEY1                                               IN
 *   PA15 KEY_ON_OFF                                        IN
 *   //电池ADC
 *   PA0                                                    IN

*/

void BSP_GPIO_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                            |GPIO_PIN_11|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

    /*Configure GPIO pins : PA1 BLE_RST, PA11  LED1 ,PA12  LED2*/
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    //spi cs pullup
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    /*Configure GPIO pins : PB0 LCD_RST, PB1 PWR,PB2 LCD_A0, PB10 I2C_SCL
                            PB11 I2C_SDA, PB8 WDI PB9 BELL */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                            |GPIO_PIN_11|GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

////////////////////// input /////////////////////////////////////////////////////////////
    /*Configure GPIO pin : PA15 KEY_ON_OFF */
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    //9个key///
    /*Configure GPIO pins : PD0 PD1 PD2 PD3 */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /*Configure GPIO pins : PB3 PB4 PB5 PB6
                            PB7 */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                            |GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}