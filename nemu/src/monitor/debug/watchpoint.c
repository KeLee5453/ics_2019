#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};//监视点池
static WP *head = NULL, *free_ = NULL;//head用于组织使用中的监视点结构, free_用于组织空闲的监视点结构, 
static int used_next;//监视下一个使用的wp的序号
static WP *wptemp;//辅助变量
void init_wp_pool() {//初始化
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;//设置序号
    wp_pool[i].next = &wp_pool[i + 1];//链接wp
	wp_pool[i].oldvalue=0;//初始化旧值
	wp_pool[i].hitnum=0;//初始化命中次数
  }
  wp_pool[NR_WP - 1].next = NULL;//设置最后一个节点

  head = NULL;
  free_ = wp_pool;
  used_next=0;
}

/* TODO: Implement the functionality of watchpoint */
bool new_wp(char* args){
	if (free_==NULL) assert(0);//空闲的监视点不存在
	WP* result=free_;//将要分配的监视点
	free_=free_->next;//空闲指向下一个
	
	result->NO=used_next;//序号为下一个要使用的序号
	used_next++;//序号+1
	result->next=NULL;
	strcpy(result->e,args);//把表达式复制到监视点里
	result->hitnum=0;
	bool success;
	result->oldvalue=expr(result->e,&success);//求值
	if(success==false){
		printf("Error: Expr() fault in new_wq.\n");
		return false;
	}
	if(head==NULL)//加入已经用了的链表，头为空，那就是头
		head=result;
	else{
		result->next=head;//加入到头部，自己就是头
		head=result;
	}
	printf("Success: Set watchpoint %d, oldvalue=%d. \n",result->NO,result->oldvalue);
	return true;
}
bool free_wp(int num){
	WP* tm=NULL;
	if(head==NULL){//如果没有使用的监视点
		printf("Error: No watchpoint.\n");
		return false;
	}
	if(head->NO==num){//如果头就是要删除的监视点
		tm=head;
		head=head->next;
	}
	else{//否则遍历链表找到要删除的监视点并且把后面的节点往前挪
		wptemp=head;
		while(wptemp->NO!=num&&wptemp->next!=NULL){
			if(wptemp->next->NO==num){
				tm=wptemp->next;
				wptemp->next=tm->next;
				break;
			
		}
		wptemp=wptemp->next;
	}
	}
	if(tm!=NULL){//加入空闲链表
		tm->next=free_;
		free_=tm;
		return true;
	}
	return false;
}
void print_wp(){//打印监视点
	if(head==NULL){//若为空
		printf("Error: No watchpoint.\n");
		return;
	}
	printf("Watchpoint:\n");
	printf("NO.		Expr		Hittimes\n");
	wptemp=head;
	while(wptemp!=NULL){
		printf("%d		%s		%d\n",wptemp->NO,wptemp->e,wptemp->hitnum);
		wptemp=wptemp->next;
	}
	
}
bool watch_wp(){//进行监视点各表达式的求值，若有变化返回true，并打印信息
	bool success;
	int result;
	if(head==NULL)return true;
	wptemp=head;
	while(wptemp!=NULL){
			result=expr(wptemp->e,&success);
			if(result!=wptemp->oldvalue){//发生改变
				wptemp->hitnum++;//触发次数加一
				printf("Hit watchpoint %d, expr: %s.\n",wptemp->NO,wptemp->e);
				printf("    Old value: %d;\n    New value: %d.\n",wptemp->oldvalue,result);
				wptemp->oldvalue=result;//更新旧值
				return false;//触发一次就返回
				
			}
			wptemp=wptemp->next;
	}
	return true;
}

