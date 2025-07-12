#include "stm32g0xx_hal.h"
#include "stdint.h"
#include  "assert.h"
#include "queue.h"
#include "protocol.h"  // 包含协议头文件，获取HEADER_VALUE和FOOTER_VALUE
// 定义状态机状态
typedef enum {
  WAIT_HEADER,       // 等待包头
  RECEIVING_DATA,    // 接收数据中
} UartRxState;

// 全局变量
static Queue uart1_queue;
static UartRxState uart1_rx_state = WAIT_HEADER;
static uint8_t rx_count = 0;
static uint8_t rx_buffer[BUFFER_SIZE];

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

void BSP_UART_Init(void)
{
  queue_init(&uart1_queue);
  BSP_USART1_UART_Init();
  BSP_USART2_UART_Init();
}

// 获取UART1队列
Queue* get_uart1_queue(void) {
  return &uart1_queue;
}

// UART1字节处理函数 - 使用状态机检测包头包尾
bool uart1_write_byte(uint8_t data) {
  switch (uart1_rx_state) {
      case WAIT_HEADER:
          // 检测包头
          if (data == HEADER_VALUE) {
              // 重置接收缓冲区
              rx_buffer[0] = data;
              rx_count = 1;
              
              // 切换状态
              uart1_rx_state = RECEIVING_DATA;
              return true;
          }
          break;
          
      case RECEIVING_DATA:
          // 检查缓冲区是否有足够空间
          if (rx_count < BUFFER_SIZE) {
              // 写入数据
              rx_buffer[rx_count++] = data;
              
              // 检测包尾
              if (data == FOOTER_VALUE) {
                  // 将完整数据包写入队列
                  bool result = queue_write_packet(&uart1_queue, rx_buffer, rx_count);
                  
                  // 重置状态
                  rx_count = 0;
                  uart1_rx_state = WAIT_HEADER;
                  
                  return result;
              }
              return true;
          } else {
              // 缓冲区溢出，重置状态
              rx_count = 0;
              uart1_rx_state = WAIT_HEADER;
              return false;
          }
          break;
  }
  
  return false;
}

static void BSP_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
  __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_RXNE);
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void BSP_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}
//uart裸机接收
void LPUART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&hlpuart1);

    uint32_t tmp_flag = 0, tmp_it_source = 0;
    tmp_flag = __HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE);
    tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE);
    if((tmp_flag != RESET) && (tmp_it_source != RESET))
    {
        uint8_t data = (uint8_t)(huart1.Instance->RDR & 0xFF);
        uart1_write_byte(data);
    }
}

uint32_t lpuart_err = 0;
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    uint32_t err = HAL_UART_GetError(huart);
	if(err &(HAL_UART_ERROR_PE | HAL_UART_ERROR_NE | HAL_UART_ERROR_FE | HAL_UART_ERROR_ORE))
    {
        if(err & HAL_UART_ERROR_ORE) /*溢出错误会关闭接收，此时重新打开*/
        {
            huart1->Instance->CR1 |= UART_IT_RXNE;
        }  
        huart->ErrorCode = HAL_UART_ERROR_NONE;
    }
    if(huart->Instance == LPUART1)
    {
        lpuart_err++;
    }
}





/*// 修改UART接收中断处理函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        // 使用状态机处理接收到的数据
        uart1_write_byte(current_rx_byte);
        
        // 继续接收下一个字节
        HAL_UART_Receive_IT(&huart1, &current_rx_byte, 1);
    }
}*/