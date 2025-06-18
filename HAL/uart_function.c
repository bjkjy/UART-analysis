/*
 * uart_function.c - STM32 HAL Version
 *
 * Change Logs:
 * Date           Author           Notes
 * 2025-06-18     Bai Jiankang     Initial 
 */

#include "usart.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "uart_function.h"

// 接收缓冲区和状态变量
static char uart2_recv_buffer[40] = {0};
static unsigned char uart2_index = 0;

// 全局变量定义
recv_type_t g_type_of_recv_uart2 = NULL_TYPE;
unsigned char g_received_finished_uart2 = 0;
float g_received_float_uart2 = 0.0f;
char g_received_string_uart2[40] = {0};

// 提示字符串
const char *returns_error_style = "Format error!\r\n";
const char *returns_null = "None entered!\r\n";

/**
 * @brief 通过UART设备发送一个字符串 (HAL版本)
 */
void uart_putstring(UART_HandleTypeDef *huart, const char *str)
{
    HAL_UART_Transmit(huart, (unsigned char*)str, strlen(str), HAL_MAX_DELAY);
}


/**
 * @brief 用户交互回调函数
 */
void user_uart2_callback(void)
{
    if(g_type_of_recv_uart2 == STRING)
    {
        uart_putstring(&huart2, "STRING: ");
        uart_putstring(&huart2, g_received_string_uart2);
        uart_putstring(&huart2, "\n");
        if(strcmp((char*)g_received_string_uart2, "Run200") == 0)
        {
            uart_putstring(&huart2, "RUN SPEED 200...\n");
        }
        else if(strcmp((char*)g_received_string_uart2, "Stop") == 0)
        {
            uart_putstring(&huart2, "STOP CAR...\n");
        }
        else
        {
            uart_putstring(&huart2, "Command error!\n");
        }
    }
    // 可以添加对NUMBER类型的处理
    else if (g_type_of_recv_uart2 == NUMBER)
    {
        char num_buf[40];
        snprintf(num_buf, sizeof(num_buf), "Num: %f\n", g_received_float_uart2);
        uart_putstring(&huart2, num_buf);
    }
}


/**
 * @brief 处理接收到的单个字符，实现状态机逻辑
 * @brief 只要输入中包含任何非纯数字字符(除了起始的'-'和唯一的小数点)，
 *              整个输入就会被当作字符串处理。
 * @param recv_char 接收到的字符
 */
void uart2_process_char(char recv_char)
{
    static recv_type_t type_of_recv_uart2 = NULL_TYPE;
    static signed int recv_data_B = 0; // 整数部分
    static signed int recv_data_F = 0; // 小数部分
    static unsigned char point_position = 0;
    static unsigned char decimal_places = 0;
    static unsigned char is_negative = 0; // 用于标记负数

    //--------不是回车或换行--------------------------------
    if (recv_char != '\r' && recv_char != '\n')
    {
        g_received_finished_uart2 = 0;

        // --- 先将字符存入缓冲区 ---
        if (uart2_index < sizeof(uart2_recv_buffer) - 1)
        {
            uart2_recv_buffer[uart2_index++] = recv_char;
        }

        // --- 根据当前状态解析 ---

        // 规则1: 如果已经是字符串模式，无需做任何事，字符已存入
        if (type_of_recv_uart2 == STRING)
        {
            // Pass
        }
        // 规则2: 如果当前是数字模式，继续尝试并联解析
        else if (type_of_recv_uart2 == NUMBER)
        {
            if (isdigit((unsigned char)recv_char))
            {
                if (point_position == 0)
                {
                    // 检查溢出 (可选但推荐)
                    if (recv_data_B > (INT32_MAX / 10)) { type_of_recv_uart2 = STRING; }
                    else { recv_data_B = recv_data_B * 10 + (recv_char - '0'); }
                }
                else
                {
                    if (recv_data_F > (INT32_MAX / 10)) { type_of_recv_uart2 = STRING; }
                    else { recv_data_F = recv_data_F * 10 + (recv_char - '0'); }
                    decimal_places++;
                }
            }
            else if (recv_char == '.' && point_position == 0)
            {
                point_position = 1;
            }
            // 如果收到无效字符，则切换到字符串模式。
            // 因为原始字符已存入缓冲区，我们无需做任何重建工作。
            else
            {
                type_of_recv_uart2 = STRING;
            }
        }
        // 规则3: 如果是初始状态，根据第一个字符决定模式
        else // type_of_recv_uart2 == NULL_TYPE
        {
            if (isdigit((unsigned char)recv_char))
            {
                type_of_recv_uart2 = NUMBER;
                recv_data_B = recv_data_B * 10 + (recv_char - '0');
            }
            else if (recv_char == '-')
            {
                type_of_recv_uart2 = NUMBER;
                is_negative = 1;
            }
            else // 任何其他字符（字母、符号等）都直接启动STRING模式
            {
                type_of_recv_uart2 = STRING;
            }
        }
    }
    //--------是回车 (或换行) ---------------------------------
    else
    {
        // 结束时，根据最终确定的类型进行处理
        if (type_of_recv_uart2 == NUMBER)
        {
            float final_value = recv_data_B + (float)recv_data_F / pow(10, decimal_places);
            if (is_negative)
            {
                final_value = -final_value;
            }
            g_received_float_uart2 = final_value;
            g_type_of_recv_uart2 = NUMBER;
            g_received_finished_uart2 = 1;
        }
        else if (type_of_recv_uart2 == STRING)
        {
            uart2_recv_buffer[uart2_index] = '\0';
            strncpy(g_received_string_uart2, uart2_recv_buffer, sizeof(g_received_string_uart2) - 1);
            g_received_string_uart2[sizeof(g_received_string_uart2) - 1] = '\0'; // 确保安全
            g_type_of_recv_uart2 = STRING;
            g_received_finished_uart2 = 1;
        }
        else // NULL_TYPE (只收到了一个回车)
        {
            uart_putstring(&huart2, returns_null);
        }

        // --- 重置所有状态机变量，为下一次接收做准备 ---
        memset(uart2_recv_buffer, 0, sizeof(uart2_recv_buffer));
        uart2_index = 0;
        recv_data_B = 0;
        recv_data_F = 0;
        point_position = 0;
        decimal_places = 0;
        is_negative = 0;
        type_of_recv_uart2 = NULL_TYPE;
    }

    if(g_received_finished_uart2)
    {
        user_uart2_callback();
        // 处理完后清除标志，避免重复调用
        g_received_finished_uart2 = 0;
        g_type_of_recv_uart2 = NULL_TYPE; // 回调处理完后，全局类型也应重置
    }
}