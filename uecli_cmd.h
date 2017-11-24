/// \file uecli_cmd.h
/// \brief ϵͳ����֧��
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
 
#include "uecli.h"

// ********************************************************************************************
// �ַ�����

// ������ʾ��
const char* STRING_PROMPT = ">>";

// ��Ȩ������Ϣ
const char* STRING_COPYRIGHT_INFO =
"\r\n**********************************************************"
"\r\n*                 UECLI Ƕ��ʽ����̨����                 *"
"\r\n**********************************************************" UECLI_NEWLINE;

// ����������ʾ
const char* STRING_INVALID_COMMAND = "\"%s\" ������Ч�����";

// �汾����
const char* STRING_VERSIONS =
"Versions: V0.2 Copyright2016" UECLI_NEWLINE
"email: zhangjinxing_2006@163.com" UECLI_NEWLINE
__DATE__ " -- " __TIME__ UECLI_NEWLINE;

//���������ַ���
const char* STRING_CLEAR_SCREEN = "\033[2J\033[0;0H";

//�����б��ַ���
const char* STRING_CMD_LIST = "%-16s";
//�Ӳ˵���ӡ�ַ���
const char* STRING_SUBMENU_LIST = "/%-15s";

// ********************************************************************************************
// ϵͳ����֧��

static inline const uecli_MenuItem* GetSyscmdHandle(void);
static const uecli_MenuItem* GetUserCmdHandle(void);
static const uecli_MenuItem* PopMenuStack(void);
static const uecli_MenuItem* SearchMatchCommand(const char* cmdline);

// ��ӡ�������б�
static void PrintHandleList(const uecli_MenuItem* ptr)
{
    const char* strformat;
    
    if (ptr)
    {
        for (; ptr->pdata; ++ptr)
        {
            strformat = (ptr->itemType==UECLI_TYPE_SUBMENU) ? 
                STRING_SUBMENU_LIST : STRING_CMD_LIST;
            uecli_snprintf(uecli.tmpString, UECLI_CFG_TEMP_MAXLEN, strformat,ptr->exename);
            uecli_PrintString(uecli.tmpString);

            // ��ӡ������Ϣ
            uecli_PrintString(ptr->desc);
            uecli_PrintString(UECLI_NEWLINE);
        }
    }
}

// help����
static void Cmd_HelpMain(int argc, char* argv[])
{
    // ��������Ϣ
#if UECLI_01_LONG_HELP>0
    if (argc >= 2)
    {
        const uecli_MenuItem* pitem = SearchMatchCommand(argv[1]);
        if (pitem && UECLI_TYPE_FUN == pitem->itemType)
        {
            if (pitem->helpstr)
                uecli_PrintString(pitem->helpstr);
        }
        else
        {
            uecli_snprintf(uecli.tmpString, UECLI_CFG_TEMP_MAXLEN, STRING_INVALID_COMMAND, argv[1]);
            uecli_PrintString(uecli.tmpString);
        }
        
        uecli_PrintString(UECLI_NEWLINE);
        return;
    }
#endif

    //��ӡϵͳ���������Ϣ
    uecli_PrintString(STRING_COPYRIGHT_INFO);
    
    PrintHandleList(GetSyscmdHandle());     // ��ӡϵͳ�����
    PrintHandleList(GetUserCmdHandle());    // ��ӡ�û������

    (void)argc;
    (void)argv;
}

// ver�汾��Ϣ����ס����
static void Cmd_VerMain(int argc, char* argv[])
{
    uecli_PrintString(STRING_VERSIONS);

    (void)argc;
    (void)argv;
}

// CLEAR����
static void Cmd_ClearMain(int argc, char* argv[])
{
    uecli_PrintString(STRING_CLEAR_SCREEN);
    (void)argc;
    (void)argv;
}

// ..������һ������
static void Cmd_UpperMain(int argc, char* argv[])
{
    PopMenuStack();
    (void)argc;
    (void)argv;
}

// ϵͳ�����
static const uecli_MenuItem uecli_syscmdList[] =
{
    UECLI_DECLARE_COMMAND(Cmd_UpperMain,   "..",       "�����ϼ��Ӳ˵�"),
    UECLI_DECLARE_COMMAND(Cmd_HelpMain,    "help",     "��ʾ֧�ֵ������б�"),
    UECLI_DECLARE_COMMAND(Cmd_VerMain,     "ver",      "��ʾ�汾�Ͱ�����Ϣ"),
    UECLI_DECLARE_COMMAND(Cmd_ClearMain,   "cls",      "�����Ļ"),
    UECLI_DECLARE_END()
};

static inline const uecli_MenuItem* GetSyscmdHandle(void)
{
    return uecli_syscmdList;
}


