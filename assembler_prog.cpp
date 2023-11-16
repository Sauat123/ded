#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <TXLib.h>

struct TextParameters
{
    int amount_commands = 0;
    int amount_numbers = 0;
    int amount_marks = 0;
    int num_of_rows = 0;
    int longest_line = 0;
    int file_size = 0;
};

int convert_commands_from_file(FILE * fp);

int convert_commands(const char * commands_buf, int line_length, int * command_number,
                     const char ** marks, int * marks_int, int * num_of_mark);

int is_digit (const char * commands_buf, int line_length, int * current_number);
int char_array_to_int(const char* str);

int get_file_size(FILE * fp);
int get_command_length(const char * commands_buf);
int find_longest_line(const char * tmp_buf, int rows, int file_size, int longest_line, int index);

TextParameters * text_info_init(TextParameters * text_info, FILE * fp);

const int NUM_OF_COMMANDS = 20;

const char * valid_commands[NUM_OF_COMMANDS] = {"hlt", "push", "pop", "add", "sub", "mult", "div", "sqrt", "sin", "cos", "pow", "rax", "rbx", "rcx", "cmp", "je",
                                   "ja", "jae", "jb", "jbe"};


enum MATH_CODES {HLT = 0,
                 PUSH = 1,
                 POP = 2,
                 ADD, SUB, MULT, DIV,
                 SQRT, RAX = 11,
                 RBX, RCX, CMP, JE,
                 JA, JAE, JB, JBE};

int main()
{
    FILE* fp = fopen("commands.txt", "r");

    if (fp == NULL)
    {
        fprintf(stderr, "Failed to open file: commands.txt\n");
        exit(EXIT_FAILURE);
    }

    convert_commands_from_file(fp);

    fclose(fp);
    return 0;
}

