/*
 * uart_function.c - ZF
 *
 * Change Logs:
 * Date           Author           Notes
 * 2025-06-18     Bai Jiankang     Initial 
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "uart_function.h"
#include "zf_common_headfile.h"

// ���ջ�������״̬����
static char uart3_recv_buffer[40] = {0};
static unsigned char uart3_index = 0;

// ȫ�ֱ�������
recv_type_t g_type_of_recv_uart3 = NULL_TYPE;
unsigned char g_received_finished_uart3 = 0;
float g_received_float_uart3 = 0.0f;
char g_received_string_uart3[40] = {0};

// ��ʾ�ַ���
const char *returns_error_style = "Format error!\r\n";
const char *returns_null = "None entered!\r\n";

/**
 * @brief ͨ��UART�豸����һ���ַ���
 */
void uart3_putstring(const char *str)
{
    uart_write_string(UART_3, str);
}


/**
 * @brief �û������ص�����
 */
void user_uart3_callback(void)
{
    if(g_type_of_recv_uart3 == STRING)
    {
        uart3_putstring("STRING: ");
        uart3_putstring(g_received_string_uart3);
        uart3_putstring("\n");
        if(strcmp((char*)g_received_string_uart3, "Run200") == 0)
        {
            uart3_putstring("RUN SPEED 200...\n");
        }
        else if(strcmp((char*)g_received_string_uart3, "Stop") == 0)
        {
            uart3_putstring("STOP CAR...\n");
        }
        else
        {
            uart3_putstring("Command error!\n");
        }
    }
    // ������Ӷ�NUMBER���͵Ĵ���
    else if (g_type_of_recv_uart3 == NUMBER)
    {
        char num_buf[40];
        snprintf(num_buf, sizeof(num_buf), "Num: %f\n", g_received_float_uart3);
        uart3_putstring(num_buf);
    }
}


/**
 * @brief ������յ��ĵ����ַ���ʵ��״̬���߼�
 * @brief ֻҪ�����а����κηǴ������ַ�(������ʼ��'-'��Ψһ��С����)��
 *              ��������ͻᱻ�����ַ�������
 * @param recv_char ���յ����ַ�
 */
void uart3_process_char(char recv_char)
{
    static recv_type_t type_of_recv_uart3 = NULL_TYPE;
    static signed int recv_data_B = 0; // ��������
    static signed int recv_data_F = 0; // С������
    static unsigned char point_position = 0;
    static unsigned char decimal_places = 0;
    static unsigned char is_negative = 0; // ���ڱ�Ǹ���

    //--------���ǻس�����--------------------------------
    if (recv_char != '\r' && recv_char != '\n')
    {
        g_received_finished_uart3 = 0;

        // --- �Ƚ��ַ����뻺���� ---
        if (uart3_index < sizeof(uart3_recv_buffer) - 1)
        {
            uart3_recv_buffer[uart3_index++] = recv_char;
        }

        // --- ���ݵ�ǰ״̬���� ---

        // ����1: ����Ѿ����ַ���ģʽ���������κ��£��ַ��Ѵ���
        if (type_of_recv_uart3 == STRING)
        {
            // Pass
        }
        // ����2: �����ǰ������ģʽ���������Բ�������
        else if (type_of_recv_uart3 == NUMBER)
        {
            if ((recv_char>='0')&&(recv_char<='9'))
            {
                if (point_position == 0)
                {
                    // ������ (��ѡ���Ƽ�)
                    if (recv_data_B > (INT32_MAX / 10)) { type_of_recv_uart3 = STRING; }
                    else { recv_data_B = recv_data_B * 10 + (recv_char - '0'); }
                }
                else
                {
                    if (recv_data_F > (INT32_MAX / 10)) { type_of_recv_uart3 = STRING; }
                    else { recv_data_F = recv_data_F * 10 + (recv_char - '0'); }
                    decimal_places++;
                }
            }
            else if (recv_char == '.' && point_position == 0)
            {
                point_position = 1;
            }
            // ����յ���Ч�ַ������л����ַ���ģʽ��
            // ��Ϊԭʼ�ַ��Ѵ��뻺�����������������κ��ؽ�������
            else
            {
                type_of_recv_uart3 = STRING;
            }
        }
        // ����3: ����ǳ�ʼ״̬�����ݵ�һ���ַ�����ģʽ
        else // type_of_recv_uart3 == NULL_TYPE
        {
            if ((recv_char>='0')&&(recv_char<='9'))
            {
                type_of_recv_uart3 = NUMBER;
                recv_data_B = recv_data_B * 10 + (recv_char - '0');
            }
            else if (recv_char == '-')
            {
                type_of_recv_uart3 = NUMBER;
                is_negative = 1;
            }
            else // �κ������ַ�����ĸ�����ŵȣ���ֱ������STRINGģʽ
            {
                type_of_recv_uart3 = STRING;
            }
        }
    }
    //--------�ǻس� (����) ---------------------------------
    else
    {
        // ����ʱ����������ȷ�������ͽ��д���
        if (type_of_recv_uart3 == NUMBER)
        {
            float final_value = recv_data_B + (float)recv_data_F / pow(10, decimal_places);
            if (is_negative)
            {
                final_value = -final_value;
            }
            g_received_float_uart3 = final_value;
            g_type_of_recv_uart3 = NUMBER;
            g_received_finished_uart3 = 1;
        }
        else if (type_of_recv_uart3 == STRING)
        {
            uart3_recv_buffer[uart3_index] = '\0';
            strncpy(g_received_string_uart3, uart3_recv_buffer, sizeof(g_received_string_uart3) - 1);
            g_received_string_uart3[sizeof(g_received_string_uart3) - 1] = '\0'; // ȷ����ȫ
            g_type_of_recv_uart3 = STRING;
            g_received_finished_uart3 = 1;
        }
        else // NULL_TYPE (ֻ�յ���һ���س�)
        {
            uart3_putstring(returns_null);
        }

        // --- ��������״̬��������Ϊ��һ�ν�����׼�� ---
        memset(uart3_recv_buffer, 0, sizeof(uart3_recv_buffer));
        uart3_index = 0;
        recv_data_B = 0;
        recv_data_F = 0;
        point_position = 0;
        decimal_places = 0;
        is_negative = 0;
        type_of_recv_uart3 = NULL_TYPE;
    }

    if(g_received_finished_uart3)
    {
        user_uart3_callback();
        // ������������־�������ظ�����
        g_received_finished_uart3 = 0;
        g_type_of_recv_uart3 = NULL_TYPE; // �ص��������ȫ������ҲӦ����
    }
}
