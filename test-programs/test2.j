global int x;
global int y;

function myfunc(int a, string b, string s)
{
   call printf("hello world!\n");
   x = 42-7+12;
   call printf("x=%d\n",x);
}

program
{
   y = 42;
   call myfunc(y, "goodbye","third arg");
   y = y + 7;
   call printf("y=%d\n", y);
}