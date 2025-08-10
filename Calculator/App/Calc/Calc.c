/******************************************************
 *  ģ�����ƣ�Calc.c
 *  ժ    Ҫ���򵥼�����
 *  ��ǰ�汾��1.0.0
 *  ��    �ߣ�CMJ
 *  ������ڣ�2025��08��09��
 *  ��    �ݣ�
 *  ע    �⣺
*******************************************************/
/* *******************************************************
                    ����ͷ�ļ�
**********************************************************/
#include "Calc.h"
#include "UART1.h"
#include <stdlib.h>   /* Ϊ������ exit���������Լ�ʵ�� */
/* *******************************************************
                    �궨��
**********************************************************/

/* *******************************************************
                    ö�ٽṹ�嶨��
**********************************************************/
/* �Ǻ����ͣ��ս�� */
typedef enum {
    T_EOF,      /* �ļ����� */
    T_NUM,      /* ������������ */
    T_PLUS,     /* + */
    T_MINUS,    /* - */
    T_MUL,      /* * */
    T_DIV,      /* / */
    T_LP,       /* ( */
    T_RP        /* ) */
} TokenType;
/* �ǺŽṹ�壺���� + ��ֵ���� T_NUM ʱ��Ч�� */
typedef struct {
    TokenType type;
    double value;
} Token;

/* *******************************************************
                    �ڲ�����
**********************************************************/
static const u8 *src;   /* ָ��ǰԴ���ַ� */
static Token token;       /* ��ǰ�Ǻ� */

/* *******************************************************
                    �ڲ���������
**********************************************************/
/* �����հ׷���������һ���Ǻ� */
static void next_token(void);
/* factor ::= [ '-' ] ( ���� | '(' expr ')' ) */
static double factor(void);
/* term ::= factor { ('*' | '/') factor } */
static double term(void);
/* expr ::= term { ('+' | '-') term } */
static double expr(void);   /* ���ʽ */

static void exit(int code);
/* *******************************************************
                    �ڲ�����ʵ��
**********************************************************/
static void next_token(void)
{
    /* 1. �����հ��ַ����ո��Ʊ�� */
    while (*src == ' ' || *src == '\t') ++src;

    /* 2. �ļ����� */
    if (*src == '\0') {
        token.type = T_EOF;
        return;
    }

    /* 3. ��ȡ���֣�֧��С����Ϳ�ѧ�������� */
    if (isdigit(*src) || *src == '.') {
        u8 *endptr;
        /* strtod �Զ��� src ת���� double��endptr ָ��������λ�� */
        token.value = strtod((const char *)src, &endptr);
        src = endptr;               /* ����ɨ��λ�� */
        token.type = T_NUM;
        return;
    }

    /* 4. ���ַ��Ǻ� */
    switch (*src) {
    case '+': token.type = T_PLUS;  break;
    case '-': token.type = T_MINUS; break;
    case '*': token.type = T_MUL;   break;
    case '/': token.type = T_DIV;   break;
    case '(': token.type = T_LP;    break;
    case ')': token.type = T_RP;    break;
    default:
        printf("�Ƿ��ַ�: %c\n", *src);
        exit(1);
    }
    ++src;  /* ǰ��һ���ַ� */
}

/* factor ::= [ '-' ] ( ���� | '(' expr ')' ) */
static double factor(void)
{
    double sign = 1.0;
	double v;

    /* ����һԪ���� */
    if (token.type == T_MINUS) {
        next_token();
        sign = -1.0;
    }

    if (token.type == T_NUM) {
        v = sign * token.value;
        next_token();
        return v;
    }

    if (token.type == T_LP) {
        next_token();           /* ���� '(' */
        v = sign * expr();
        if (token.type != T_RP) {
            printf("ȱ�� ')'\n");
            exit(1);
        }
        next_token();           /* ���� ')' */
        return v;
    }

    printf("�﷨�����������ֻ�������\n");
    exit(1);
}

/* term ::= factor { ('*' | '/') factor } */
static double term(void)
{		
	
	double rhs;
    double v = factor();    /* ��ȡ��һ�� factor */
    while (1) {
        TokenType op = token.type;
        if (op != T_MUL && op != T_DIV) break;
        next_token();               /* ��������� */
        rhs = factor();      /* �Ҳ� factor */
        if (op == T_MUL)
            v *= rhs;
        else {
            if (fabs(rhs) < 1e-12) {   /* ��ֹ���� */
                fprintf(stderr, "�������\n");
                exit(1);
            }
            v /= rhs;
        }
    }
    return v;
}

/* expr ::= term { ('+' | '-') term } */
static double expr(void)
{		
	double rhs;
    double v = term();      /* ��ȡ��һ�� term */
    while (1) {
        TokenType op = token.type;
        if (op != T_PLUS && op != T_MINUS) break;
        next_token();
        rhs = term();
        if (op == T_PLUS)
            v += rhs;
        else
            v -= rhs;
    }
    return v;
}

static void exit(int code)
{
    (void)code;          /* ������������� */
    while (1);           /* ��ѭ�������ߵ�ơ���ӡ������Ϣ */
}
/* *******************************************************
                    API����ʵ��
**********************************************************/
/* �����ַ������ʽ������ double ��� */
double calc(const u8 *s)
{		
	double result;
    src = s;            /* ��ʼ��Դ��ָ�� */
    next_token();       /* Ԥ����һ���Ǻ� */
    result = expr();
    /* ���������Ӧֻʣ EOF������˵���ж����ַ� */
    if (token.type != T_EOF) {
        printf("�����ַ�\n");
        exit(1);
    }
    return result;
}