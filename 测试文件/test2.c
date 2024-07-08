int add(int x,int y)
{
	return x+y;
}

int main()
{
	int a[100],b[100],c[100];
	for(int i=0;i<100;++i)
	{
		a[i]=i;
		b[i]=i;
		c[i]=add(a[i],b[i]);
	}
	return c[99];
}