int convert_commands_from_file(FILE * fp)
{
    struct TextParameters text_information;
    TextParameters* text_info = &text_information;
    text_info_init(text_info, fp);

    txSetConsoleAttr (FOREGROUND_LIGHTMAGENTA | BACKGROUND_BLACK);
    printf("\nMAIN PARAMETERS: \n"
    "amount_commands = %d\n"
    "amount_numbers = %d\n"
    "amount_marks = %d\n\n", text_info->amount_commands, text_info->amount_numbers, text_info->amount_marks);
    txSetConsoleAttr (FOREGROUND_LIGHTGRAY | BACKGROUND_BLACK);

    char * commands_buf[text_info->amount_commands];
    const char * marks[text_info->amount_marks];
    int * commands_int = (int*)calloc(text_info->amount_commands / sizeof(int), sizeof(int));
    int * numbers_int = (int*)calloc(text_info->amount_numbers, sizeof(int));
    int * marks_int = (int*)calloc(text_info->amount_marks, sizeof(int));

    if (commands_int == NULL || marks_int == NULL || numbers_int == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    int command_number = 0;
    int index = 0;
    int numbers_index = 0;
    int commands_index = 0;
    int num_of_mark = 0;

    FILE* commands_fp = fopen("commands.bin", "wb");
    FILE* numbers_fp = fopen("numbers.bin", "wb");
    if(commands_fp == NULL || numbers_fp == NULL)
    {
        fprintf(stderr, "Cannot find file for writing\n");
        exit(EXIT_FAILURE);
    }

    rewind(fp);

    for(int i = 0; i < text_info->amount_commands + text_info->amount_numbers; i++)
    {
        int current_number = 0;
        commands_buf[i] = (char*)calloc(text_info->longest_line, sizeof(char));
        marks[i] = (char*)calloc(text_info->amount_marks, sizeof(char));
        if (commands_buf[i] == NULL || marks[i] == NULL)
        {
            fprintf(stderr, "Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }

        fscanf(fp, "%s", commands_buf[i]);

        txSetConsoleAttr(FOREGROUND_YELLOW | BACKGROUND_BLACK);
        printf("\n \n-------------%s \n", commands_buf[i]);
        txSetConsoleAttr(FOREGROUND_LIGHTGRAY | BACKGROUND_BLACK);

        int line_length = strlen(commands_buf[i]);
        int is_digit_in_row = is_digit(commands_buf[i], line_length, &current_number);

        if(is_digit_in_row)
        {
            commands_int[commands_index] = command_number;
            numbers_int[numbers_index] = current_number;

            numbers_index++;
            printf("commands_buf[i-1] = %s \n", commands_buf[i-1]);

            continue;
        }

        int commands_length = get_command_length(commands_buf[i]);

        convert_commands(commands_buf[i], commands_length, &command_number, marks,
        marks_int, &num_of_mark);

        index++;

        commands_int[commands_index] = command_number;
        commands_index++;
    }

    fwrite(commands_int, sizeof(int), text_info->amount_commands, commands_fp);
    fwrite(numbers_int, sizeof(int), text_info->amount_numbers, numbers_fp);

    txSetConsoleAttr(FOREGROUND_YELLOW | BACKGROUND_BLACK);
    for(int pip = 0; pip < text_info->amount_commands; pip++)
    {
        printf("commands_int [%d] = %d \n", pip, commands_int[pip]);
    }
    txSetConsoleAttr(FOREGROUND_LIGHTGRAY | BACKGROUND_BLACK);
    printf("\n");

    txSetConsoleAttr(FOREGROUND_MAGENTA | BACKGROUND_BLACK);
    for(int j = 0; j < text_info->amount_numbers; j++)
    {
        printf("numbers_int[%d] = %d \n", j, numbers_int[j]);
    }
    txSetConsoleAttr(FOREGROUND_LIGHTGRAY | BACKGROUND_BLACK);
    printf("\n");

    txSetConsoleAttr(FOREGROUND_LIGHTMAGENTA | BACKGROUND_BLACK);
    for(int j = 0; j < text_info->amount_marks; j++)
    {
        printf("marks_int[%d] = %d \n", j, marks_int[j]);
    }
    txSetConsoleAttr(FOREGROUND_LIGHTGRAY | BACKGROUND_BLACK);
    printf("\n");

    return 1;
}

int convert_commands(const char * commands_buf, int line_length, int * command_number, const char ** marks, int * marks_int, int * num_of_mark)
{
    int i = 0;
    int command_length = get_command_length(commands_buf);


    for(; i < 20; i++)
    {
        if(strncmp(commands_buf, valid_commands[i], line_length) == 0)
        {
            *command_number = i;
            return 1;
        }
    }

    for(int j = 0; j <= *num_of_mark; j++)
    {
        if(strncmp(commands_buf, marks[j], command_length-1) == 0)
        {
            *command_number = marks_int[j] + 1;
            marks_int[j+1] = *command_number;
            return 0;
        }
    }

    marks[*num_of_mark] = (char*)calloc(strlen(commands_buf), sizeof(char));
    if (marks[*num_of_mark] == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    marks[*num_of_mark] = commands_buf;

    *command_number = *num_of_mark + i;
    marks_int[*num_of_mark] = *command_number;

    *num_of_mark = (*num_of_mark) + 3;

    return 0;
}

int get_file_size(FILE * fp)
{
    fseek(fp, 0L, SEEK_END);
    int file_size = ftell(fp);

    fseek(fp, 0L, SEEK_SET);
    return file_size;
}

int get_command_length(const char * commands_buf)
{
    int length = 0;
    int check = isalpha(commands_buf[0]);
    while(check)
    {
        length++;
        check = isalpha(commands_buf[length]);
    }
    return length;
}

int is_digit (const char * commands_buf, int line_length, int * current_number)
{
    for(int i = 0; i < line_length; i++)
    {

        if(isdigit(commands_buf[i]))
        {
            *current_number = atoi(commands_buf + i);
            return 1;
        }
    }

    if(*current_number > 0)
    {
        return 1;
    }

    return 0;
}

TextParameters * text_info_init(TextParameters * text_info, FILE * fp)
{
    text_info->file_size = get_file_size(fp);

    char * tmp_buf = (char*)calloc(text_info->file_size, sizeof(char));
    if (tmp_buf == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    fread(tmp_buf, sizeof(char), text_info->file_size, fp);

    for(int i = 0; i < text_info->file_size; i++)
    {
        if(isspace(tmp_buf[i]))
        {
            text_info->amount_commands++;
        }

        if(tmp_buf[i] == ':')
        {
            text_info->amount_marks = text_info->amount_marks + 2;
        }

        if(isdigit(tmp_buf[i]))
        {
            if(isdigit(tmp_buf[i-1]) == 0)
            {
                text_info->amount_numbers++;
            }
        }

        if (tmp_buf[i] == '\n')
        {
            text_info->num_of_rows++;
        }
    }

    text_info->longest_line = find_longest_line(tmp_buf, text_info->num_of_rows, text_info->file_size, 0, 0);

    free(tmp_buf);

    text_info->amount_commands = text_info->amount_commands + 1 - text_info->amount_numbers;

    return text_info;
}

int find_longest_line(const char * tmp_buf, int rows, int file_size, int longest_line, int index)
{
    int longest_line_counter = 0;

    for(; index < file_size; index++)
    {
        if(tmp_buf[index] == '\0' || tmp_buf[index] == '\n')
        {
            break;
        }
        longest_line_counter++;
    }

    if(longest_line_counter > longest_line)
    {
        longest_line = longest_line_counter;
    }

    if(index < file_size - 1)
    {
        int next_line_length = find_longest_line(tmp_buf, rows, file_size, 0, index+1);
        if(next_line_length > longest_line)
        {
            longest_line = next_line_length;
        }
    }

    return longest_line;
}
