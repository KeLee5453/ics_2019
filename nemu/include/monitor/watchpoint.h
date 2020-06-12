#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"
typedef struct watchpoint {
	int NO;//监视点序号
	struct watchpoint *next;//下一个监视点

	/* TODO: Add more members if necessary */
	int oldvalue;//监视点的旧值
	char e[32];//监视的表达式
	int hitnum;//命中次数

} WP;
bool new_wp(char * args);//新建监视点
bool free_wp(int num);//删除监视点
void print_wp();//打印监视点
bool watch_wp();//监视点值变化
#endif
