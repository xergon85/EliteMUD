CLASS_WARRIOR = 1;
CLASS_WIZARD = 2;
int error_proc(int a, int b, int c)
{
  CLASS_WIZARD = a + b + c;
}

do(ch, "' The value of error_proc = " + error_proc(1, 2, 3));
b = 42;
error_proc(1, 2, 3);
do(ch, "' The value in b + 4 = " + ((b + 2) * 3));

