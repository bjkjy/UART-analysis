/*
 * uart_function.h - ZF
 *
 * Change Logs:
 * Date           Author           Notes
 * 2025-06-18     Bai Jiankang     Initial 
 */
 
#ifndef APPLICATIONS_UART_H_
#define APPLICATIONS_UART_H_

// 接收数据类型枚举
typedef enum
{
    NUMBER = 0,
    STRING,
    NULL_TYPE
} recv_type_t;

// 外部全局变量声明
extern recv_type_t g_type_of_recv_uart3;
extern unsigned char g_received_finished_uart3;
extern float g_received_float_uart3;
extern char g_received_string_uart3[40];

void uart3_process_char(char recv_char);

#endif /* APPLICATIONS_UART_H_ */
