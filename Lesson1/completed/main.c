#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_ROWS 1000
#define MAX_COLS 1000
#define MAX_WORDS 1000
#define MAX_NUMS 1000
#define MAX_WORD_LENGTH 50

char *VAN_BAN_PATH = "vanban.txt";
char *STOPW_PATH = "stopw.txt";

char *bannedWords[MAX_WORDS];
char *words[MAX_WORDS];
int rowIndices[MAX_WORDS][MAX_ROWS];
int colIndices[MAX_WORDS][MAX_COLS];
int types[MAX_NUMS];

int compareLowerString(char *s1, char *s2)
{
    char *us1 = (char *)s1,
         *us2 = (char *)s2;

    while (tolower(*us1) == tolower(*us2++))
        if (*us1++ == '\0')
            return (0);
    return (tolower(*us1) - tolower(*--us2));
}

int isAlphabetWord(char *word)
{
    for (int i = 0; i < strlen(word); i++)
    {
        if (!isalpha(word[i]))
        {
            return 0;
        }
    }
    return 1;
}

int isInteger(char *input)
{
    for (int i = 0; i < strlen(input); i++)
    {
        if (!isdigit(input[i]))
        {
            return 0;
        }
    }
    return 1;
}

int isDouble(char *input)
{
    int num_digits = 0;
    for (int i = 0; i < strlen(input); i++)
    {
        if (i == 0)
        {
            if (input[i] == '-' || input[i] == '+')
                continue;
        }
        if (input[i] == '.')
        {
            if (num_digits == 0)
                return 0;
        }
        else
        {
            if (isdigit(input[i]))
            {
                num_digits++;
            }
            else
                return 0;
        }
    } /* end for loop */
    if (num_digits == 0)
        return 0;
    else
        return 1;
}

void cleanLine(char *str)
{
    int len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        if (!isalnum(str[i]) && str[i] != '.' && str[i] != ',' && str[i] != '-' && str[i] != ' ')
        {
            for (int j = i; j < len; j++)
            {
                str[j] = str[j + 1];
            }
            len--;
            i--;
        }
    }
    str[len] = ' ';
}

void cleanWord(char *str)
{
    if (!isInteger(str) && !isDouble(str))
    {
        for (int i = 0; i < strlen(str); i++)
        {
            if (!isalnum(str[i]) && str[i] != '-')
            {
                if (str[i] == '.')
                {
                    if (isupper(str[i + 1]))
                        continue;
                    else if (str[i + 1] == '\0')
                        str[i] = '\0';
                }
                str[i] = '\0';
                break;
            }
        }
    }
}

int isLastWord(char *word)
{
    return word[strlen(word) - 1] == '.' ? 1 : 0;
}

int isProperNoun(char *word, char *prevWord)
{
    if (strlen(prevWord) != 0 && isupper(word[0]) && !isLastWord(prevWord))
    {
        return 1;
    }
    return 0;
}

