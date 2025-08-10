/******************************************************
 *  模块名称：Calc.c
 *  摘    要：简单计算器
 *  当前版本：1.0.0
 *  作    者：CMJ
 *  完成日期：2025年08月09日
 *  内    容：
 *  注    意：
*******************************************************/
/* *******************************************************
                    包含头文件
**********************************************************/
#include "Calc.h"
#include "UART1.h"
#include <stdlib.h>   /* 为了声明 exit，但下面自己实现 */
/* *******************************************************
                    宏定义
**********************************************************/

/* *******************************************************
                    枚举结构体定义
**********************************************************/
/* 记号类型：终结符 */
typedef enum {
    T_EOF,      /* 文件结束 */
    T_NUM,      /* 浮点数字面量 */
    T_PLUS,     /* + */
    T_MINUS,    /* - */
    T_MUL,      /* * */
    T_DIV,      /* / */
    T_LP,       /* ( */
    T_RP        /* ) */
} TokenType;
/* 记号结构体：类型 + 数值（仅 T_NUM 时有效） */
typedef struct {
    TokenType type;
    double value;
} Token;

/* *******************************************************
                    内部变量
**********************************************************/
static const u8 *src;   /* 指向当前源码字符 */
static Token token;       /* 当前记号 */

/* *******************************************************
                    内部函数声明
**********************************************************/
/* 跳过空白符并读入下一个记号 */
static void next_token(void);
/* factor ::= [ '-' ] ( 数字 | '(' expr ')' ) */
static double factor(void);
/* term ::= factor { ('*' | '/') factor } */
static double term(void);
/* expr ::= term { ('+' | '-') term } */
static double expr(void);   /* 表达式 */

static void exit(int code);
/* *******************************************************
                    内部函数实现
**********************************************************/
static void next_token(void)
{
    /* 1. 跳过空白字符：空格、制表符 */
    while (*src == ' ' || *src == '\t') ++src;

    /* 2. 文件结束 */
    if (*src == '\0') {
        token.type = T_EOF;
        return;
    }

    /* 3. 读取数字（支持小数点和科学计数法） */
    if (isdigit(*src) || *src == '.') {
        u8 *endptr;
        /* strtod 自动把 src 转换到 double，endptr 指向解析后的位置 */
        token.value = strtod((const char *)src, &endptr);
        src = endptr;               /* 更新扫描位置 */
        token.type = T_NUM;
        return;
    }

    /* 4. 单字符记号 */
    switch (*src) {
    case '+': token.type = T_PLUS;  break;
    case '-': token.type = T_MINUS; break;
    case '*': token.type = T_MUL;   break;
    case '/': token.type = T_DIV;   break;
    case '(': token.type = T_LP;    break;
    case ')': token.type = T_RP;    break;
    default:
        printf("非法字符: %c\n", *src);
        exit(1);
    }
    ++src;  /* 前进一个字符 */
}

/* factor ::= [ '-' ] ( 数字 | '(' expr ')' ) */
static double factor(void)
{
    double sign = 1.0;
	double v;

    /* 处理一元负号 */
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
        next_token();           /* 跳过 '(' */
        v = sign * expr();
        if (token.type != T_RP) {
            printf("缺少 ')'\n");
            exit(1);
        }
        next_token();           /* 跳过 ')' */
        return v;
    }

    printf("语法错误：期望数字或左括号\n");
    exit(1);
}

/* term ::= factor { ('*' | '/') factor } */
static double term(void)
{		
	
	double rhs;
    double v = factor();    /* 先取第一个 factor */
    while (1) {
        TokenType op = token.type;
        if (op != T_MUL && op != T_DIV) break;
        next_token();               /* 跳过运算符 */
        rhs = factor();      /* 右侧 factor */
        if (op == T_MUL)
            v *= rhs;
        else {
            if (fabs(rhs) < 1e-12) {   /* 防止除零 */
                fprintf(stderr, "除零错误\n");
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
    double v = term();      /* 先取第一个 term */
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
    (void)code;          /* 避免编译器警告 */
    while (1);           /* 死循环，或者点灯、打印调试信息 */
}
/* *******************************************************
                    API函数实现
**********************************************************/
/* 计算字符串表达式，返回 double 结果 */
double calc(const u8 *s)
{		
	double result;
    src = s;            /* 初始化源码指针 */
    next_token();       /* 预读第一个记号 */
    result = expr();
    /* 计算结束后应只剩 EOF，否则说明有多余字符 */
    if (token.type != T_EOF) {
        printf("多余字符\n");
        exit(1);
    }
    return result;
}