/// \file uecli_cfg.h
/// \brief uecli����ͷ�ļ�
///
/// \details
/// uecli ��Ƕ��ʽ����������нӿ�֧��ģ��
/// ��֧�����ȫ����ʷ��¼���Ӳ˵�����������Ϣ������
///
/// \author ��ʫ��
/// \par
/// (C) Copyright ���ݴ��ʿƼ����޹�˾
///
/// \version
/// 2017/07/24 ��ʫ�� ��ʼ�汾\n

#ifndef _U_E_CLI_CFG_H_
#define _U_E_CLI_CFG_H_

// ********************************************************************************************
// ����

/// \defgroup  UECLI_CFG������ö���
/// \{
#define UECLI_CFG_STR_MAXLEN     (32)    ///< �����������ַ�����󳤶�
#define UECLI_CFG_TEMP_MAXLEN    (64)    ///< �ڲ�������ʱ�ַ�����󳤶�
#define UECLI_CFG_MAX_ARGNUM     (6)     ///< ���������������ָ��������
#define UECLI_CFG_MAX_POMPTLINE  (32)    ///< ��ʾ���ַ�����󳤶�
#define UECLI_CFG_HISTORY_COUNT  (8)     ///< ������������ʷ�������
#define UECLI_CFG_STACK_COUNT    (8)     ///< �Ӳ˵���ջ�������
#define UECLI_CFG_AUTOCOMP_NUM   (16)    ///< �Զ���ȫ�����ʾ����
/// \}

/// \defgroup  UECLI_01����֧�ֿ���
/// \{
#define UECLI_01_COLOR          (0)     ///< ֧����ɫ��չ
#define UECLI_01_IN_HOOK        (1)     ///< ���빳��֧��
#define UECLI_01_LONG_HELP      (1)     ///< ��������Ϣ֧��
#define UECLI_01_SUBMENU        (1)     ///< �Ӳ˵�֧��
#define UECLI_01_HISTORY        (1)     ///< ������ʷ֧��
#define UECLI_01_COMPLETE       (1)     ///< �Զ���ȫ֧��
/// \}

///< ���з�
#define UECLI_NEWLINE   "\r\n"

///< log֧��
#define UECLI_LOG(FORMAT, ...) /*eprintf(FORMAT UECLI_NEWLINE, ##__VA_ARGS__)*/

//�ַ���������������
#include "estring.h"
#define uecli_snprintf(sd,num,ss,...) esnprintf(sd,num,ss,##__VA_ARGS__)
#define uecli_isprintfchar(ch) ((ch)>=0x20&&(ch)<=0x7f)

///< �����ַ���
#define uecli_strncpy(sd,ss,num) estrncpy(sd,ss,num)

///< �����ַ��Ƚ�
#define uecli_strcasecmp(sd,ss) estrcmpNocase(sd,ss)

///< ׷���ַ�
#define uecli_strncat(sd, ss, len)  estrncat(sd,ss,len)

///< ��ȡ�ַ�������
#define uecli_strlen(str)   estrlen(str)

#include <assert.h>
#define uecli_assert(x) assert(x)

#endif
