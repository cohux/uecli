/**
 * @file     uecli.c
 * @brief    ��СǶ��ʽcli�����нӿ�ģ������ļ�
 * @author   ��ʫ��
 * @par
 * (C) Copyright ���ݴ��ʿƼ����޹�˾
 * @version
 * 2017/07/28 ��ʫ�� �޶�˵��\n
 *
 */ 

// ********************************************************************************************
// ͷ�ļ�

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "uecli.h"

// ********************************************************************************************
// �ڲ��궨��

// ѭ���ݼ�
#define CYCLE_DECREMENT(X,MAXVALU) ((X)>0 ? ((X)-1) : (MAXVALU))

// ѭ������
#define CYCLE_INCREMENT(X,MAXVALU) ((X)<(MAXVALU) ? ((X)+1) : 0)

// ��Ч����ʷ��¼����
#define INVALID_HISTORY_INDEX   (-1)

// ********************************************************************************************
// �ڲ����Ͷ���

/**
* @brief �����������ַ�������.
*
* �ɿ��ٻ�ȡ���ȣ����ٸ��ӡ�ɾ���ַ�
*/
typedef struct
{
    /* �����������ַ������� */
    char str[UECLI_CFG_MAX_CMDLINE];
    /* �ַ������� */
    int length;
}IdxString;

/**
* @brief uecli��С�����ж�������.
*
*/
typedef struct
{
    /* �û������ַ������� */
    IdxString instring;
    /* �ڲ���ʱ�ַ������� */
    char tmpString[TEMP_STRING_LEN];
    /* ��ʾ���ַ������� */
    char prompt[UECLI_CFG_MAX_POMPTLINE];
    /* �����б����ָ�� */
    const uecli_Handle* cmdtable;

    /* �Ӳ˵�֧�� */
#if UECLI_CFG_SUBMENU>0
    /* �Ӳ˵���ջ */
    const uecli_Handle* stack[UECLI_CFG_STACK_COUNT];
    /* ��ջ���� ָ�������е�Ԫ */
    int stackpos;
#endif

    /* ��������ʷ��¼֧�� */
#if UECLI_CFG_HISTORY>0
    /* ��ʷ��¼�ַ������� */
    char history[UECLI_CFG_HISTORY_COUNT][UECLI_CFG_MAX_CMDLINE];
    /* ��ʷ��¼����β������ */
    int historyEnd;
    /* ��ʷ��¼��ѯ���� */
    int historyindex;
    /* ��չ����״̬֧�� */
    char extSequenceState;
#endif
    /* ����������֧�� */
    const uecli_Handle* lockHandle;  /* ����������� */
}uecli_Type;

// ********************************************************************************************
// �ڲ�����

// cli�����нӿڶ���
static uecli_Type uecli;

// ********************************************************************************************
// ��������
#include "uecli_cmd.h"

void uecli_port_out(const void* buff, uint32_t num);
bool uecli_port_init(void);
// ********************************************************************************************
// �ڲ�����

