// TD: How to build project? Please create a Makfile

#include <stdio.h>
#include <stdlib.h>
#include <TXLib.h>

const int MULTIPLIER = 2;
const int START_SIZE = 20;
const int ERROR_CODE = 127;

/*
    TD: For next project better to replace "const char *"
        with typedef to easily change type of tree nodes. It can be
        applied to any data structure that you write.
*/

typedef struct TreeNode{
    const char * data = 0;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

/*
    TD: rename to smth like "create_blablalba"
    "operator new" is associated with operator overloading (C++ feature).
    It can cause confusion.
*/

TreeNode * operator_new(const char * data);
TreeNode * add_node(TreeNode * tree);

void tree_print(TreeNode * node_t);
int delete_tree_node(TreeNode * tree);

int analyse_answers(TreeNode * node_t);
int scan_answers();

const char ** scan_tree_file(char * text, const char ** pointers_to_nodes, const int symbols, int file_size);
TreeNode * build_tree(const char ** pointers_to_nodes, TreeNode * tree_node, int * index, int symbols);

char * my_scanf();
int get_file_size(FILE * fp);

int is_nil(const char * text, int index);
int find_bracket(const char * text, int index, int file_size);
int count_text_symbols(const char * text, int file_size);
void clean_buffer();

/*
    TD: Remove internals from main.
        Divide program into clear modules like:

            reading module:
                reads database and creates tree

            tree module:
                processes all operations with tree (add node, add child)

            akinator game module:
                is responsible for all modes 
                    (play mode,
                    definition mode,
                    comparison mode,
                    show database mode,
                    dump database,
                    exit)

        It's just example. Try to make clearer border between differen
        parts of your program. Remember, like in quadratic equation, solve module, output module...
*/
int main()
{
    // TD: hardcode filename. make it configurable from cmd, it's easy!
    FILE * fp_read_tree = fopen("tree_data.txt", "rb");
    if(fp_read_tree == NULL)
    {
        fprintf(stderr, "Can't open tree_data.txt file");
        exit(EXIT_FAILURE);
    }

    int file_size = get_file_size(fp_read_tree);

    char * text = (char*)calloc(file_size, sizeof(char));
    fread(text, sizeof(char), file_size, fp_read_tree);

    int symbols = count_text_symbols(text, file_size);

    /*
        TD: Rename. pointer_to_nodes in this context seems like
            pointer to TreeNode.

            Call thing that is contatined in this array somehow.

                like: token_ptr* token_array;

            Double pointers are terrifying. Please use typedef for
            easier reading. 
    */
    const char ** pointers_to_nodes = (const char**)calloc(symbols, sizeof(char));

    /*
        TD: May be put all this info about text file into structure?
            Like structure that associated with one database reading session.

            So many parameteres are kinda confusing.

            You will easier free all resources that are associated with reading session.
    */
    pointers_to_nodes = scan_tree_file(text, pointers_to_nodes, symbols, file_size);

    TreeNode * tree = (TreeNode*)calloc(1, sizeof(TreeNode));
    int index = 0;
    tree = build_tree(pointers_to_nodes, tree, &index, symbols);

    printf("Let's play\n");

    if(analyse_answers(tree))
    {
        printf("Wanna play again?\n");
    }

    char leave_symbol = 0;;
    while(true)
    {
        printf("Press \'q\' if you want to leave\n");
        scanf("%c", &leave_symbol);
        clean_buffer();

        if(leave_symbol == 'q')
        {
            return 0;
        }

        else
        {
            analyse_answers(tree);
        }
    }
    return 0;
}

TreeNode * operator_new(const char * data)
{
    TreeNode* new_node = (TreeNode*)calloc(sizeof(TreeNode), 1);

    if (new_node == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    new_node->data = strdup(data);
    new_node->right = NULL;
    new_node->left = NULL;

    return new_node;
}

TreeNode * add_node(TreeNode * tree)
{
    /*
        TD: One of akinator's tasks is to replace printf
            with some speaker function (txSpeak, eSpeak, festival...)

            So it's better to use more abstract function to output messages (like output("Who is this?"));

            Then when you'll need to go replace printf with speaker function,
            you'll just need to replace implementation of output() function, and to change nothing
            in the rest of code.  
    */
    printf("Who is this???\n");

    /*
        TD:
        1) copypaste of "calloc + scanf"
        2) check return value of calloc. please write wrapper of calloc that does it.
        3) why "START_SIZE * 2"?
    */

    const char* differ = (const char*)calloc(START_SIZE * 2, sizeof(char));
    const char* new_person = (const char*)calloc(START_SIZE * 2, sizeof(char));

    new_person = my_scanf();

    printf("How does %s differ from %s\n", tree->data, new_person);

    differ = my_scanf();

    tree->right = operator_new(tree->data);
    tree->data = differ;
    tree->left = operator_new(new_person);

    return tree;
}

int delete_tree_node(TreeNode * tree)
{
    if(tree == NULL)
    {
        free(tree);
        return 0;
    }

    tree->left = NULL;
    tree->right = NULL;
    tree->data = 0;

    free(tree);


    return 1;
}

void tree_print(TreeNode * node_t)
{
    if(node_t == NULL)
    {
        printf(" nil ");
        return;
    }

    if(node_t != NULL)
    {
        /*
            TD: Unconvinient to write every time this boilerplate code, like
                "txSetConsoleAttr(FOREGROUND_LIGHTGRAY | BACKGROUND_BLACK);"
            
                Make wrapper around printf that allows to set color of
                text and then automatically return standard color.

                Should look like:
                color_printf(RED, "How are you?");
        */
        txSetConsoleAttr(FOREGROUND_RED | BACKGROUND_BLACK);
        printf("(");
        txSetConsoleAttr(FOREGROUND_LIGHTGRAY | BACKGROUND_BLACK);

        txSetConsoleAttr(FOREGROUND_YELLOW | BACKGROUND_BLACK);
        printf("%s", node_t->data);
        txSetConsoleAttr(FOREGROUND_LIGHTGRAY | BACKGROUND_BLACK);

        tree_print(node_t->left);
        tree_print(node_t->right);

        txSetConsoleAttr(FOREGROUND_GREEN | BACKGROUND_BLACK);
        printf(")");
        txSetConsoleAttr(FOREGROUND_LIGHTGRAY | BACKGROUND_BLACK);
    }
}

int analyse_answers(TreeNode * node_t)
{
    if(node_t->right == NULL && node_t->left == NULL)
    {
        printf("Is this %s?\n", node_t->data);
        int check = scan_answers();

        if(check > 0)
        {
            printf("Yes, i know\n");
            return 1;
        }

        if(check == 0)
        {
            add_node(node_t);
        }

        return 1;
    }

    printf("%s? \n", node_t->data);

    int check = scan_answers();

    if(check == 1)
    {
        analyse_answers(node_t->left);
    }

    if(check == 0)
    {
        analyse_answers(node_t->right);
    }

    if(check == ERROR_CODE)
    {
        analyse_answers(node_t);
    }

    return 1;
}

int scan_answers()
{
    const char * str = (const char*)calloc(START_SIZE, sizeof(char));
    str = my_scanf();

    /*
        TD: length of "yes" can be compute in compile time using sizeof
    */
    if(strncmp(str, "yes", strlen("yes")) != 0 && strncmp(str, "no", strlen("no")) != 0)
    {
        printf("You can only answer yes or no \n");
    }

    // TD: do just one comparison (now it's one here and one in "if" above)
    if(strcmp(str, "yes") == 0)
    {
        return 1; // TD:: magic number
    }

    if(strcmp(str, "no") == 0)
    {
        return 0;
    }

    return ERROR_CODE;
}

const char ** scan_tree_file(char * text, const char ** pointers_to_nodes, const int symbols, int file_size)
{
    int pointers_index = 0;
    int bracket_index = 0;
    int find_nil_index = 0;

    for(int i = 0; i < file_size; i++)
    {
        int nil_index = 0;
        pointers_to_nodes[pointers_index] = (const char*)calloc(symbols, sizeof(char));

        nil_index = is_nil(text, i);

        if(nil_index > 0)
        {
            pointers_to_nodes[pointers_index] = &text[nil_index];

            text[nil_index + strlen("nil")] = '\0';
            pointers_index++;

            continue;
        }

        if(text[i] == '(')
        {
            pointers_to_nodes[pointers_index] = &text[i + 2];

            for(find_nil_index = i + 1; find_nil_index < file_size; find_nil_index++)
            {
                nil_index = is_nil(text, find_nil_index);
                if(nil_index > 0 || text[find_nil_index] == '(')
                {
                    break;
                }
            }

            if(nil_index > 0)
            {
                text[nil_index - 1] = '\0';
            }

            bracket_index = find_bracket(text, i, file_size);

            if(bracket_index == 1)
            {
                fprintf(stderr, "ERROR in find bracket(const char* text, int index, int file_size)");
            }

            text[bracket_index - 1] = '\0';

            pointers_index++;
        }
    }

    return pointers_to_nodes;
}

TreeNode * build_tree(const char ** pointers_to_nodes, TreeNode * tree_node, int * index, int symbols)
{
    if(strcmp(pointers_to_nodes[*index], "nil") == 0)
    {
        return NULL;
    }

    tree_node = operator_new(pointers_to_nodes[(*index)]);

    (*index)++;

    tree_node->left = build_tree(pointers_to_nodes, tree_node->left, index, symbols);
    (*index)++;
    tree_node->right = build_tree(pointers_to_nodes, tree_node->right, index, symbols);

    return tree_node;
}

char * my_scanf() // TD: why not standard getline()?
{
    int i = 0;
    int start_size = START_SIZE;
    char ch = 0;

    char * str = (char*)calloc(START_SIZE, sizeof(char));

    while((ch = (char)getchar()) != '\n')
    {
        if(i > start_size - 1)
        {
            start_size *= MULTIPLIER;
            str = (char*)realloc(str, start_size);
        }

        str[i] = ch;
        i++;
    }

    str[i] = '\0';

    //printf("%s \n", str);

    return str;
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


int find_bracket(const char * text, int index, int file_size)
{
    for(; index < file_size; index++)
    {
        if(text[index] == ')' || text[index] == '(')
        {
            //printf("%c%c%c", text[index - 3], text[index - 2], text[index - 1]);
            return index;
        }
    }

    return 0;
}

/*
    TD: why return index?
        it's more natural for so called function to return bool
*/
int is_nil(const char * text, int index)
{
    // TD: strncmp()?
    if(text[index] == 'n' && text[index + 1] == 'i' && text[index + 2] == 'l')
    {
       /*
       printf("1). %c\n"
               "2). %c\n"
               "3). %c\n", text[index], text[index+1], text[index+2]); */
        return index;
    }

    return 0;
}

void clean_buffer()
{
    while (getchar() != '\n')
    {
        continue;
    }
}


int get_file_size(FILE * fp)
{
    fseek(fp, 0L, SEEK_END);
    int file_size = ftell(fp);

    fseek(fp, 0L, SEEK_SET);
    return file_size;
}

