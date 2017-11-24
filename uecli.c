/// \file uecli.c
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

// ********************************************************************************************
// ͷ�ļ�

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


// �����������ַ�������.
//
// �ɿ��ٻ�ȡ���ȣ����ٸ��ӡ�ɾ���ַ�
typedef struct
{
    char str[UECLI_CFG_STR_MAXLEN];    // �����������ַ�������
    int length;     // �ַ�������
}IdxString;

// cli�����нӿڶ���
static struct uecli_Type
{
    
    IdxString instring;                     // �û������ַ�������
    char tmpString[UECLI_CFG_TEMP_MAXLEN];   // �ڲ���ʱ�ַ�������
    char prompt[UECLI_CFG_MAX_POMPTLINE];   // ��ʾ���ַ�������
    const uecli_MenuItem* cmdlist;            // �����б����ָ��

    // ���빳��֧��
#if UECLI_01_IN_HOOK>0
    void(*hookfun)(int,const char**) ;  // ����Ӿ�� 
#endif

    // �Ӳ˵�֧��
#if UECLI_01_SUBMENU>0
    const uecli_MenuItem* stack[UECLI_CFG_STACK_COUNT];   // �Ӳ˵���ջ
    int stackpos;    // ��ջ���� ָ�������е�Ԫ
#endif

    // ��������ʷ��¼֧��
#if UECLI_01_HISTORY>0
    char history[UECLI_CFG_HISTORY_COUNT][UECLI_CFG_STR_MAXLEN];   // ��ʷ��¼�ַ�������
    int historyEnd;             // ��ʷ��¼����β������
    int historyindex;           // ��ʷ��¼��ѯ����
    char extSequenceState;      // ��չ����״̬֧��
#endif

}uecli;

// ********************************************************************************************
// �ڲ�����

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
        uecli_snprintf(pistr->str, UECLI_CFG_STR_MAXLEN, "\033[%dD\033[J",
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

    // ����ָ�����ַ���
    if (str1 && '\0' != *str1)
    {
        uecli_strncpy(pistr->str, str1, UECLI_CFG_STR_MAXLEN);
        pistr->length = uecli_strlen(pistr->str);
        uecli_PrintString(pistr->str);
    }
}