// ��ȡ�����������ַ���
static inline const char* GetCmdlineString(IdxString* pistr)
{
    return pistr->str;
}
// ��ȡ�������ַ�������
static inline int GetCmdlineLength(IdxString* pistr)
{
    return pistr->length;
}
// ��������������ַ���
static inline void ClearCmdlineString(IdxString* pistr)
{
    pistr->length = 0;
    pistr->str[0] = '\0';
}
// ���������������ʾ
static inline void ClearCmdlineInput(IdxString* pistr)
{
    if (pistr->length)
    {
        uecli_snprintf(pistr->str, UECLI_CFG_MAX_CMDLINE, "\033[%dD\033[J",
            (uint8_t)pistr->length);
        uecli_PrintString(pistr->str);
    }
    ClearCmdlineString(pistr);
}
// �����ַ�������������ʾ
static inline void CopyStringToCmdline(IdxString* pistr, const char* str1)
{
    // ���֮ǰ������
    ClearCmdlineInput(pistr);

    //����ָ�����ַ���
    if (str1 && '\0' != *str1)
    {
        pistr->length = uecli_strlen(str1);
        uecli_strncpy(pistr->str, str1, UECLI_CFG_MAX_CMDLINE);
        uecli_PrintString(pistr->str);
    }
}
// �����ַ�
static inline char AppendCmdlinechar(IdxString* pistr, char c)
{
    if (pistr->length < UECLI_CFG_MAX_CMDLINE - 1)
    {
        pistr->str[pistr->length++] = c;
        pistr->str[pistr->length] = '\0';
        return c;
    }
    return '\0';
}
// ɾ�����һ���ַ�
static inline char DeleteCmdlineChar(IdxString* pistr)
{
    char c = '\0';
    if (pistr->length)
    {
        c = pistr->str[--pistr->length];
        pistr->str[pistr->length] = '\0';
    }
    return c;
}
// ����ַ����Ƿ�հ�
static inline bool IsBlankString(const char* str)
{
    return (NULL == str || '\0' == *str);
}
// �Ƿ�Ϊ�հ��ַ�
static inline bool IsBlankChar(char c)
{
    return (' ' == c || '\t' == c || '\r' == c);
}
// ��ȡ��һ���ָ�
static char* GetNextDelim(char* strline, char** pstr)
{
    assert(strline);

    // �������пհ��ַ�
    for (; IsBlankChar(*strline); ++strline);

    // �����ַ�����βֱ�ӷ���NULL
    if ('\0' == *strline) return NULL;
    *pstr = strline;

    bool mask = false;
    // ��β��0
    for (char c; ; ++strline)
    {
        if ('\0' == (c = *strline))
            return strline;
        if (c == '"')
            mask = !mask;
        else if (!mask && IsBlankChar(c))
            break;
    }

    *strline = '\0';
    return strline + 1;
}
// �ָ��ַ���
static int SplitString(char* str, char* substr[], int size)
{
    // ��������
    assert(str);
    assert(substr);

    int i = 0;
    for (; i < size; ++substr)
    {
        if (NULL == (str=GetNextDelim(str, substr)))
            break;
        ++i;
    }
    return i;
}
// ��ӡ��ʾ��
static inline void PrintCLIPrompt(void)
{
    uecli_PrintString(uecli.prompt);
}

// ********************************************************************************************
// ������֧��

#if UECLI_CFG_SUBMENU>0

//����cli��ʾ���ַ���
static void UpdateCLIPrompt(void)
{
    char* pstr = uecli.prompt;

    uecli_strncpy(pstr, UECLI_NEWLINE, UECLI_CFG_MAX_POMPTLINE);
    /* ����Ӳ˵�·�� */
    for (int i = 0; i < uecli.stackpos; ++i)
    {
        uecli_strncat(pstr, "\\", UECLI_CFG_MAX_POMPTLINE);
        uecli_strncat(pstr, uecli.stack[i]->exename, UECLI_CFG_MAX_POMPTLINE);
    }
    uecli_strncat(pstr, STRING_PROMPT, UECLI_CFG_MAX_POMPTLINE);
}
// ��ȡջ��
static inline const uecli_Handle* GetMenuStackHandle(void)
{
    if (uecli.stackpos)
        return uecli.stack[uecli.stackpos - 1];
    return NULL;
}
// ��ջ
static inline void PushMenuStack(const uecli_Handle* ptr)
{
    /* ������������򸲸� */
    if (uecli.stackpos >= UECLI_CFG_STACK_COUNT)
        uecli.stackpos = UECLI_CFG_STACK_COUNT - 1;
    uecli.stack[uecli.stackpos++] = ptr;
    UpdateCLIPrompt();
}
// ��ջ
static inline const uecli_Handle* PopMenuStack(void)
{
    const uecli_Handle* ptr = NULL;
    if (uecli.stackpos)
    {
        ptr = uecli.stack[--uecli.stackpos];
        UpdateCLIPrompt();
    }
    return ptr;
}

#else
static inline const uecli_Handle* PopMenuStack(void)
{
    return NULL;
}
static inline void PushMenuStack(const uecli_Handle* ptr)
{
}
//����cli��ʾ���ַ���
static void UpdateCLIPrompt(void)
{
    char* pstr = uecli.prompt;

    uecli_strncpy(pstr, UECLI_NEWLINE, UECLI_CFG_MAX_POMPTLINE);
    uecli_strncat(pstr, STRING_PROMPT, UECLI_CFG_MAX_POMPTLINE);
}
#endif