int isBanned(char word[])
{
    for (int i = 0; i < MAX_WORDS && bannedWords[i] != NULL; i++)
    {
        if (compareLowerString(word, bannedWords[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void appendWordPosition(int wordIndex, int rowIndex, int colIndex, int typeIndex)
{
    for (int i = 0; i < MAX_ROWS; i++)
    {
        if (rowIndices[wordIndex][i] == 0)
        {
            rowIndices[wordIndex][i] = rowIndex;
            colIndices[wordIndex][i] = colIndex;
            types[wordIndex] = typeIndex;
            return;
        }
    }
}

void appendWords(char word[], int rowIndex, int colIndex, int typeIndex)
{
    for (int i = 0; i < MAX_WORDS; i++)
    {
        if (words[i] == NULL)
        {
            words[i] = malloc(strlen(word) + 1);
            strcpy(words[i], word);
            appendWordPosition(i, rowIndex, colIndex, typeIndex);
            return;
        }
        else if (strcmp(words[i], word) == 0)
        {
            appendWordPosition(i, rowIndex, colIndex, typeIndex);
            return;
        }
    }
}

int numberOfOccurences(int wordIndex)
{
    int count = 0;
    for (int i = 0; i < MAX_ROWS; i++)
    {
        if (rowIndices[wordIndex][i] != 0)
        {
            count += 1;
        }
        else
        {
            break;
        }
    }
    return count;
}

void readFileStopWord(char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        printf("Cannot open file %s\n", fileName);
        return;
    }

    int wordIndex = 0;
    char word[MAX_WORD_LENGTH];
    while (fscanf(fp, "%s", word) != EOF)
    {
        bannedWords[wordIndex] = malloc(strlen(word) * sizeof(char));
        strcpy(bannedWords[wordIndex], word);
        wordIndex += 1;
    }
    fclose(fp);
}

void buildTable()
{
    readFileStopWord(STOPW_PATH);
    FILE *fp = fopen(VAN_BAN_PATH, "r");
    if (fp == NULL)
    {
        printf("Cannot open file %s", VAN_BAN_PATH);
        return;
    }
    int rowIndex = 1;
    char line[MAX_COLS];
    while (fgets(line, MAX_COLS, fp) != NULL)
    {
        cleanLine(line);
        char prevWord[MAX_WORD_LENGTH];
        prevWord[0] = '\0';
        char word[MAX_WORD_LENGTH];
        int typeIndex = 0;
        int colIndex = 1;
        int k = 0;
        for (int i = 0; i < strlen(line); i++)
        {
            if (line[i] == ' ' || line[i] == '\0' || line[i] == '\n' || line[i] == EOF)
            {
                word[k] = '\0';
                char cleanedWord[MAX_WORD_LENGTH];
                strcpy(cleanedWord, word);
                cleanWord(cleanedWord);
                if (strlen(cleanedWord) != 0 && !isBanned(cleanedWord))
                {
                    if (isProperNoun(cleanedWord, prevWord))
                        typeIndex = 1;
                    else if (isInteger(cleanedWord))
                        typeIndex = 2;
                    else if (isDouble(cleanedWord))
                        typeIndex = 3;
                    else if (isAlphabetWord(cleanedWord))
                        typeIndex = 0;
                    appendWords(cleanedWord, rowIndex, colIndex, typeIndex);
                }
                colIndex += 1;
                k = 0;
                strcpy(prevWord, word);
            }
            else
            {
                word[k] = line[i];
                k += 1;
            }
        }
        rowIndex++;
    }
    fclose(fp);
}

void swap(int i, int j)
{
    char *temp = words[i];
    words[i] = words[j];
    words[j] = temp;

    int tmp = types[i];
    types[i] = types[j];
    types[j] = tmp;

    for (int k = 0; k < MAX_ROWS; k++)
    {
        int temp = rowIndices[i][k];
        rowIndices[i][k] = rowIndices[j][k];
        rowIndices[j][k] = temp;
    }

    for (int k = 0; k < MAX_COLS; k++)
    {
        int temp = colIndices[i][k];
        colIndices[i][k] = colIndices[j][k];
        colIndices[j][k] = temp;
    }
}

void sortTable()
{
    for (int i = 0; i < MAX_WORDS && words[i] != NULL; i++)
    {
        for (int j = i + 1; j < MAX_WORDS && words[j] != NULL; j++)
        {
            if (strcmp(words[j], words[i]) < 0)
            {
                swap(i, j);
            }
        }
    }
}

void printTable()
{
    printf("0: normal\n1: proper noun\n2: integer\n3: real number\n\n");
    for (int i = 0; i < MAX_WORDS; i++)
    {
        if (words[i] == NULL)
        {
            break;
        }
        else
        {
            printf("%-15s Appear: %d Type: %d Positions:", words[i], numberOfOccurences(i), types[i]);
            for (int j = 0; j < MAX_ROWS; j++)
            {
                if (rowIndices[i][j] == 0)
                {
                    break;
                }
                printf(" (%d, %d)", rowIndices[i][j], colIndices[i][j]);
            }
            printf("\n");
        }
    }
}

int main()
{
    buildTable();
    sortTable();
    printTable();
    return 0;
}