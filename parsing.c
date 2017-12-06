#include <stdio.h>
#include <stdlib.h>

// 为了windows平台和linux平台兼容
#ifdef _WIN32
#include <string.h>

static char buffer[2048];
// windows上没有readline，因此这里只能自己写一个
char* readline(char* prompt){
    fputs(prompt,stdout);
    fgets(buffer,2048,stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy,buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

void add_history(char* unused){}

#else
#include <readline/readline.h>
#include <readline/history.h>
#endif
#include "mpc.h"
// 检测除零异常的标志位,1表示产生异常，0表示未产生异常
int is_div_error = 0;
// 检测min和max后面没有数的异常标志位
int is_min_max_error = 0;
// 计算表达式结果的函数
long eval_op(long x,char* op,long y) {
    if(0 == strcmp(op,"+")) {
        return x + y;
    }
    if(0 == strcmp(op,"-")) {
        return x - y;
    }
    if(0 ==strcmp(op,"*")) {
        return x * y;
    }
    if(0 == strcmp(op,"/")) {
        return x / y;
    }
    if(0 == strcmp(op,"%")) {
        return x % y;
    }
    if(0 == strcmp(op,"^")) {
        // 乘方计算
        int mul_res = 1;
        if(0 == y){
            return mul_res;
        }
        for(int i=0;i<y;i++){
           mul_res *= x; 
        }
        return mul_res; 
    }
    return 0;
}

// 算最小值函数
long min_value_handle(mpc_ast_t* t) {
    int result = atoi(t->children[2]->contents);
    int i = 2;
    i++;
    while(t->children[i] != NULL){
       if(result < atoi(t->children[i]->contents)) {
           result = atoi(t->children[i]->contents);
       }
       i++;
    }
    return result;  
}

// 算最大值函数
long max_value_handle(mpc_ast_t* t) {
    int result = atoi(t->children[2]->contents);
    int i = 2;
    i++;
    while(t->children[i] != NULL){
       if(result < atoi(t->children[i]->contents)) {
           result = atoi(t->children[i]->contents);
       }
       i++;
    }
    return result;  
}
// 递归处理树
long eval(mpc_ast_t* t) {
    // 如果是tag为number，则直接返回数字
    if(strstr(t->tag,"number")) {
        return atoi(t->contents);
    }
    // 操作数是第一个孩子节点
    char* op = t->children[1]->contents;
    // 这里如果是min或max则需要特殊处理
    if (0 == strcmp(op,"min") && t->children[2] != NULL) {
        return min_value_handle(t);
    }
    if (0 == strcmp(op,"min") && t->children[2] == NULL) {
        is_min_max_error = 1;
        return 0;
    }
    if (0 == strcmp(op,"max") && t->children[2] != NULL) {
        return max_value_handle(t);
    }
    if (0 == strcmp(op,"max") && t->children[2] == NULL) {
        is_min_max_error = 1;
        return 0;
    }

    // 递归处理下一个孩子节点,将结果保存下来
    long x = eval(t->children[2]);

    int i=3;
    while(strstr(t->children[i]->tag, "expr")) {
        // 发现除零异常
        if(0 == strcmp(op,"/") && 0 == eval(t->children[i])) {
            is_div_error = 1;
            break;
        } else if(0 == strcmp(op,"%") && 0 == eval(t->children[i])) {
            is_div_error = 1;
            break;
        } else {
            x = eval_op(x,op,eval(t->children[i]));
            i++;
        }
    }
    return x;
}

int main(int argc,char ** argv){
    // 使用mpc生成解析器
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                   \
         number   : /-?[0-9]+/;                             \
         operator : '+' | '-' | '*' | '/' | '%' | '^' | 'min' | 'max';            \
         expr     : <number> | '(' <operator> <expr>+ ')' ; \
         lispy    : /^/ <operator> <expr>+ /$/;             \
        ",
    Number,Operator,Expr,Lispy);

    // 打印版本和退出信息
    puts("Lispy Version 0.0.0.0.1");
    puts("Press Ctrl+c to Exit\n");

    // 循环接受输入
    while(1){
        //fputs("lispy> ",stdout);
        char* input = readline("lispy> ");
        
        add_history(input);
        // printf("No you're a %s",input);

        // 使用mpc解析输入
        mpc_result_t r;
        if (mpc_parse("<stdin>",input,Lispy,&r)) {
            // 解析成功
            // mpc_ast_print(r.output);
            // 打印运算结果
            long result = eval(r.output);
            if(is_div_error){
                printf("Find zero div error!!\n");
            } else {
                printf("%li\n",eval(r.output));
            }
            if(is_min_max_error){
                printf("Need a number");
            } else {
                printf("%li\n",eval(r.output));
            }
            mpc_ast_delete(r.output);
        } else {
            // 解析失败
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }
    // 释放解析器
    mpc_cleanup(4,Number,Operator,Expr,Lispy);
    return 0;
}