// ��ȡ�û�����
static inline const uecli_Handle* GetUserCmdHandle(void)
{
#if UECLI_CFG_SUBMENU>0
    const uecli_Handle* phand = GetMenuStackHandle();
    if (NULL != phand)
        return (const uecli_Handle*)phand->pdata;
#endif
    return uecli.cmdtable;
}

// ********************************************************************************************
// ��ʷ��¼֧��

#if UECLI_CFG_HISTORY>0
// ����������ʷ
static void SaveHistory(const char* cmdline)
{
    /* �հ��ַ���ֱ���˳� */
    if (IsBlankString(cmdline))
        return;

    /* ����������в��ظ����¼ */
    int i = uecli.historyEnd;
    if (uecli_strcasecmp(uecli.history[i], cmdline))
    {
        i = CYCLE_INCREMENT(i, UECLI_CFG_HISTORY_COUNT - 1);
        uecli_strncpy(uecli.history[i], cmdline, UECLI_CFG_MAX_CMDLINE);
        uecli.historyEnd = i;
    }
    uecli.historyindex = INVALID_HISTORY_INDEX;
}
// ��ȡ��ʷ��¼�����б���ʼλ��
static inline int GetStartHistoryPostion(void)
{
    return **uecli.history == '\0' ? 1 :
        CYCLE_INCREMENT(uecli.historyEnd, UECLI_CFG_HISTORY_COUNT-1);
}
// ��ȡ��һ����ʷ��¼
static const char* GetNextHistory(void)
{
    register int i = uecli.historyindex;
    int starti = GetStartHistoryPostion();

    /* �ж��Ƿ�Ϊ�� */
    if ('\0' == uecli.history[uecli.historyEnd][0])
        return NULL;
    else if (INVALID_HISTORY_INDEX == i)
        uecli.historyindex = starti;
    else if (i != uecli.historyEnd)
    {
        i = CYCLE_INCREMENT(i, UECLI_CFG_HISTORY_COUNT - 1);
        uecli.historyindex = i;
    }
    assert(uecli.history[uecli.historyindex][0] != '\0');
    return uecli.history[uecli.historyindex];
}
// ��ȡ��һ����ʷ��¼
static const char* GetPriveHistory(void)
{
    register int i = uecli.historyindex;
    int starti = GetStartHistoryPostion();

    /* �ж��Ƿ�Ϊ�� */
    if ('\0' == uecli.history[uecli.historyEnd][0])
        return NULL;
    else if (INVALID_HISTORY_INDEX == i)
        uecli.historyindex = uecli.historyEnd;
    else if (i != starti)
    {
        i = CYCLE_DECREMENT(i, UECLI_CFG_HISTORY_COUNT - 1);
        uecli.historyindex = i;
    }
    assert(uecli.history[uecli.historyindex][0] != '\0');
    return uecli.history[uecli.historyindex];
}
// ������չ�ַ�����
static char ProcessESCSequence(char c)
{
    const char* tstr;
    switch (c)
    {
    case '\x1b':    /* ��ʼ���� */
        uecli.extSequenceState = 1;
        return 0;
        break;
    case '[':
        if (1 == uecli.extSequenceState)
        {
            uecli.extSequenceState = 2;
            return 0;
        }
        break;
    case 'A':       /* �ϼ�ͷ */
        if (2 == uecli.extSequenceState)
        {
            uecli.extSequenceState = 0;
            if (!IsBlankString(tstr = GetPriveHistory()))
                CopyStringToCmdline(&uecli.instring, tstr);
            return 0;
        }
        break;
    case 'B':       /* �¼�ͷ */
        if (2 == uecli.extSequenceState)
        {
            uecli.extSequenceState = 0;
            if (!IsBlankString(tstr = GetNextHistory()))
                CopyStringToCmdline(&uecli.instring, tstr);
            return 0;
        }
        break;
    default:
        break;
    }
    return c;
}

