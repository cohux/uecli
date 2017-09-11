/**
 * @file     uecli_cfg.h
 * @brief    uecli����ͷ�ļ�
 * @author   ��ʫ��
 * @par
 * (C) Copyright ���ݴ��ʿƼ����޹�˾
 * @version
 * 2017/07/31 ��ʫ�� �޶�˵��\n
 *
 */ 

#ifndef _U_E_CLI_CFG_H_
#define _U_E_CLI_CFG_H_

// ********************************************************************************************
// ����

/* �����������ַ�����󳤶� */
#define UECLI_CFG_MAX_CMDLINE    (32)

// �ڲ���ʱ�ַ��������С
#define TEMP_STRING_LEN     (64)

/* ���������������ָ�������� */
#define UECLI_CFG_MAX_ARGNUM     (6)

/* ��ʾ���ַ�����󳤶� */
#define UECLI_CFG_MAX_POMPTLINE  (32)

/* ������������ʷ������� */
#define UECLI_CFG_HISTORY_COUNT (16)

/* �Ӳ˵���ջ������� */
#define UECLI_CFG_STACK_COUNT   (4)

/* �Զ���ȫ�����ʾ���� */
#define UECLI_CFG_AUTOCOMP_NUM  (16)

// ����֧�ֿ���
//#define SHELL_CFG_COLOR         (1)     /* ֧����ɫ��չ */
#define UECLI_CFG_LONG_HELP     (1)     /* ��������Ϣ֧�� */
#define UECLI_CFG_SUBMENU       (1)     /* �����ջ֧�� */
#define UECLI_CFG_HISTORY       (1)     /* ������ʷ֧�� */
#define UECLI_CFG_COMPLETE      (1)     /* �Զ���ȫ֧�� */

/* ���з� */
#define UECLI_NEWLINE   "\r\n"

/* log֧�� */
#define UECLI_LOG(FORMAT, ...) /*eprintf(FORMAT UECLI_NEWLINE, ##__VA_ARGS__)*/

//�ַ���������������
#include "estring.h"
#define uecli_snprintf(sd,num,ss,...) esnprintf(sd,num,ss,##__VA_ARGS__)
#define uecli_isprintfchar(ch) ((ch)>=0x20&&(ch)<=0x7f)
#define uecli_strncpy(sd,ss,num) estrncpy(sd,ss,num)
#define uecli_strcasecmp(sd,ss) estrcmpNocase(sd,ss)
#define uecli_strncat(sd, ss, len)  estrncat(sd,ss,len)
#define uecli_strlen(str)   estrlen(str)

#endif
