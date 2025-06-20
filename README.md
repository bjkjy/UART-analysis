# 快速开始

## 1. 把.c和.h文件放在项目**编译目录**下

## 2. 然后在串口回调函数所在文件开始部分导入本项目头文件（例如uart_function.h）
```
#include "uart_function.h"
```
## 3. 定义缓冲区变量并在串口回调函数中调用串口处理函数
```
unsigned char ReciveBuffer1;
```
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
	// 每次接收到一个char就会进入这个地方
	uart2_process_char(ReciveBuffer2); // 处理接收到的数据
        HAL_UART_Receive_IT(huart, &ReciveBuffer2, sizeof(ReciveBuffer2));
    }
}
```