#define PROCESS_ESC_SEQUENCE(c) ProcessESCSequence(c)
#define SAVE_HISTORY(cmd)  SaveHistory(cmd)
#else
#define PROCESS_ESC_SEQUENCE(c) (1)
#define SAVE_HISTORY(cmd)
#endif

// ********************************************************************************************
// ���ȫ֧��

#if UECLI_CFG_COMPLETE>0
// ����ƥ���ַ������� 1��ȫƥ�� 0����ƥ�� -1��ƥ��
static int StringIsMatch(const char* str1, const char* str2)
{
    assert(str1);
    assert(str2);

    if (str1 == str2) return 1;

    for (char c; ; ++str1, ++str2)
    {
        if ('\0' == (c=*str1))
        {
            if ('\0' == *str2)
                return 1;
            return 0;
        }
        else if (c != *str2)
            break;
    }
    return -1;
}
// ��ӡƥ�������ַ�����
static void PrintMatchCmdTable(const char** matchtable, int len)
{
    uecli_PrintString(UECLI_NEWLINE);
    const char* formatstr = NULL;
    /* ��ӡ�б� */
    for (int i = 0; i < len; ++i)
    {
        if ((i + 1) % 4 == 0)
            formatstr = "%s" UECLI_NEWLINE;
        else
            formatstr = "%-16s";

        uecli_snprintf(uecli.tmpString, TEMP_STRING_LEN, formatstr, matchtable[i]);
        uecli_PrintString(uecli.tmpString);
    }
    /* ��ӡ��ʾ�� */
    uecli_PrintString(UECLI_NEWLINE);
    PrintCLIPrompt();
    uecli_PrintString(GetCmdlineString(&uecli.instring));
}
// ����ƥ������
static int SearchAutoCompleteCommand(const char** strtable, int idx, const uecli_Handle* phand)
{
    for (; phand->pdata; ++phand)
    {
        if (!StringIsMatch(GetCmdlineString(&uecli.instring), phand->exename))
        {
            if (idx >= UECLI_CFG_AUTOCOMP_NUM - 1)
            {
                strtable[UECLI_CFG_AUTOCOMP_NUM - 1] = "...";
                idx = UECLI_CFG_AUTOCOMP_NUM;
                break;
            }
            strtable[idx] = phand->exename;
            ++idx;
        }
    }
    return idx;
}
// �Զ���ȫ֧��
static void AutoCompleteInstring(void)
{
    /* ������ֱ����ʾ������Ϣ */
    if (!GetCmdlineLength(&uecli.instring))
    {
        uecli_PrintString(UECLI_NEWLINE);
        Cmd_HelpMain(0, 0);
        PrintCLIPrompt();
        return;
    }

    const char* matchStringTable[UECLI_CFG_AUTOCOMP_NUM];
    int i = 0;

    /* ƥ��ϵͳ���� */
    i = SearchAutoCompleteCommand(matchStringTable, i, GetSyscmdHandle());
    if (i < UECLI_CFG_AUTOCOMP_NUM)
        i = SearchAutoCompleteCommand(matchStringTable, i, GetUserCmdHandle());


    /* ֻ�ҵ�1��ƥ�� ֱ����� */
    if (1 == i)
        CopyStringToCmdline(&uecli.instring, matchStringTable[0]);
    else if (i)
        PrintMatchCmdTable(matchStringTable, i);
}
#else
static void AutoCompleteInstring(void)
{
}
#endif

