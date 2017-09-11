/**
 * @file     uecli_cmd.h
 * @brief    uecli�ڲ�ϵͳ����ʵ��
 * @author   ��ʫ��
 * @par
 * (C) Copyright ���ݴ��ʿƼ����޹�˾
 * @version
 * 2017/07/24 ��ʫ�� �޶�˵��\n
 *
 */

#include "uecli_cfg.h"

// ********************************************************************************************
// �ַ�����

// ������ʾ��
const char* STRING_PROMPT = ">>";

// ��Ȩ������Ϣ
const char* STRING_COPYRIGHT_INFO =
"\r\n**********************************************************"
"\r\n*                 UECLI Ƕ��ʽ����̨����                 *"
"\r\n**********************************************************\r\n";

// ����������ʾ
const char* STRING_INVALID_COMMAND = "\"%s\" ������Ч�����\r\n";

// �汾����
const char* STRING_VERSIONS =
"Versions: V0.1 Copyright2016 \r\n"
"email: zhangjinxing_2006@163.com\r\n"
__DATE__ " -- " __TIME__"\r\n";

//���������ַ���
const char* STRING_CLEAR_SCREEN = "\033[2J\033[0;0H";

//�����б��ַ���
const char* STRING_CMD_LIST = "%-16s %s\r\n";

// ********************************************************************************************
// ϵͳ����֧��

static inline const uecli_Handle* GetSyscmdHandle(void);
static const uecli_Handle* GetUserCmdHandle(void);
static const uecli_Handle* PopMenuStack(void);
static const uecli_Handle* SearchMatchCommand(const char* cmdline);

// ��ӡ�������б�
static void PrintHandleList(const uecli_Handle* ptr)
{
    if (ptr)
    {
        for (; ptr->pdata; ++ptr)
        {
            uecli_snprintf(uecli.tmpString, TEMP_STRING_LEN, STRING_CMD_LIST,
                ptr->exename, ptr->desc);
            uecli_PrintString(uecli.tmpString);
        }
    }
}

// ��ӡ֧�ֵ������б�
static void PrintCommandList()
{
    PrintHandleList(GetSyscmdHandle());      // ��ӡϵͳ�����
    PrintHandleList(GetUserCmdHandle());    // ��ӡ�û������
}
// help����
static void Cmd_HelpMain(int argc, char* argv[])
{
#if UECLI_CFG_LONG_HELP>0
    if (argc >= 2)
    {
        const uecli_Handle* phand = SearchMatchCommand(argv[1]);
        if (phand && UECLI_TYPE_FUN == phand->itemType)
        {
            if (phand->helpstr)
                uecli_PrintString(phand->helpstr);
        }
        else
        {
            uecli_snprintf(uecli.tmpString, TEMP_STRING_LEN, STRING_INVALID_COMMAND, argv[1]);
            uecli_PrintString(uecli.tmpString); /* �����Ϣ */
        }
        return;
    }
#endif

    //��ӡϵͳ���������Ϣ
    uecli_PrintString(STRING_COPYRIGHT_INFO);
    PrintCommandList();
}
// ver�汾��Ϣ����ס����
static void Cmd_VerMain(int argc, char* argv[])
{
    uecli_PrintString(STRING_VERSIONS);

    argc = argc;
    argv = argv;
}
// CLEAR����
static void Cmd_ClearMain(int argc, char* argv[])
{
    uecli_PrintString(STRING_CLEAR_SCREEN);
    argc = argc;
    argv = argv;
}
// ..������һ������
static void Cmd_UpperMain(int argc, char* argv[])
{
    PopMenuStack();
    argc = argc;
    argv = argv;
}
// ϵͳ�����
static const uecli_Handle uecli_syscmdList[] =
{
    UECLI_DECLARE_COMMAND(Cmd_UpperMain,   "..",       "�����ϼ��Ӳ˵�"),
    UECLI_DECLARE_COMMAND(Cmd_HelpMain,    "help",     "��ʾ֧�ֵ������б�"),
    UECLI_DECLARE_COMMAND(Cmd_VerMain,     "ver",      "��ʾ�汾�Ͱ�����Ϣ"),
    UECLI_DECLARE_COMMAND(Cmd_ClearMain,   "cls",      "�����Ļ"),
    UECLI_ITEM_END()
};

static inline const uecli_Handle* GetSyscmdHandle(void)
{
    return uecli_syscmdList;
}
