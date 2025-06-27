

#include "stm32g0xx_hal.h"
#include "stdint.h"
#include  "assert.h"

SPI_HandleTypeDef hspi1;


void BSP_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }

}

void BSP_SPI_TxRx(const uint8_t *pTxData, uint8_t *pRxData, uint16_t Size)
{
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)pTxData, pRxData, Size, (Size + 9)/ 10);
	assert(status == HAL_OK);	
}

void BSP_SPI_Tx(const uint8_t *pData, uint16_t Size)
{
	HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, (uint8_t*)pData, Size, (Size + 9)/ 10);
	assert(status == HAL_OK);
}

void BSP_SPI_Rx(uint8_t *pData, uint16_t Size)
{
	HAL_StatusTypeDef status = HAL_SPI_Receive(&hspi1, pData, Size, (Size + 9)/ 10);
	assert(status == HAL_OK);	
}
