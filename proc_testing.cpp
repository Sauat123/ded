#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <math.h>

#define STACK_DUMP(stack, filep) stack_dump((stack), (filep), #stack, __LINE__, __FILE__, __func__)

#define STACK_PUSH(stack, value, filep) \
    stack_push(stack, value);     \
    stack_dump((stack), (filep), #stack, __LINE__, __FILE__, __func__); \

#define STACK_POP(stack, filep) \
    stack_pop(stack);     \
    stack_dump((stack), (filep), #stack, __LINE__, __FILE__, __func__); \

typedef int elem_t;
typedef unsigned long long elem_cannary;
typedef unsigned long long hash_t;

const int STACK_SIZE = 10;
const int MULTIPLIER = 2;
const elem_cannary CANNARY_VALUE = 127;
const int POISON_VALUE = 0xBADEDA;
const int LEFT_CANNARY_PLACE = (sizeof(elem_cannary) - sizeof(elem_t)) / sizeof(elem_t);
const int NUM_OF_KEYS = 6;

#define NULL_STRUCT_POINTER (1 << 0)
#define NULL_ARRAY_POINTER (1 << 1)
#define TOP_LESS_THEN_SIZE (1 << 2)
#define STACK_SIZE_NEGATIVE (1 << 3)
#define STACK_TOP_NEGATIVE (1 << 4)

#define CHECK_ERROR(condition, error_flag) \
    if(condition) {                       \
        errors.errorcode |= error_flag;     \
    }

#define PRINT_MESSAGE(msg) fprintf(stderr, "(msg)");
/*
enum ERRORS {CLEAR = 0,
             NULL_STRUCT_POINTER = 2,  // 10100000  errors on bits
             NULL_ARRAY_POINTER = 4,
             TOP_LESS_THEN_SIZE = 8,
             STACK_SIZE_NEGATIVE = 16,
             STACK_TOP_NEGATIVE = 32,
             STACK_CHANGED = 64};
*/
struct stack_t
{
    int stack_size = 0;
    int top = sizeof(elem_cannary);
    elem_t * data = 0;
    hash_t stack_hash = 0;
};

struct Registers{
    int rax = 0;
    int rbx = 0;
    int rcx = 0;
};

struct program_error
{
    unsigned int errorcode: 3;
}errors;

void stack_dump(const stack_t * stk1, FILE* fp, const char * stack_name, int line, const char * file, const char * func);

int resize_stack (stack_t * stk1);

elem_t stack_pop (stack_t * stk1);
elem_t stack_push (stack_t * stk1, elem_t value);

int stack_error (const stack_t * stk1);
void stack_error_analyse(const stack_t *stk1);

int stack_dtor(stack_t * stk1);
int stack_ctor(stack_t * stk1);

unsigned long long hash_function (stack_t *stk1);
elem_t check_hash (stack_t *stk1);

///-----------------------------------------------------------------------------------------

int do_math_operation(int * commands, int * numbers, FILE * file_dump, stack_t * stk1, Registers * registers, int commands_file_size);

int sub_element(stack_t * stk1, FILE * dump_fp);
int add_element(stack_t * stk1, FILE * dump_fp);

int pop_element(stack_t * stk1, FILE * dump_fp, int * commands, Registers * registers, int * poped_value, int index);
int push_element(stack_t * stk1, int value, FILE * dump_fp, int * commands, Registers * registers, int index);

int div_element(stack_t * stk1, FILE * dump_fp);

int compare_values(int * commands, int condition, int * numbers, int reg_and_num, int only_reg, int * i, int * j, Registers * registers);
int find_compared_registers(int * commands, int i, int * condition, int * reg_and_num, int * only_reg);

int jump_to_mark(int * commands_int, int * index);

int get_file_size(FILE * fp);
void is_file_open(FILE * fp);

enum MATH_CODES {HLT = 0,
                 PUSH = 1,
                 POP = 2,
                 ADD, SUB, MULT, DIV,
                 SQRT, RAX = 11,
                 RBX, RCX, CMP, JE,
                 JA, JAE, JB, JBE};

enum CMP_CODES {CMP_NUM = 1,
                CMP_REG};

int main()
{
    FILE * dump_fp = fopen("stack_logs2.txt", "w");
    is_file_open(dump_fp);

    FILE * numbers_fp = fopen("numbers.bin", "rb");
    is_file_open(dump_fp);

    FILE * commands_fp = fopen("commands.bin", "rb");
    is_file_open(dump_fp);

    struct stack_t stk_main;
    struct stack_t * stk1 = &stk_main;

    struct Registers processor_registers;
    struct Registers * registers = &processor_registers;

    stack_ctor(stk1);
    stack_error_analyse(stk1);

    int commands_file_size = get_file_size(commands_fp);
    int numbers_file_size = get_file_size(numbers_fp);

    int * commands = (int*)calloc(commands_file_size, sizeof(int));
    int * numbers = (int*)calloc(numbers_file_size, sizeof(int));

    if(commands == NULL || numbers == NULL)
    {
        fprintf(stderr, "Memory allocation error");
    }

    fread(commands, sizeof(int), commands_file_size, commands_fp);
    fread(numbers, sizeof(int), numbers_file_size, numbers_fp);

    do_math_operation(commands, numbers, dump_fp, stk1, registers, commands_file_size);

    for(int i = sizeof(elem_cannary); i < commands_file_size / 4; i++)
    {
        printf("REZ = %d \n", stk1->data[i]);
    }
}

void is_file_open(FILE * fp)
{
    if(fp == NULL)
    {
        fprintf(stderr, "can't find this file");
        exit(1);
    }
}

int do_math_operation(int * commands, int * numbers, FILE * file_dump, stack_t * stk1, Registers * registers, int commands_file_size)
{
    int math_operation = 0;
    int poped_value = 0;
    int j = 0;

    int condition = 1;
    int reg_and_num = 0;
    int only_reg = 0;
    int cmp_ret_val = 0;

    for(int i = 0; i < commands_file_size / 4; i++)
    {

        math_operation = commands[i];

        if(math_operation >= RAX && math_operation <= RCX)
        {
            if(commands[i-1] == PUSH)
            {
                math_operation = commands[i-1];
            }

            if(commands[i] >= RAX && commands[i] <= RCX)
            {
                math_operation = commands[i-1];
            }
        }

        switch(math_operation)
        {
            case(PUSH):

                if(commands[i] >= RAX || commands[i+1] >= RAX)
                {
                    push_element(stk1, numbers[j], file_dump, commands, registers, i);
                    j++;
                }

                else if(commands[i+1] < RAX)
                {
                    push_element(stk1, numbers[j], file_dump, commands, registers, i);
                    j++;

                }

                else
                {
                    printf("PROBLEM COMMAND1  = %d \n"
                    "PROBLEM COMMAND2 = %d\n", commands[i], commands[i+1]);
                    printf("ERROR IN STACK PUSH CASE \n");
                }

                break;

            case(POP):

                if(commands[i] >= RAX)
                {
                    poped_value = pop_element(stk1, file_dump, commands, registers, &poped_value, i);
                }

                else if(commands[i] < RAX)
                {
                    pop_element(stk1, file_dump, commands, registers, &poped_value, i);
                }

                else
                {
                    printf("ERROR IN STACK POP CASE \n");
                }
                break;

            case(ADD):
                add_element(stk1, file_dump);
                break;

            case(SUB):
                sub_element(stk1, file_dump);
                break;

            case(DIV):
                div_element(stk1, file_dump);
                break;

            case(CMP):

                find_compared_registers(commands, i, &condition, &reg_and_num, &only_reg);
                cmp_ret_val = compare_values(commands, condition, numbers, reg_and_num, only_reg, &i, &j, registers);

                break;

            case(JE):

                if(cmp_ret_val == 0)
                {
                    jump_to_mark(commands, &i);
                }

                break;

            case(JA):

                if(cmp_ret_val == 1)
                {
                    jump_to_mark(commands, &i);
                }
                break;

            case(JAE):

                if(cmp_ret_val == 1 || cmp_ret_val == 0)
                {
                    jump_to_mark(commands, &i);
                }
                break;

            case(JB):

                if(cmp_ret_val == -1)
                {
                    jump_to_mark(commands, &i);
                }
                break;

            case(JBE):

                if(cmp_ret_val == -1 || cmp_ret_val == 0)
                {
                    jump_to_mark(commands, &i);
                }

                break;

            case(HLT):
                return 1;

            default:
                continue;
        }

    }
    return 1;
}

int push_element(stack_t * stk1, int value, FILE * dump_fp, int * commands, Registers * registers, int index)
{
    stack_error_analyse(stk1);

    if(commands[index + 1] >= RAX && commands[index + 1] <= RCX)
    {
        switch(commands[index+1])
        {
            case(RAX):
                STACK_PUSH(stk1, registers->rax, dump_fp);
                return registers->rax;
                break;

            case(RBX):
                STACK_PUSH(stk1, registers->rbx, dump_fp);
                return registers->rbx;
                break;

            case(RCX):
                STACK_PUSH(stk1, registers->rcx, dump_fp);
                return registers->rcx;
                break;

            default:
                printf("PROBLEM COMMAND = %d", commands[index + 1]);
                printf("IN PUSH ERROR");
        }
    }

    else if(commands[index] >= RAX && commands[index] <= RCX)
    {
        return 1;
    }

    else
    {
        STACK_PUSH(stk1, value, dump_fp);
        //printf("PUSHED VALUE = %d\n", value);
    }

    return value;
}

int pop_element(stack_t * stk1, FILE * dump_fp, int * commands, Registers * registers, int * poped_value, int index)
{
    stack_error_analyse(stk1);

    if(commands[index + 1] >= RAX)
    {
        switch(commands[index + 1])
        {
            case(RAX):
                registers->rax = STACK_POP(stk1, dump_fp);
                return registers->rax;
                break;

            case(RBX):
                registers->rbx = STACK_POP(stk1, dump_fp);
                return registers->rbx;
                break;

            case(RCX):
                registers->rcx = STACK_POP(stk1, dump_fp);
                return registers->rcx;
                break;

            default:
                printf("IN POP");
                exit(1);
        }
    }

    else if(commands[index] >= RAX)
    {
        return registers->rax;
    }

    else
    {
        STACK_POP(stk1, dump_fp);
    }

    return (*poped_value);
}

int add_element(stack_t * stk1, FILE * dump_fp)
{
    stack_error_analyse(stk1);
    hash_function(stk1);

    int summand1 = STACK_POP(stk1, dump_fp);
    int summand2 = STACK_POP(stk1, dump_fp);
    /*
    printf("summand1 = %d \n", summand1);
    printf("summand2 = %d \n", summand2);
    printf("SUM = %d \n", summand1+summand2);
    */
    STACK_PUSH(stk1, summand1 + summand2, dump_fp);

    return 1;
}

int sub_element(stack_t * stk1, FILE * dump_fp)
{
    stack_error_analyse(stk1);
    hash_function(stk1);

    int deductible = STACK_POP(stk1, dump_fp);
    int reduced = STACK_POP(stk1, dump_fp);
/*
    printf("deductible = %d \n", deductible);
    printf("reduced = %d \n", reduced);
    printf("DIFF = %d \n", reduced - deductible);
  */
    STACK_PUSH(stk1, reduced - deductible, dump_fp);

    return 1;
}

int div_element(stack_t * stk1, FILE * dump_fp)
{
    stack_error_analyse(stk1);

    int divisible = STACK_POP(stk1, dump_fp);
    int devider = STACK_POP(stk1, dump_fp);

    STACK_PUSH(stk1, divisible / devider, dump_fp);

    return 1;
}

int get_file_size(FILE * fp)
{
    fseek(fp, 0L, SEEK_END);
    int file_size = ftell(fp);

    fseek(fp, 0L, SEEK_SET);
    return file_size;
}

int find_compared_registers(int * commands, int i, int * condition, int * reg_and_num, int * only_reg)
{
    int number_of_registers = 0;
    if(commands[i+1] <= RCX && commands[i+1] >= RAX)
    {
        *condition = 2;

        number_of_registers++;
    }

    if(commands[i+2] <= RCX && commands[i+2] >= RAX)
    {
        *condition = 2;

        number_of_registers++;
    }

    if(number_of_registers == 0)
    {
        return 0;
    }

    if(number_of_registers == 2)
    {
        *only_reg = 1;
    }

    else
    {
        *reg_and_num = 1;
    }

    return 0;
}

int compare_values(int * commands, int condition, int * numbers, int reg_and_num, int only_reg, int * i, int * j, Registers * registers)
{

    switch(condition){

        case(CMP_NUM):

            if(numbers[*j-2] > numbers[*j-1])
            {
                return 1;
            }

            else if(numbers[*j - 2] == numbers[*j - 1])
            {
                printf("COMPARED NUMBERS = %d || %d \n", numbers[*j-2], numbers[*j-1]);
                return 0;
            }

            else
            {
                printf("COMPARED NUMBERS = %d || %d \n", numbers[*j-2], numbers[*j-1]);
                return -1;
            }

            break;

        case(CMP_REG):
            switch(commands[*i + 1])
            {
                case(RAX):
                    if(reg_and_num)
                    {
                        if(registers->rax > numbers[*j])
                            (*j)++;
                            (*i)++;
                            return 1;
                        if(registers->rax == numbers[*j])
                            (*j)++;
                            (*i)++;
                            return 0;
                    }

                    if(only_reg)
                    {
                        if(commands[*i + 2] == RBX)
                        {
                            if(registers->rax > registers->rbx)
                                return 1;

                            else if(registers->rax == registers->rbx)
                                return 0;

                            else
                                return -1;
                        }

                        if(commands[*i + 2] == RCX)
                        {
                            if(registers->rax < registers->rcx)
                                return 1;

                            else if(registers->rax == registers->rcx)
                                return 0;

                            else
                                return -1;
                        }
                    }
                    break;

                case(RBX):
                    if(reg_and_num)
                    {
                        if(registers->rbx > numbers[*j])
                        {
                            (*j)++;
                            return 1;
                        }

                        else if(registers->rbx == numbers[*j])
                        {
                            (*j)++;
                            return 0;
                        }

                        else
                        {
                            (*j)++;
                            return -1;
                        }
                    }

                    if(only_reg)
                    {
                        if(commands[*i + 2] == RAX)
                        {
                            if(registers->rbx > registers->rax)
                                return 1;
                            if(registers->rbx == registers->rax)
                                return 0;
                        }

                        if(commands[*i + 2] == RCX)
                        {
                            if(registers->rbx < registers->rcx)
                                return 1;

                            else if(registers->rbx == registers->rcx)
                                return 0;

                            else
                                return -1;
                        }
                    }
                    break;

                case(RCX):
                    if(reg_and_num)
                    {
                        if(registers->rcx > numbers[*j])
                        {
                            (*j)++;
                            return 1;
                        }

                        else if(registers->rcx == numbers[*j])
                        {
                            (*j)++;
                            return 0;
                        }

                        else
                        {
                            (*j)++;
                            return -1;
                        }
                    }

                    if(only_reg)
                    {
                        if(commands[*i + 2] == RAX)
                        {
                            if(registers->rcx > registers->rax)
                                return 1;

                            else if(registers->rcx == registers->rbx)
                                return 0;

                            else
                                return -1;
                        }

                        if(commands[*i + 2] == RBX)
                        {
                            if(registers->rcx < registers->rbx)
                                return 1;
                            else if(registers->rcx == registers->rbx)
                                return 0;
                            else
                                return -1;
                        }
                    }

                default:
                fprintf(stderr, "ERROR in register case CMP \n");
                return 228;

                break;
        }

        default:
            fprintf(stderr, "ERROR in CMP");
            return 228;
    }
}

int jump_to_mark(int * commands_int, int * index)
{
    for(int i = 0; i < 17; i++)        /// num_of_commands
    {
        if(commands_int[(*index) + 1] == commands_int[i] - 1)
        {
            *index = i;
            return 1;
        }
    }
    return 0;
}

///-----------------------------------------------------------------------------------------

int stack_ctor(stack_t * stk1)
{
    if(stk1->data == NULL && stk1->stack_size == 0 && stk1->top == sizeof(elem_cannary))
    {
        int cannary_memory = sizeof(elem_cannary)*2;

        stk1->data = (elem_t*)calloc(STACK_SIZE + cannary_memory, sizeof(elem_t));

        stack_error_analyse(stk1); //not exit in this situation

        stk1->stack_size = STACK_SIZE;
        //int cannary_left = 0;

        stk1->stack_hash = hash_function(stk1);
        //int cannary_right = stk1->stack_size; // пересчитать канкрейку  4 bytes to end 4 bytes to cannary right

        //cannary_init((elem_cannary*)&stk1->data[cannary_left], (elem_cannary*)&stk1->data[cannary_right]);
        return 1;
    }
    else
        return 0;
}

int stack_dtor(struct stack_t * stk1)
{                                       //check for normal stack
    if(stk1->data == NULL)
    {
        return 0;
    }

    for(int i = 0; i < stk1->stack_size; i++)
    {
        stk1->data[i] = POISON_VALUE;
    }

    free(stk1->data);  // double destruction
    stk1->stack_size = 0;
    stk1->top = sizeof(elem_cannary);

    return 1;

}

void stack_error_analyse(const stack_t *stk1)
{
    if(stk1 == NULL)
    {
        fprintf(stderr, "structure pointer is null\n");
        exit(1);
    }

    CHECK_ERROR(stk1->data == NULL, (int)NULL_ARRAY_POINTER);

    //CHECK_ERROR(stk1->top - elem_cannary_size < 0, (int)TOP_LESS_THEN_SIZE);
    CHECK_ERROR(stk1->stack_size < 0, (int)STACK_SIZE_NEGATIVE);
    CHECK_ERROR(stk1->top < 0, (int)STACK_TOP_NEGATIVE);

    if (errors.errorcode != 0)
    {
        if (errors.errorcode & NULL_ARRAY_POINTER)
        {
            PRINT_MESSAGE(array pointer is null);
        }

        if (errors.errorcode & TOP_LESS_THEN_SIZE)
        {
            PRINT_MESSAGE(stacks memory is full: stk->top > stk->size);
        }
                                                                                //use define?
        if (errors.errorcode & STACK_SIZE_NEGATIVE)
        {
            PRINT_MESSAGE(stack size is negative: stk1->stack_size < 0);
        }

        if (errors.errorcode & STACK_TOP_NEGATIVE)
        {
            PRINT_MESSAGE(stack top is negative: stk1->top < 0);  //string in print_message
        }
        printf("ERROR CODE = %d", errors.errorcode);
        //exit(EXIT_FAILURE);
    }
}

elem_t stack_push (stack_t * stk1, elem_t value)
{
    stack_error_analyse(stk1);

    /*int cannary_left = 0;
    int cannary_right = stk1->stack_size;
     */
    check_hash(stk1);                                //check hash to error_functions
    //check_cannary(stk1->data[cannary_left], stk1->data[cannary_right]);
    //printf("stack size: %d \n", stk1->stack_size);

    if(stk1->stack_size - 2 <= stk1->top)
    {
        //printf("RESIZE REALIZED++ \n");
        resize_stack(stk1);
    }

    stk1->data[stk1->top] = value;
    stk1->top++;

    stk1->stack_hash = hash_function(stk1);
    //printf("stk hash = %u \n", stk1->stack_hash);

    return stk1->data[stk1->top-1];
}

elem_t stack_pop (stack_t * stk1)
{
    stack_error_analyse(stk1);

    /*int cannary_left = 0;
    int cannary_right = stk1->stack_size;
     */
    check_hash(stk1);
    //check_cannary(stk1->data[cannary_left], stk1->data[cannary_right])
    if((stk1->top * 2 < stk1->stack_size ))
    {
        //printf("RESIZE REALIZED -- \n");
        resize_stack(stk1);
    }
    //printf("stack size: %d \n", stk1->stack_size);
    //printf("stk hash = %u \n", stk1->stack_hash);

    stk1->top--;

    int poped_value = stk1->data[stk1->top];
    stk1->data[stk1->top] = 0;

    stk1->stack_hash = hash_function(stk1);

    return poped_value;
}

int resize_stack (stack_t * stk1)
{
    stack_error_analyse(stk1);

    //int cannary_left = 0;

    if(stk1->stack_size - 2 <= stk1->top)
    {
        stk1->stack_size *= MULTIPLIER;

        stk1->data = (elem_t*)realloc(stk1->data, stk1->stack_size * sizeof(elem_t) + 2*sizeof(elem_cannary));
        //printf("RELOCATED memory = %d", stk1->stack_size * sizeof(elem_t) + 2*sizeof(elem_cannary));

        //printf("stack size = %d ", stk1->stack_size);
        //printf("CANNARY POINTER in incrementing = %d\n", cannary_right);

        if(stk1->data == NULL)
        {
            fprintf(stderr, "stk1->data = NULL \n");
            exit(1);
        }

        memset(stk1->data + stk1->top, 0, (stk1->stack_size - stk1->top - 1) * sizeof(elem_t));

    }

    else if(stk1->top * 2 < stk1->stack_size)
    {
        stk1->stack_size /= MULTIPLIER;

        //printf("stack size = %d ", stk1->stack_size);

        stk1->data = (elem_t*)realloc(stk1->data, (stk1->stack_size * sizeof(elem_t) + 2*sizeof(elem_cannary)));

        if(stk1->data == NULL)
        {
            fprintf(stderr, "stk1->data = NULL \n");
            exit(1);
        }

        memset(stk1->data + (stk1->top+1), 0, stk1->stack_size);
    }

    return 1;
}

void stack_dump(const stack_t * stk1, FILE * fp, const char * stack_name, int line, const char * file, const char * func)
{
    fprintf(fp, " \n \nStack [%p] name: %s \n"
    "Function called on line %d in %s() \n "
    "In file %s \n", &stk1, stack_name, line, func, file);  //100% call, if stk == 0, don't use stk...


    for (size_t i = 7; i < stk1->stack_size + sizeof(elem_cannary); i++)
    {
        fprintf(fp, "\n stk->data[%d]: %d \n", i, stk1->data[i]);
    }

    /*printf("fprintf value: %d  ", a);
    printf("stk->data[0] = %d", stk->data[0]);*/

    stack_error_analyse(stk1);
}

unsigned long long hash_function (stack_t *stk1)
{
    unsigned long long result = 0;

    for(int i = 0; i < stk1->stack_size; i++)
    {
        result += (((stk1->data[i])*37 + 4)*3);
    }

    //printf("\n hash_function result = %u \n", result);

    return result;
}

elem_t check_hash (stack_t *stk1)
{
    unsigned long long calculated_hash = hash_function(stk1);

    /*printf("     \n stk1-stack_hash = %d \n", stk1->stack_hash);
    printf("     \n IF old hash = %d \n", calculated_hash);
    */
    if(calculated_hash != stk1->stack_hash)
    {
        fprintf(stderr, "\n STACK CHANGED \n");
        exit(1);
    }

    //printf("calculated hash = %u   stk1->stack_hash = %u \n", calculated_hash, stk1->stack_hash);
    return 1;

}
