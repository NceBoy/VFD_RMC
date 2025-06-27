#ifndef  BSP_SPI_H_
#define  BSP_SPI_H_
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif


void BSP_SPI_Init(void);
void BSP_SPI_TxRx(const uint8_t *pTxData, uint8_t *pRxData, uint16_t Size);
void BSP_SPI_Tx(const uint8_t *pData, uint16_t Size);
void BSP_SPI_Rx(uint8_t *pData, uint16_t Size);



#ifdef __cplusplus
}
#endif

#endif     