// ���������ַ�
static char cli_echo(char c)
{
    UECLI_LOG("���յ��ַ�0X%02x:%c", c, c);

    //������������
    switch (c)
    {
    case '\r':
    case '\n':  /* �س����� */
        uecli_PrintString(UECLI_NEWLINE);
        return '\n';
        break;
    case '\b':  /* �˸��ַ� */
        if (DeleteCmdlineChar(&uecli.instring))
            uecli_PrintString("\b \b");
        break;
    case '\t':  /* �Ʊ�� */
        AutoCompleteInstring();
        break;
    case '\x1b': /* ESC ��չ���ܷ� */
        PROCESS_ESC_SEQUENCE(c);
        break;
    default:    /* �����ɴ�ӡ�ַ� */
        if (uecli_isprintfchar(c) && PROCESS_ESC_SEQUENCE(c) && AppendCmdlinechar(&uecli.instring,c))
            uecli_port_out((const uint8_t*)&c, 1);		/* ������� */
    }

    return c;
}
// ����ƥ���������
static const uecli_Handle* SearchMatchCommand(const char* cmdline)
{
    const uecli_Handle* ptr = NULL;
    /* ����ϵͳ�����б� */
    for (ptr = GetSyscmdHandle(); ptr->pdata; ++ptr)
    {
        if (!uecli_strcasecmp(cmdline, ptr->exename))
            return ptr;
    }
    /* �����û������б� */
    for (ptr = GetUserCmdHandle(); ptr->pdata; ++ptr)
    {
        if (!uecli_strcasecmp(cmdline, ptr->exename))
            return ptr;
    }
    return NULL;
}

// ********************************************************************************************
// �ⲿ�ӿں���

/**
 * @brief   ��������������ַ������зָ��������ִ��
 *
 * @param[in]   cmdline   ����������ַ���
 * @return  void
 */
void uecli_ExeCmdine(const char* cmdstring)
{
    char strbuff[UECLI_CFG_MAX_CMDLINE];
    char* argcbuff[UECLI_CFG_MAX_ARGNUM];

    /* �ָ��ַ�����ı�ԭ���ݣ��ȿ���һ�ݳ��� */
    uecli_strncpy(strbuff, cmdstring, UECLI_CFG_MAX_CMDLINE);
    int count = SplitString(strbuff, argcbuff, UECLI_CFG_MAX_ARGNUM);

    /* ������ִ������ */
    if (count)
    {
        UECLI_LOG("���յ�������:%s", cmdstring);

        const uecli_Handle* phand = SearchMatchCommand(argcbuff[0]);
        if (NULL != phand)
        {
            SAVE_HISTORY(cmdstring);
            if (UECLI_TYPE_FUN == phand->itemType)
                ((void(*)(int, char**))(uint32_t)phand->pdata)(count, argcbuff);
            else
            {
                PushMenuStack(phand);
            }
        }
        else
        {
            UECLI_LOG("\"%s\" ������Ч������", argcbuff[0]);
            uecli_snprintf(uecli.tmpString, TEMP_STRING_LEN,STRING_INVALID_COMMAND, argcbuff[0]);
            uecli_PrintString(uecli.tmpString);
        }
        
    }
    /* ������������� ��ӡ��ʾ�� */
    ClearCmdlineString(&uecli.instring);
    PrintCLIPrompt();
}
/**
 * @brief      ������յ����ַ�����
 *
 * @param[in]  recchar   ���յ����ַ���������
 * @param[in]  len       ��������
 * @return     void
 */
void uecli_ProcessRecChar(const char recchar[], int len)
{
    //������յ���ÿ���ַ�
    for (int i = 0; i < len; ++i)
    {
        /* ���Դ��� */
        if ('\n' == cli_echo(recchar[i]))
        {
            /* ִ�������� */
            uecli_ExeCmdine(GetCmdlineString(&uecli.instring));
            break;
        }
    }
}
/**
 * @brief      ��ӡָ�����ַ���
 *
 * @param[in]  str      ����ӡ���ַ���
 * @return     void
 */
void uecli_PrintString(const char* str)
{
    if (str)
        uecli_port_out(str, uecli_strlen(str));
}
/**
 * @brief      ��ʼ��CLI�����ж���
 *
 * @param[in]  phand     ��֧�ֵ������
 * @return     void
 */
void uecli_Initialize(const uecli_Handle* phand)
{
    memset(&uecli,0,sizeof(uecli));
    uecli.cmdtable = phand;

    uecli_port_init();      // �ײ�ӿڳ�ʼ��
    uecli_PrintString(STRING_COPYRIGHT_INFO);
    UpdateCLIPrompt();      // ������ʾ���ַ���
    PrintCLIPrompt();       // ��ӡ��ʾ��
}

