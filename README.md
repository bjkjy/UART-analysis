# 快速开始

```
/**
  * @brief  串口接收回调函数
  * @param  huart handle
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)  // 判断是哪个UART触发了中断
    {
			  uart2_process_char(ReciveBuffer2); // 处理接收到的数据
        HAL_UART_Receive_IT(huart, &ReciveBuffer2, sizeof(ReciveBuffer2));
    }
}
```
