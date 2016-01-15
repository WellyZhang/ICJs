function f(a)
{
	if ((a == 0) or (a == 1))
		return 1
	else 
		return f(a - 1) + f(a - 2)
}
var i=0
var str="globalvar"
while (i < 11)
{
	var str = f(i)
	str
	i=i + 1
}
str