#include "./subtractTime.h"
#include <stdio.h>

main ()
{
struct timeval first, second, result;


first.tv_sec = 3;
first.tv_usec = 2;

second.tv_sec = 2;
second.tv_usec =1;
subtractTime (&result, &first, &second);

printf ("The result, %d s.%d usec, must be equal to 1.1\n", result.tv_sec, result.tv_usec);

first.tv_sec = 3;
first.tv_usec = 2;

second.tv_sec = 2;
second.tv_usec =4;
subtractTime (&result, &first, &second); /* note that this is '3.000002 - 2.000004', so the result is 0.999998 */

printf ("The result, %d s.%d usec, must be equal to 0.999998\n", result.tv_sec, result.tv_usec);


first.tv_sec = 3;
first.tv_usec = 2;

second.tv_sec = 3;
second.tv_usec =2;
subtractTime (&result, &first, &second);

printf ("The result, %d s.%d usec, must be equal to 0.0\n", result.tv_sec, result.tv_usec);



}
