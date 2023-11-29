#include <stdio.h>
#include <stdlib.h>
#include <TXLib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

int find_bracket(const char * text, int index, int file_size)
{
    for(; index < file_size; index++)
    {
        if(text[index] == ')' || text[index] == '(')
        {
            return index;
        }
    }

    return 0;
}


char ** scan_expression(char * text, char ** pointers_to_nodes, int file_size, int symbols)
{
    int pointers_index = 0;
    int bracket_index = 0;
    int find_nil_index = 0;

    for(int i = 0; i < file_size; i++)
    {

        pointers_to_nodes[pointers_index] = (char*)calloc(symbols, sizeof(char));


        if(text[i] == '(')
        {
            pointers_to_nodes[pointers_index] = &text[i];

            bracket_index = find_bracket(text, i + 1, file_size);

            text[bracket_index + 1] = '\0';
            printf("IN BRACKET INDEX - %s\n", pointers_to_nodes[pointers_index]);

            i = bracket_index;

            pointers_index++;
        }


        if(text[i] == ' ' || text[i] == '\0')
        {
            if(text[i + 1] != '(')
            {
                pointers_to_nodes[pointers_index] = &text[i + 1];

                for(find_nil_index = i + 1; find_nil_index < file_size; find_nil_index++)
                {
                    if(text[find_nil_index] == ' ')
                    {
                        text[find_nil_index] = '\0';
                        break;
                    }
                }

                pointers_index++;
            }

            else
            {
                continue;
            }
        }
    }

    return pointers_to_nodes;
}


int get_file_size(FILE * fp)
{
    fseek(fp, 0L, SEEK_END);
    int file_size = ftell(fp);

    fseek(fp, 0L, SEEK_SET);
    return file_size;
}

int count_text_symbols(const char * text, int file_size)
{
    int counter = 0;

    for(int i = 0; i < file_size; i++)
    {
        if(isspace(text[i]))
        {
            counter++;
        }
    }

    return counter+1;
}

int main()
{
    FILE * fp_read_tree = fopen("my_expression.txt", "rb");
    if(fp_read_tree == NULL)
    {
        fprintf(stderr, "Can't open tree_data.txt file");
        exit(EXIT_FAILURE);
    }

    int file_size = get_file_size(fp_read_tree);

    char * text = (char*)calloc(file_size, sizeof(char));
    fread(text, sizeof(char), file_size, fp_read_tree);

    int symbols = count_text_symbols(text, file_size);
    char ** pointers_to_nodes = (char**)calloc(40, sizeof(char));
    pointers_to_nodes = scan_expression(text, pointers_to_nodes, file_size, symbols);

    for(int i = 0; i < 10; i++)
    {
        printf("%s\n", pointers_to_nodes[i]);
    }
}

