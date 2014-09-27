// show some different constructions

int add(int a, int b)
{
	return a + b;
}

int min(int a, int b)
{
	return a < b ? a : b;
}

int sum(int n, int * a)
{
	int i;
	int s = 0;
	for (i = 0 ; i < n ; i++)
		s += a[i];
	return s;
}

int my_strlen(char * c)
{
	int len = 0;
	while (*c++ != '\0')
		len++;
	return len;
}


struct list
{
	int (*function)(int arg);
	struct list * next;
};

struct list *
end_of_list(
	struct list * l
)
{
	while(l->next)
		l = l->next;
	return l;
}

int call_last_function(struct list * l, int arg)
{
	l = end_of_list(l);
	return l->function(arg);
}


int function_call(void)
{
	extern int args();
	return args(0,2,3,4,5,6,7,8,9,10);
}
