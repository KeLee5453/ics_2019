#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
extern const char *regsl[];
extern const char *regsw[];
extern const char *regsb[];
uint32_t eval(int p,int q);
bool check_parentheses(int p,int q);
int find_main_operator(int p,int q);
enum {
  TK_NOTYPE = 256, TK_EQ,TK_NUM,TK_HEX,TK_REG,TK_NEQ,TK_AND,TK_OR,TK_NEGATIVE,TK_DEREF,
  /* TODO: Add more token types */

};
typedef int elementtype;
typedef struct {
	elementtype element[MAX];
	int size;
}STACK;
STACK makeNull(STACK *s)//将栈置空 
{
	s->size=0;
	return *s;
}
bool empty(STACK *s)//判断栈是否为空
{
	if(s->size==0)
		return true;
	return false;
}
int size(STACK *s)//返回栈的大小
{
	return s->size;
}
elementtype top(STACK *s)//返回栈顶元素
{
	if(s->size==0)
		exit(1);
	else
		return s->element[0];
}
void push(STACK *s,elementtype e)//压栈操作
{
	if(s->size==0){
		s->element[0]=e;
		s->size++;
	}
	else{
		for(int i=s->size;i>0;i--){
			s->element[i]=s->element[i-1];
		}
		s->element[0]=e;
		s->size++;
	}
}
void pop(STACK *s)//把栈顶元素弹出栈
{
	if(s->size==0)
		return;
	if(s->size==1){
		s->size=0;
		return;
	}
	for(int i=0;i<s->size-1;i++){
		s->element[i]=s->element[i+1];
	}
	s->size--;
} 
static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
	{" +", TK_NOTYPE},    // spaces
	{"0x[1-9A-Fa-f][0-9A-Fa-f]*",TK_HEX},
	{"0|[1-9][0-9]*", TK_NUM},//整数
	{"\\$(eax|edx|ecx|ebx|ebp|esi|edi|esp|ax|dx|cx|bx|pc|bp|si|di|sp|al|dl|cl|bl|ah|dh|ch|bh|eip)",TK_REG},
	{"==",TK_EQ},
	{"!=",TK_NEQ},
	{"&&",TK_AND},
	{"\\|\\|",TK_OR},
	{"!",'!'},
	{"\\+", '+'},         // plus
	{"\\-", '-'},			//减	
	{"\\*", '*'},			//乘	
	{"\\/", '/'},			//除
	{"\\)", ')'},			//右括号
	{"\\(", '('},			//左括号  
 
    

};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("Error: regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("Match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;
		if(substr_len>32){
				printf("Error: The token's length is out of range, at position %d with len %d.",position-substr_len, substr_len);
				assert(0);
				return false;
			}
		if(rules[i].token_type==TK_NOTYPE)
			break;//跳过空格
		else{
			
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
			  				
			tokens[nr_token].type=rules[i].token_type;//类型赋值
			switch (rules[i].token_type) {//运算符不用处理，可以通过类型确定
				case TK_NUM://数字直接加入
					strncpy(tokens[nr_token].str,substr_start,substr_len);
					*(tokens[nr_token].str+substr_len)='\0';
					break;
				case TK_HEX://16进制除去0x
					strncpy(tokens[nr_token].str,substr_start+2,substr_len-2);
					*(tokens[nr_token].str+substr_len-2)='\0';
					break;
				case TK_REG://寄存器除去$符号
					strncpy(tokens[nr_token].str,substr_start+1,substr_len-1);
					*(tokens[nr_token].str+substr_len-1)='\0';
					break;
				
				//default: TODO();
        }
		printf("Success: Correctly identified: nr_token=%d,type=%d,str=%s\n",nr_token,tokens[nr_token].type,tokens[nr_token].str);
		nr_token+=1;
        break;
      }
	 }
    }

    if (i == NR_REGEX) {
      printf("Error: No match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  
  return true;
}

uint32_t expr(char *e, bool *success) {

  if (!make_token(e)) {

    *success = false;
    return 0;
  }

   /* TODO: Insert codes to evaluate the expression. */
  for(int i=0;i<nr_token;i++){
	  if(tokens[i].type=='-'){//区分负运算与减法
		  if(i==0)
			  tokens[i].type=TK_NEGATIVE;//若前一字符为空，则为负运算
		  if(tokens[i-1].type==TK_NUM||tokens[i-1].type==TK_HEX||tokens[i-1].type==')');//若前一个为数或者）则为减法
		  else
			  tokens[i].type=TK_NEGATIVE;//否则是负运算
	  }
	  if(tokens[i].type=='*'){//区分乘法与指针解引用
		  if(i==0)
			  tokens[i].type=TK_DEREF;//若前一字符为空，则为指针解引用
		  if(tokens[i-1].type==TK_NUM||tokens[i-1].type==TK_HEX||tokens[i-1].type==')');//若前一个为数或者）则为乘法
		  else
			  tokens[i].type=TK_DEREF;//否则是指针解引用
	  }
		  
  }
  *success=true;
  return eval(0,nr_token-1);//计算整个表达式

}

uint32_t eval(int p, int q) {
	//printf("eval中p为%d,qwei %d\n",p,q);
	if (p > q) {
		/* Bad expression */
		printf("Error: p>q in eval() in p=%d, q=%d.\n",p,q);
		assert(0);
	}
	else if (p == q) {//单个字符

		/* Single token.
		 * For now this token should be a number.
		 * Return the value of the number.
		 */
		if(tokens[q].type==TK_NUM){//十进制数字
			char *ptr;
			int r=strtoul(tokens[q].str,&ptr,10);
			return r;
		}
		else if(tokens[q].type==TK_HEX){//十六进制
			char *ptr;
			int r=strtoul(tokens[q].str,&ptr,16);
			return r;
		}
		else if(tokens[q].type==TK_REG){//寄存器
			for(int k=0;k<8;k++){//依次遍历
				if(strcmp(tokens[p].str,regsl[k])==0)
					return reg_l(k);
				if(strcmp(tokens[p].str,regsw[k])==0)
					return reg_w(k);
				if(strcmp(tokens[p].str,regsb[k])==0)
					return reg_b(k);
		}
		if(strcmp(tokens[p].str,"pc")==0||strcmp(tokens[p].str,"eip")==0)
			return cpu.pc;//eip寄存器
		else{
			printf("Error: in TK_REG in p=%d, q=%d.\n",p,q);
			assert(0);
		}
		}
		else{
			printf("Error: in single token in p=%d, q=%d.\n",p,q);
			assert(0);
		}
	}
	else if (check_parentheses(p, q) == true) {//被括号包围
		/* The expression is surrounded by a matched pair of parentheses.
		 * If that is the case, just throw away the parentheses.
		 */
		return eval(p + 1, q - 1);//递归子表达式
	}
	else {
		int op = find_main_operator(p,q);//找主运算符
		vaddr_t addr;int result;//TK_DEREF中
		int op_type=tokens[op].type;
		//单目运算符
		switch(op_type){//按照不同的运算符进行不同的运算，并显示结果
			case TK_NEGATIVE:
				result=-eval(p+1,q);
				printf("Operator= -.\n");
				printf("Value=%d.\n",result);
				return result;
			case TK_DEREF://指针解引用
				addr=eval(p+1,q);
				result=vaddr_read(addr,4);
				printf("Addr=%u(0x%x), Value=%d(0x%08x).\n",addr,addr,result,result);
				return result;
			case '!':
				printf("Operator= !.\n");
				result=eval(p+1,q);
				if(result==0){
					printf("Value=1.\n");
					return 1;
				}
				else {
					printf("Value=0.\n");
					return 0;
				}
		}
		//双目运算符
		int val1 = eval(p, op - 1);//运算对象的值
		int val2 = eval(op + 1, q);
		printf("Val1=%d, Val2=%d.\n",val1,val2);
		
		switch (op_type) {
			case '+':
				printf("Operator= +.\n");
				printf("Value=%d.\n",val1+val2);
				return val1 + val2;
			case '-': 
				printf("Operator= -.\n");
				printf("Value=%d.\n",val1-val2);
				return val1-val2;
			case '*': 
				printf("Operator= *.\n");
				printf("Value=%d.\n",val1*val2);
				return val1*val2;
			case '/': 
				if(val2==0){
					printf("Error: The val2 can't be 0.\n");
					assert(0);
				}
				printf("Operator= /.\n");
				printf("Value=%d.\n",val1/val2);
				return val1/val2;
			case TK_EQ:
				printf("Operator= ==.\n");
				printf("Value=%d.\n",val1==val2);
				return val1==val2;
			case TK_NEQ:
				printf("Operator= !=.\n");
				printf("Value=%d.\n",val1!=val2);
				return val1!=val2;
			case TK_AND:
				printf("Operator= &&.\n");
				printf("Value=%d.\n",val1&&val2);
				return val1&&val2;
			case TK_OR:
				printf("Operator= ||.\n");
				printf("Value=%d.\n",val1||val2);
				return val1||val2;
			default: 
				printf("Error: Invalid operator.\n");
				assert(0);
		}
	}
}

bool check_parentheses(int p,int q){
	//printf("parent p为%d,q为 %d\n",p,q);
	STACK st;
	st=makeNull(&st);
	if(p>=q){
		printf("Error: p>=q in check_parentheses in p=%d, q=%d.\n",p,q);
		return false;
	}
	if(tokens[p].type!='('||tokens[q].type!=')')
		return false;//不是被括号包围
	for(int cur=p+1;cur<q;cur++){
		if(tokens[cur].type=='('){//遇到（加入栈
			push(&st,tokens[cur].type);
		}
		if(tokens[cur].type==')'){//遇到）弹出栈
			if(st.size>0)
				pop(&st);
			else{
				printf("Error: Missing '('.\n");
				return false;
			}
		}
	}
	if(st.size==0) return true;//匹配
	else return false;
}
int find_main_operator(int p,int q){
	int pos[5]={-1,-1,-1,-1,-1};
	STACK st;
	st=makeNull(&st);
	//printf("pos %d,,%d",pos[0],pos[1]);
	for(int i=p;i<=q;i++){
		if(tokens[i].type=='('){//遇到左括号压栈
			push(&st,tokens[i].type);
		}
		if(tokens[i].type==')'){//遇到）弹出栈
			pop(&st);
		}
		if(st.size>0)//栈不为空继续扫描
			continue;
		else if(tokens[i].type=='!'||tokens[i].type==TK_DEREF||tokens[i].type==TK_NEGATIVE)
			pos[4]=i;//优先级最高
		else if(tokens[i].type=='*'||tokens[i].type=='/')
			pos[3]=i;
		else if(tokens[i].type=='+'||tokens[i].type=='-')
			pos[2]=i;
		else if(tokens[i].type==TK_EQ||tokens[i].type==TK_NEQ)
			pos[1]=i;
		else if(tokens[i].type==TK_AND||tokens[i].type==TK_OR)
			pos[0]=i;//优先级最低
	}
	for(int i=0;i<5;i++){//找位置
		if(pos[i]!=-1) return pos[i];
	}
	printf("Error: in find_main_operator: p=%d,q=%d.\n",p,q);
	assert(0);
}
