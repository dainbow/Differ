#include "Latex.h"

char* MakeLatexTitle(Tree* tree, char const* name) {
    assert(name != nullptr);

    static char endName[TEX_MAX_NAME_LENGTH] = "";
    GenerateOutputName(name, endName, TEX_PATH, TEX_OUTPUT_FORMAT);

    FILE* output = fopen(endName, "w");
    assert(output != nullptr);

    fprintf(output, "\\documentclass[12pt, a4paper]{article}\n"
                    "\\usepackage[utf8]{inputenc}\n"
                    "\\usepackage[russian]{babel}\n"
                    "\\usepackage[T2A]{fontenc}\n"
                    "\\usepackage{amssymb}\n"
                    "\\usepackage{amsfonts}\n"
                    "\\usepackage{amsmath}\n\n");

    fprintf(output, "\\author{Великий дифференциатор, доцент математических наук}\n"
                    "\\title{Работа всей моей жизни, великое дифференцирование}\n\n");
    
    fprintf(output, "\\begin{document}\n" 
                    "\\maketitle\n");

    fprintf(output, "Приступаю к дифференцированию этого наилегчайшего выражения.\n\n");
    fprintf(output, "$");
    PrintTexTree(tree->root, output);
    fprintf(output, "$\n\n");
    
    fclose(output);

    return endName;
}

#define PARENTHESIS_IF(branch, action)                                                      \
    if ((node->type       == TYPE_OP)       &&                                              \
        (node-> branch ->type == TYPE_OP)   &&                                              \
        (node->data != 'l')                 &&                                              \
        (node-> branch ->data != 'l')) {                                                    \
        if (CompareOperations((int8_t)node->data, (int8_t)node-> branch ->data) >= 0) {     \
            action;                                                                         \
        }                                                                                   \
    }   

void PrintTexTree(Node* node, FILE* output) {
    assert(node  != nullptr);
    assert(output != nullptr);

    switch(node->type) {
        case TYPE_OP:
            switch(node->data) {
                case '/':
                    fprintf(output, "\\frac{");
                    break;
                case 'l':
                   fprintf(output, "\\log_{");
                   break;
                default:
                    break; 
            }
            break;
        default:
            break;
    }

    if (node->left != nullptr) {
        PARENTHESIS_IF(left, fprintf(output, "("));
        PrintTexTree(node->left, output);
        PARENTHESIS_IF(left, fprintf(output, ")"));
    }

    switch (node->type) {
        case TYPE_OP:
            switch (node->data) {
                case '/':
                    fprintf(output, "}{");
                    break;
                case 'l':
                    fprintf(output, "}(");
                    break;
                case '^':
                    fprintf(output, "^{");
                    break;
                default:
                    fprintf(output, "%c", node->data);
                    break;
            }
            break;
        case TYPE_UNO:
            switch (node->data) {
                case 's':
                    fprintf(output, "\\sin(");
                    break;
                case 'c':
                    fprintf(output, "\\cos(");
                    break;
                default:
                    break;
            }
            break;
        case TYPE_CONST:
            fprintf(output, "%d", node->data);
            break;
        case TYPE_VAR:
            fprintf(output, "%c", node->data);
            break;
        default:
            break;
    }
    

    if (node->right != nullptr) {
        PARENTHESIS_IF(right, fprintf(output, "("));
        PrintTexTree(node->right, output);
        PARENTHESIS_IF(right, fprintf(output, ")"));
    }

    switch (node->type) {
        case TYPE_OP:
            switch (node->data) {
                case '/':
                    fprintf(output, "}");
                    break;    
                case 'l':
                    fprintf(output, ")");
                    break;
                case '^':
                    fprintf(output, "}");
                    break;
                default:
                    break;
            }
            break;
        case TYPE_UNO:
            fprintf(output, ")");
            break;
        default:
            break;
    }
}

int32_t CompareOperations(int8_t firstOper, int8_t secondOper) {
    int32_t firstValue  = GiveOperationPriority(firstOper);
    int32_t secondValue = GiveOperationPriority(secondOper);

    if (firstValue > secondValue)
        return 1;
    else if (firstValue == secondValue)
        return 0;
    else
        return -1;
}

int32_t GiveOperationPriority(int8_t operation) {
    switch (operation) {
    case '+':
    case '-':
        return 1;
    case '*':
    case '/':
        return 2;
    case '^':
        return 3;
    default:
        assert(0 && "INVALID OPERATOR");
        break;
    }

    return 0;
}
