
#include <stdio.h>
#include "uecli.h"

int uecli_port_in(void* buff, uint16_t maxnum);

void estrtoitest(int argc, char** argv)
{
    if (argc > 1)
    {
        int32_t temp;
        int res = estrtoi(argv[1], &temp);

        char tcharbuff[128];
        esnprintf(tcharbuff, 128, "ִ�н����%d %o %x, ����ֵ��%d\r\n", temp, temp, temp, res);
        uecli_PrintString(tcharbuff);
    }

    argc;
    argv;
}
void testprintf(int argc, char** argv)
{
    char tstr[128];
    tstr[0] = '\0';

    if (argc > 2)
    {
        int valu = 0;
        if (estrtoi(argv[2], &valu))
            esnprintf(tstr,128,argv[1], valu);
        else
            esnprintf(tstr, 128, "%s", "��������ȷ�Ĳ���\r\n");
    }
    else if (argc > 1)
        esnprintf(tstr, 128, argv[1]);
    else
        esnprintf(tstr, 128, "%s", "��������ȷ�Ĳ���\r\n");

    uecli_PrintString(tstr);
    argc;
    argv;
}

void menufun(int argc, char** argv)
{
    uecli_PrintString("testmenu\n");
    argc;
    argv;
}

const uecli_MenuItem submenu[] =
{
    UECLI_DECLARE_COMMAND(menufun,"testmenu", "�Ӳ˵�����",""),
    UECLI_DECLARE_END()
};

void eputchar(char chr)
{
    putchar(chr);
}

const uecli_MenuItem handtalbe[]=
{
    UECLI_DECLARE_COMMAND(estrtoitest,"estrtoi", "����estrtoi"),
    UECLI_DECLARE_COMMAND(testprintf,"eprintf", "����eprintf"),
    UECLI_DECLARE_SUBMENU(submenu,"menu1", "�Ӳ˵�����"),
    UECLI_DECLARE_END()
};

int main(void)
{
    uecli_Initialize(handtalbe);

    char buff[1024];
    int len;

    for (;;)
    {
        len = uecli_port_in(buff, 1024);	/* ��ȡ���� */
        uecli_ProcessRecChar(buff, len);
    }
}


