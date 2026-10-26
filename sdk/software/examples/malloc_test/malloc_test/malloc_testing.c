/***************************************************************************
* Project                   	: shakti devt board
* Name of the file	        : malloc_testing.c
* Brief Description of file     : Test for malloc implementation
* Name of Author    	        : Abhinav Ramnath
* Email ID                      : abhinavramnath13@gmail.com

 Copyright (C) 2019  IIT Madras. All rights reserved.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
***************************************************************************/
/**
 * @file malloc_testing.c
 * @brief Test code for malloc testing
 * @detail This file contains the tests to verify the working of malloc
 */

extern char _free_space[];
extern char _STACK_SIZE[];
extern char _HEAP_SIZE[];
extern char _stack_end[];
extern char _stack[];
extern char _heap[];
extern char _heap_end[];
extern char __bss_end[];
extern char __bss_start[];
extern char __sbss_end[];
extern char __sbss_start[];


char *test1=(char *)&_free_space;
char *test3=(char *)&_STACK_SIZE;
char *test4=(char *)&_HEAP_SIZE;
char *test5=(char *)&_stack_end;
char *test6=(char *)&_stack;
char *test7=(char *)&_heap;
char *test8=(char *)&_heap_end;
char *test9=(char *)&__bss_end;
char *test10=(char *)&__bss_start;
char *test11=(char *)&__sbss_end;
char *test12=(char *)&__sbss_start;

int c1,c2,c3,c4,c5;
int *pointer;

/** @fn int main()
 * @brief main runs the code to test malloc
 * @details runs the code to test malloc
 * @return returns int
 */
int main()
{

  printf("\n\n\n\n\n\nTest Running");
  printf("\n %x %x %x %x %x %x %x %x %x %x %x",test1,test3,test4,test5,test6,test7,test8,test9,test10,test11,test12);
  printf("\n %x %d %x %x %x %x %x",&pointer,c1,&c1,&c2,&c3,&c4,&c5);
  
  static int b1,b2,b3,b4,b5;
  printf("\n %x %x %x %x %x",&b1,&b2,&b3,&b4,&b5);

  int a1,a2,a3,a4,a5;
  printf("\n %x %x %x %x %x",&a1,&a2,&a3,&a4,&a5);

int **a;
int *b,*c,*d;
int i=0;int n;

//Enter Size of Array;
n= 25;
printf("\n size of array %d %x",n,&n);
b=malloc(n*sizeof(int))	;

//printf("\n\n\n\n\n\n\n %x %x %x %x %x %x %x  \n\n\n\n\n",test1,test3,test4,test5,test6,test7,test8);

printf("\n B's address %x %x",b,&b);
for(i=0;i<n;i++)
{
  b[i]=i+1;
}

for(i=0;i<n;i++)
{
    printf("\n b[%d]'s address is : %x",i,&b[i]);
    printf("\n b[%d]'s value is : %d",i,b[i]);
}

free(b);

printf("\nArray B is freed");
printf("\nb=%x",b);
printf("\n size of array %d",n);

c=malloc(n*sizeof(int))	;
printf("\n The previously allocated block from the free list is used to allocate Array C since they are of the same size");

printf("\n C's address %x",c);

for(i=0;i<n;i++)
{
    c[i]=i+1;
}

for(i=0;i<n;i++)
{
    printf("\n c[%d]'s address is : %x",i,&c[i]);
    printf("\n c[%d]'s value is : %d",i,c[i]);
}
free(c);

printf("\nc=%x",c);

int x;
x=30;

printf("\n\n\nx address is %x",&x);
d=malloc(x*sizeof(int));

printf("\n D's address %x",d);
for(i=0;i<x;i++)
{
    printf("\nSetting value %d , %x",i+1,&d[i]); 
    d[i]=i+1;
}

for(i=0;i<x;i++)
{
    printf("\n d[%d]'s address is : %x",i,&d[i]);
    printf("\n d[%d]'s value is : %d",i,d[i]);
}

free(d);

printf("\nd=%x",d);

}