// �����ַ�
static inline char AppendCmdlinechar(IdxString* pistr, char c)
{
    if (pistr->length < UECLI_CFG_STR_MAXLEN - 1)
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
static char* GetNextDelim(char* instring, char** pstr)
{
    uecli_assert(instring);

    // �������пհ��ַ�
    for (; IsBlankChar(*instring); ++instring);

    register char c = *instring;
    bool quotemask = false;     // ˫���ű��

    if ('"' == c)
    {
        c = *(++instring);
        quotemask = true;
    }
    if ('\0' == c)
        return NULL;
            
    // ���հ��ַ�
    for (*pstr = instring; ; ++instring)
    {
        if ('\0' == (c=*instring))
            return instring;
        else if (quotemask && '"' == c)
            break;
        else if (!quotemask && IsBlankChar(c))
            break;
    }

    *instring = '\0';
    return instring + 1;
}

// �ָ��ַ���
static int SplitString(char* str, char* substr[], int size)
{
    // ��������
    uecli_assert(str);
    uecli_assert(substr);

    if (IsBlankString(str) || !substr)
        return 0;

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
// �Ӳ˵�֧��

#if UECLI_01_SUBMENU>0

// ����cli��ʾ���ַ���
static void UpdateCLIPrompt(void)
{
    char* pstr = uecli.prompt;

    uecli_strncpy(pstr, UECLI_NEWLINE, UECLI_CFG_MAX_POMPTLINE);
    
    // ����Ӳ˵�·��
    for (int i = 0; i < uecli.stackpos; ++i)
    {
        uecli_strncat(pstr, "\\", UECLI_CFG_MAX_POMPTLINE);
        uecli_strncat(pstr, uecli.stack[i]->exename, UECLI_CFG_MAX_POMPTLINE);
    }
    uecli_strncat(pstr, STRING_PROMPT, UECLI_CFG_MAX_POMPTLINE);
}

// ջ��
static inline const uecli_MenuItem* GetMenuStackHandle(void)
{
    if (uecli.stackpos)
        return uecli.stack[uecli.stackpos - 1];
    return NULL;
}

// �˵���ջ��ջ
static inline void PushMenuStack(const uecli_MenuItem* ptr)
{
    // ������������򸲸�
    if (uecli.stackpos >= UECLI_CFG_STACK_COUNT)
        uecli.stackpos = UECLI_CFG_STACK_COUNT - 1;
    uecli.stack[uecli.stackpos++] = ptr;
    
    UpdateCLIPrompt();
}

// �˵���ջ��ջ
static inline const uecli_MenuItem* PopMenuStack(void)
{
    const uecli_MenuItem* ptr = NULL;
    if (uecli.stackpos)
    {
        ptr = uecli.stack[--uecli.stackpos];
        UpdateCLIPrompt();
    }
    return ptr;
}

#else
//����cli��ʾ���ַ���
static void UpdateCLIPrompt(void)
{
    char* pstr = uecli.prompt;

    uecli_strncpy(pstr, UECLI_NEWLINE, UECLI_CFG_MAX_POMPTLINE);
    uecli_strncat(pstr, STRING_PROMPT, UECLI_CFG_MAX_POMPTLINE);
}
static inline const uecli_MenuItem* PopMenuStack(void)
{
    return NULL;
}
#endif

// ��ȡ�û�����
static inline const uecli_MenuItem* GetUserCmdHandle(void)
{
#if UECLI_01_SUBMENU>0
    const uecli_MenuItem* phand = GetMenuStackHandle();
    if (NULL != phand)
        return (const uecli_MenuItem*)phand->pdata;
#endif
    return uecli.cmdlist;
}

// ********************************************************************************************
// ��ʷ��¼֧��

#if UECLI_01_HISTORY>0
// ����������ʷ
static void SaveHistory(const char* cmdline)
{
    // �հ��ַ���ֱ���˳�
    if (IsBlankString(cmdline))
        return;

    // ����������в��ظ����¼
    int i = uecli.historyEnd;
    if (uecli_strcasecmp(uecli.history[i], cmdline))
    {
        i = CYCLE_INCREMENT(i, UECLI_CFG_HISTORY_COUNT - 1);
        uecli_strncpy(uecli.history[i], cmdline, UECLI_CFG_STR_MAXLEN);
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
    int i = uecli.historyindex;
    int starti = GetStartHistoryPostion();

    // �ж��Ƿ�Ϊ��
    if ('\0' == uecli.history[uecli.historyEnd][0])
        return NULL;
    else if (INVALID_HISTORY_INDEX == i)
        uecli.historyindex = starti;
    else if (i != uecli.historyEnd)
    {
        i = CYCLE_INCREMENT(i, UECLI_CFG_HISTORY_COUNT - 1);
        uecli.historyindex = i;
    }
    
    uecli_assert(uecli.history[uecli.historyindex][0] != '\0');
    return uecli.history[uecli.historyindex];
}

// ��ȡ��һ����ʷ��¼
static const char* GetPriveHistory(void)
{
    int i = uecli.historyindex;
    int starti = GetStartHistoryPostion();

    // �ж��Ƿ�Ϊ��
    if ('\0' == uecli.history[uecli.historyEnd][0])
        return NULL;
    else if (INVALID_HISTORY_INDEX == i)
        uecli.historyindex = uecli.historyEnd;
    else if (i != starti)
    {
        i = CYCLE_DECREMENT(i, UECLI_CFG_HISTORY_COUNT - 1);
        uecli.historyindex = i;
    }
    
    uecli_assert(uecli.history[uecli.historyindex][0] != '\0');
    return uecli.history[uecli.historyindex];
}

// ������չ�ַ����� ����ֵ��ʾ�Ƿ����
static char ProcessESCSequence(char c)
{
    const char* tstr;
    switch (c)
    {
    case '\x1b':    // ��ʼ����
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
    case 'A':       // �ϼ�ͷ
        if (2 == uecli.extSequenceState)
        {
            uecli.extSequenceState = 0;
            if (!IsBlankString(tstr = GetPriveHistory()))
                CopyStringToCmdline(&uecli.instring, tstr);
            return 0;
        }
        break;
    case 'B':       // �¼�ͷ
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
#endif

// ********************************************************************************************
// ���ȫ֧��

#if UECLI_01_COMPLETE>0

// ��ָ�����ȱȽ��ַ���
static int strncmpNocase(const char* src, const char* dst, unsigned int len)
{
    // ��������
    uecli_assert(src);
    uecli_assert(dst);

    char c1, c2;
    int ret = 0;

    // �ж�ָ���Ƿ����
    if (src == dst) return 0;

    for (; c2 = *src, '\0' != (c1 = *dst); ++src, ++dst)
    {
        if (c1 >= 'A' && c1 <= 'Z')
            c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z')
            c2 += 32;

        if (c2 != c1)
            break;
        else if (!--len)
            return 0;
    }

    // ��ʽ�������ֵ
    ret = c2 - c1;
    if (ret < 0)
        ret = -1;
    else if (ret > 0)
        ret = 1;
    return(ret);
}

// ��ӡƥ�������ַ�����
static void PrintMatchCmdTable(const char** matchtable, int len)
{
    uecli_PrintString(UECLI_NEWLINE);
    const char* formatstr = NULL;
    
    // ��ӡ�б�
    for (int i = 0; i < len; ++i)
    {
        if ((i + 1) % 4 == 0)
            formatstr = "%s" UECLI_NEWLINE;
        else
            formatstr = "%-16s";

        uecli_snprintf(uecli.tmpString, UECLI_CFG_TEMP_MAXLEN, formatstr, matchtable[i]);
        uecli_PrintString(uecli.tmpString);
    }
    
    // ��ӡ��ʾ��
    uecli_PrintString(UECLI_NEWLINE);
    PrintCLIPrompt();
    uecli_PrintString(GetCmdlineString(&uecli.instring));
}

// ����ƥ������
static int SearchCompleteCmd(const char** strtable, int startindex, const uecli_MenuItem* list)
{
    int strlength = GetCmdlineLength(&uecli.instring);
    for (; list->pdata; ++list)
    {
        if (!strncmpNocase(GetCmdlineString(&uecli.instring), list->exename, strlength))
        {
            if (startindex >= UECLI_CFG_AUTOCOMP_NUM - 1)
            {
                strtable[UECLI_CFG_AUTOCOMP_NUM - 1] = "...";
                startindex = UECLI_CFG_AUTOCOMP_NUM;
                break;
            }
            strtable[startindex] = list->exename;
            ++startindex;
        }
    }
    return startindex;
}

// �Զ���ȫ֧��
static void AutoCompleteInstring(void)
{
    // ������ֱ����ʾ������Ϣ
    if (!GetCmdlineLength(&uecli.instring))
    {
        uecli_PrintString(UECLI_NEWLINE);
        Cmd_HelpMain(0, 0);
        PrintCLIPrompt();
        return;
    }

    const char* matchStringTable[UECLI_CFG_AUTOCOMP_NUM];
    int i = 0;

    // ƥ��ϵͳ����
    i = SearchCompleteCmd(matchStringTable, i, GetSyscmdHandle());
    if (i < UECLI_CFG_AUTOCOMP_NUM)
        i = SearchCompleteCmd(matchStringTable, i, GetUserCmdHandle());

    // ֻ�ҵ�1��ƥ�� ֱ�����
    if (1 == i)
        CopyStringToCmdline(&uecli.instring, matchStringTable[0]);
    else if (i)
        PrintMatchCmdTable(matchStringTable, i);
}
#endif

// ���������ַ�
static char cli_echo(char c)
{
    UECLI_LOG("���յ��ַ�0X%02x:%c", c, c);

    // ������������
    switch (c)
    {
    case '\r':
    case '\n':  // �س�����
        uecli_PrintString(UECLI_NEWLINE);
        return '\n';
        break;
    case '\b':  // �˸��ַ�
        if (DeleteCmdlineChar(&uecli.instring))
            uecli_PrintString("\b \b");
        break;
        
    #if UECLI_01_COMPLETE>0
    case '\t':  // �Ʊ��
        AutoCompleteInstring();
        break;
    #endif  
    
    #if UECLI_01_HISTORY>0
    case '\x1b': // ESC ��չ���ܷ�
        ProcessESCSequence(c);
        break;
    #endif
    
    default:    // �����ɴ�ӡ�ַ�
    #if UECLI_01_HISTORY>0
        if (!ProcessESCSequence(c))
            break;
    #endif
        if (uecli_isprintfchar(c) && AppendCmdlinechar(&uecli.instring,c))
            uecli_port_out((const uint8_t*)&c, 1);		/* ������� */
    }

    return c;
}

// ����ƥ���������
static const uecli_MenuItem* SearchMatchCommand(const char* cmdstring)
{
    const uecli_MenuItem* ptr = NULL;
    
    // ����ϵͳ�����б�
    for (ptr = GetSyscmdHandle(); ptr->pdata; ++ptr)
    {
        if (!uecli_strcasecmp(cmdstring, ptr->exename))
            return ptr;
    }
    
    // �����û������б�
    for (ptr = GetUserCmdHandle(); ptr->pdata; ++ptr)
    {
        if (!uecli_strcasecmp(cmdstring, ptr->exename))
            return ptr;
    }
    return NULL;
}

// ********************************************************************************************
// �ⲿ�ӿں���

/// \brief ��ʼ��CLI�����ж���
///
/// \param cmdlist ��֧�ֵ������
/// \return void
void uecli_Initialize(const uecli_MenuItem* cmdlist)
{
    memset(&uecli,0,sizeof(uecli));
    uecli.cmdlist = cmdlist;

    uecli_port_init();      // �ײ�ӿڳ�ʼ��
    uecli_PrintString(STRING_COPYRIGHT_INFO);
    UpdateCLIPrompt();      // ������ʾ���ַ���
    PrintCLIPrompt();       // ��ӡ��ʾ��
}

#if UECLI_01_IN_HOOK>0

/// \brief ���빳��֧�� ���ú����е��������ݶ�����ָ���ĺ���
///
/// \param pfun ���Ӵ�����
/// \return void* ֮ǰ�Ĺ���ָ��
void* uecli_SetHook(void(*pfun)(int,char**))
{
    void* temp = (void*)(uecli.hookfun);
    uecli.hookfun = (void(*)(int,const char**))pfun;

    return temp;
}

/// \brief ����Ƿ����ù���
///
/// \return bool
bool uecli_IsValidHook(void)
{
    return uecli.hookfun != NULL;
}
#endif

/// \brief ��������������ַ������зָ��������ִ��
///
/// \param cmdline ����������ַ���
/// \return void
void uecli_ExeCmdine(const char* cmdstring)
{
    char strbuff[UECLI_CFG_STR_MAXLEN];
    char* argcbuff[UECLI_CFG_MAX_ARGNUM];

    // �ָ��ַ�����ı�ԭ���ݣ��ȿ���һ�ݳ���
    uecli_strncpy(strbuff, cmdstring, UECLI_CFG_STR_MAXLEN);
    int count = SplitString(strbuff, argcbuff, UECLI_CFG_MAX_ARGNUM);

    // ������ִ������
    if (count)
    {
        UECLI_LOG("���յ�������:%s", cmdstring);

        const uecli_MenuItem* phand = SearchMatchCommand(argcbuff[0]);
        if (NULL != phand)
        {
        #if UECLI_01_HISTORY>0
            SaveHistory(cmdstring);
        #endif
            if (UECLI_TYPE_FUN == phand->itemType)
                ((void(*)(int, char**))(uint32_t)phand->pdata)(count, argcbuff);
            else
            {
            #if UECLI_01_SUBMENU>0
                PushMenuStack(phand);
            #endif
            }
        }
        else
        {
            UECLI_LOG("\"%s\" ������Ч������", argcbuff[0]);
            uecli_snprintf(uecli.tmpString, UECLI_CFG_TEMP_MAXLEN,STRING_INVALID_COMMAND, argcbuff[0]);
            uecli_PrintString(uecli.tmpString);
            uecli_PrintString(UECLI_NEWLINE);
        }
    }
    
    // ������������� ��ӡ��ʾ��
    ClearCmdlineString(&uecli.instring);
    PrintCLIPrompt();
}

/// \brief ������յ����ַ�����
///
/// \param recchar ���յ����ַ���������
/// \param len ��������
/// \return void
void uecli_ProcessRecChar(const char recchar[], int len)
{
    // ����Ƿ�����
    if (uecli.hookfun)
    {
        uecli.hookfun(len,&recchar);
        return;
    }
    
    // ������յ���ÿ���ַ�
    for (int i = 0; i < len; ++i)
    {
        if ('\n' == cli_echo(recchar[i]))
        {
            /* ִ�������� */
            uecli_ExeCmdine(GetCmdlineString(&uecli.instring));
            break;
        }
    }
}

/// \brief ��ӡָ�����ַ���
///
/// \param str ����ӡ���ַ���
/// \return void
void uecli_PrintString(const char* str)
{
    if (str)
        uecli_port_out(str, uecli_strlen(str));
}

