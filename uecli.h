/// \file uecli.h
/// \brief �����ļ�
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

#ifndef _U_E_CLI_H_
#define _U_E_CLI_H_

// ********************************************************************************************
// ͷ�ļ�

#include <stdint.h>
#include <stdbool.h>
#include "uecli_cfg.h"

// ********************************************************************************************
// ���Ͷ���

/// \brief ����˵�����Ŀ����
///
typedef enum
{
    UECLI_TYPE_FUN = 0,     ///< ����ĿΪһ�������Ե�����
    UECLI_TYPE_SUBMENU      ///< ����ĿΪ�Ӳ˵�
}uecli_ItemType;

/// \brief �����б����Ͷ���
///
/// �û����붨�������б����飬���ڳ�ʼ��ʱ���ø�����
/// ����Ϊ����֧�ֵĸ���������б�
typedef struct
{
    const void* pdata;          ///< ָ����������Ӳ˵�����������ָ��
    uecli_ItemType itemType;    ///< ��ʾ��ǰ��Ŀ���ͣ�����or�Ӳ˵�
    const char* exename;        ///< ����ƥ��ִ������������ַ���
    const char* desc;           ///< ��ʾ��help�����е�˵���ַ���
    
#if UECLI_01_LONG_HELP>0
    const char* helpstr;        ///< ʹ��help+����ʱ��ʾ�ĳ�������Ϣ
#endif
}uecli_MenuItem;

/// \defgroup  UECLI_DECLARE����ӿ���������
/// \{
#if UECLI_01_LONG_HELP>0     
#define UECLI_DECLARE_COMMAND(PDATA, EXENAME, HELPSTR, ...) \
    {(void*)PDATA, UECLI_TYPE_FUN, EXENAME, HELPSTR, ##__VA_ARGS__}

#define UECLI_DECLARE_SUBMENU(PDATA, MENUNAME, HELPSTR) \
    {(void*)PDATA, UECLI_TYPE_SUBMENU, MENUNAME, HELPSTR, 0}
    
#else

///< ��������˵���
#define UECLI_DECLARE_COMMAND(PDATA, EXENAME, HELPSTR, ...) \
    {(void*)PDATA, UECLI_TYPE_FUN, EXENAME, HELPSTR}

///< �����Ӳ˵���
#define UECLI_DECLARE_SUBMENU(PDATA, MENUNAME, HELPSTR) \
    {(void*)PDATA, UECLI_TYPE_SUBMENU, MENUNAME, HELPSTR}
#endif

///< �����б����β
#define UECLI_DECLARE_END() {0}
/// \}

// ********************************************************************************************
// �ӿں���

//����C C++��ϱ��
#ifdef __cplusplus
extern "C" {
#endif

void uecli_Initialize(const uecli_MenuItem* cmdlist);
void uecli_ExeCmdine(const char* cmdstring);
void uecli_ProcessRecChar(const char recchar[], int len);
void uecli_PrintString(const char* str);

#if UECLI_01_IN_HOOK>0
void* uecli_SetHook(void(*pfun)(int,char**));
bool uecli_IsValidHook(void);
#endif

//����C C++��ϱ��
#ifdef __cplusplus
}
#endif

#endif
