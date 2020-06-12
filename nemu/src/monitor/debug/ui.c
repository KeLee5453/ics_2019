#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
//函数声明
void cpu_exec(uint64_t);
static int cmd_help(char *args);
static int cmd_c(char *args);
static int cmd_q(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_x(char *args);
static int cmd_d(char *args);

void isa_reg_display();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}
//继续运行
static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}
//退出运行
static int cmd_q(char *args) {
	return -1;
}
//单步调试
static int cmd_si(char *args){
	uint64_t N=0;
	if(args==NULL)//没有参数时候
		N=1;//默认为1
	else{
		int flag=sscanf(args,"%lu",&N);//读取参数
		if(flag<=0){//解析失败
			printf("Error: Args error in cmd_si\n");
			return 0;
		}
	}
	cpu_exec(N);
	return 0;
}
//打印程序状态
static int cmd_info(char *args) {
	char c;
	if(args ==NULL ){//无参数报错
		printf("Error: Args error in cmd_info\n");
		return 0;
	}
	int flag=sscanf(args,"%c",&c);
	if(flag<=0){//解析失败
		printf("Error: Args error in cmd_info\n");
		return 0;
	}
	if (c=='r'){//打印寄存器信息
		isa_reg_display();
		return 0;
	}
	if(c=='w'){
		print_wp();
		return 0;
	}
	printf("Error: Args error in cmd_info\n");
	return 0;
}
static int cmd_p(char *args) {//计算表达式的值
	//char s[256];
	//int flag=sscanf(args,"%s",s);//解析参数
	//if(flag<=0){//解析失败
	//	printf("Args error in cmd_p.\n");
	//	return 0;
	//}
	bool success=false;

	uint32_t r=expr(args, &success);

	if(success==false){
		printf("Error: in evaluation .\n");
		return 0;
	}
	else{
		printf("The expression's value is %u .\n",r);
		return 0;
	}
	
}
static int cmd_w(char *args) {
	new_wp(args);
	return 0;
}
static int cmd_x(char *args) {
	int len=0;//长度
	vaddr_t addr;//起始地址
	char *ptr;//表达式
	len=strtoul(args,&ptr,10);//解析出整数
	if(ptr==NULL){//解析失败
		printf("Error: Args error in cmd_x\n");
		return 0;
	}
	bool success;
	addr=expr(ptr,&success);//求值
	if(success==false){
		printf("Error: in evaluation .\n");
		return 0;
	}
	printf("Memory:");//打印内存
	for(int i=0;i<len;i++){
		printf("\n0x%x:	0x%08x",addr+4*i,isa_vaddr_read(addr+4*i,4));
	}
	printf("\n");
	return 0;
}

static int cmd_d(char *args) {
	int num=0;
	int flag=sscanf(args,"%d",&num);//解析参数
	if(flag<=0){//解析失败
		printf("Error: Args error in cmd_x\n");
		return 0;
	}
	int r=free_wp(num);
	if(r==true)printf("Success: Delete watchpoint %d.\n",num);
  return 0;
}


static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  {"si","Args:[N]; Eg:si 10; Execute [N] instructions step by step",cmd_si},
  {"info","Args:r/w; Eg:info r; Erint information about registers or watchpoint", cmd_info},
  {"p","Args:[EXPR]; Eg:p $eax+1; Evaluate the expression [EXPR]",cmd_p},
  {"x","Args:[N],[EXPR]; Eg:x10 $esp; Evaluate the expression [EXPR] and take the result as the starting memory address, and output [N] bytes in hexadecimal",cmd_x},
  {"w","Args:[EXPR]; Eg:w *0x2000; Set the watchpoint",cmd_w},
  {"d","Args:[N]; Delete the watchpoint with sequence [N]",cmd_d},
//命令提示符
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}



void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
