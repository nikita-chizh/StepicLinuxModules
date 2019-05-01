#ifndef HELLO3_H
#define HELLO3_H

int array_sum(short *arr, size_t n);

ssize_t generate_output(int sum, short *arr, size_t size, char *buf);

void foo();

#define CHECKER_MACRO() foo()

#endif // HELLO3_H
