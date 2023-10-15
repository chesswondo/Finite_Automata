#define  _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define  NON  ((unsigned int)-1)

typedef struct DFA_
{
    unsigned int ** t;  // Таблиця переходів
    unsigned int *  f;  // Таблиця кінцевих станів
    unsigned int   st;  // Кількість станів
    unsigned int   al;  // Довжина алфавіта
    unsigned int   s0;  // Початковий стан
    unsigned int   cs;  // Поточний стан
} DFA;

// Виділення необхідної пам'яті
DFA * makeFA(unsigned int alphabetLength, unsigned int stateCount, unsigned int s0)
{
    DFA * fa = malloc(sizeof(DFA));                  if (!fa)    return NULL; // Для автомата
    fa->st  = stateCount;
    fa->al  = alphabetLength;
    fa->f   = calloc(fa->st,sizeof(unsigned int));   if (!fa->f) return NULL; // Для таблиці f
    fa->t   = calloc(fa->st,sizeof(unsigned int *)); if (!fa->t) return NULL; // Для таблиці t (перша розмірність)
    for(unsigned int i = 0; i < fa->st; ++i)                                  // Для таблиці t (друга розмірність)
    {
        if (NULL == (fa->t[i] = calloc(fa->al,sizeof(unsigned int)))) return NULL;
        for(unsigned int j = 0; j < fa->al; ++j) fa->t[i][j] = NON;
    }
    // Ініціалізація станів
    fa->s0 = fa->cs = s0;
    return fa;
}

// Звільнення пам'яті автомата
void killFA(DFA* fa)
{
    // Звільняємо пам'ять в зворотному порядку
    free(fa->f);
    for(unsigned int i = 0; i < fa->st; ++i) free(fa->t[i]);
    free(fa->t);
    free(fa);
}


int addRule(DFA*fa, unsigned int s, char c, unsigned int sp)
{
    // Додаємо правило в таблицю переходів

    // Перевірки на коректність даних (вихід за межі)
    if (s  >= fa->st) return 0;
    if (sp >= fa->st) return 0;
    if ((unsigned int)(c-'a') >= fa->al) return 0;

    // Якщо таке правило вже є - це полилка (автомат детермінований)
    if (fa->t[s][c-'a'] != NON) return 0;

    fa->t[s][c-'a'] = sp;
    return 1;
}


int addFinite(DFA*fa, unsigned int fs)
{
    // Додаємо позначки кінцевих станів
    // Перевірка на коректність даних (вихід за межі)
    if (fs >= fa->st) return 0;
    fa->f[fs] = 1;
    return 1;
}

// Побудова DFA за даними з файлу
DFA * readDFA(FILE* in)
{
    unsigned int A, S, s0, F, f;
    // З перевірками коректності зчитуваних даних
    if (fscanf(in,"%u %u %u",&A,&S,&s0) != 3) { fprintf(stderr,"Wrong file format 1\n"); exit(1); }
    // Будуємо DFA
    DFA * fa = makeFA(A,S,s0);
    if (fa == NULL)                           { fprintf(stderr,"No memory\n");           exit(1); }
    // Додаємо кінцеві стани
    if (fscanf(in,"%u",&F) != 1)              { fprintf(stderr,"Wrong file format 2\n"); exit(1); }
    for(unsigned int j = 0; j < F; ++j)
    {
        if (fscanf(in,"%u",&f) != 1)          { fprintf(stderr,"Wrong file format 3\n"); exit(1); }
        if (!addFinite(fa,f))                 { fprintf(stderr,"Wrong file format 4\n"); exit(1); }
    }
    while(fgetc(in) != '\n');   // Сброс до кінця рядка - видалення '\n' з буфера вводу

    // Читаємо правила переходу до кінця файлу
    // Зчитуємо рядками
    for(char line[256]; fgets(line,sizeof(line),in);)
    {
        if (line[0] == '\n') continue; // Ігноруємо пусті рядки
        unsigned int s, sp; char c;
        // Читаємо дані з рядка
        if (sscanf(line,"%u %c %u",&s,&c,&sp) != 3) { fprintf(stderr,"Wrong file format 5\n"); exit(1); }
        // Додаємо правило. При помилці виходимо з програми
        if (!addRule(fa,s,c,sp))                    { fprintf(stderr,"Wrong transition rule %s\n", line); exit(1); }
    }
    return fa;
}

// Робота DFA
int accept(DFA*fa, const char * s)
{
    // Поточний стан = початковому
    fa->cs = fa->s0;
    // Для всіх символів рядка
    for(const char *c = s; *c; ++c)
    {
        if (*c == '\n') break;                           // Ігноруємо кінець рядка
        if ((unsigned int )(*c-'a') >= fa->al) return 0; // Невірний символ (не з алфавіту)
        
        fa->cs = fa->t[fa->cs][*c-'a'];                  // Виконоємо перехід
        if (fa->cs == NON) return 0;                     // Такого переходу не існує
    }
    // Все зчитано
    return fa->f[fa->cs];                                // Повертаємо признак кінцевого стану
}


int main(int argc, const char * argv[])
{
    // Отримуємо ім'я файлу та відкриваємо його
    char fileName[_MAX_PATH+1]; // Filename
    // Get filename
    if (argc > 1)
        strncpy(fileName,argv[1],_MAX_PATH);
    else
    {
        printf("Enter the file name: ");
        fgets(fileName,_MAX_PATH+1,stdin);
        if (fileName[strlen(fileName)-1] == '\n')
            fileName[strlen(fileName)-1] = '\0';
    }

    FILE * in = fopen(fileName,"rt");
    if (in == NULL)
    {
        fprintf(stderr,"Can not open file `%s`\n",fileName);
        return 1;
    }

    // Будуємо DFA
    DFA * fa = readDFA(in);
    fclose(in);


#if 0   // Простий тест

    // Доки зчитуємо по рядку, доти виводимо, чи приймає його DFA
    for(char line[256]; fgets(line,sizeof(line),stdin);)
    {
        if (line[0] == '\n') continue; // Empty string
        printf("Line %s       is %s\n",line,accept(fa,line)?"accepted":"rejected");
    }

#else

    // Завдання 9 — перевірка всіх k-символьних рядків
    for(;;)
    {
        printf("Task 9. Please, enter k (0 for quit): ");
        unsigned int k;
        // Зчитали k
        if (scanf("%d",&k) != 1) 
        {
            // Невірний ввод — ігноруємо
            // Сброс до кінця введеного рядка
            while(fgetc(stdin) != '\n');
            continue;
        }
        if (k == 0) break;

        // Генеруємо всі k-символьні рядки 

        // Рядок кодується як число від 0 до N^k, де N — розмір алфавіту
        // Потім розглядаємо запис в N-ій системі, з a -> 0, b -> 1, etc
        unsigned int last = 1;
        for(unsigned int i = 0; i < k; ++i) last *= fa->al;

        // Рядок
        char * s = malloc(k+1);

        int acpt = 1;
        // Будуємо рядок
        for(unsigned int i = 0; i < last; ++i)
        {
            unsigned int m = i;
            for(unsigned int j = 0; j < k; ++j)
            {
                s[j] = (char)('a' + m%fa->al);
                m /= fa->al;
            }
            s[k] = 0;

            //printf("Line %s       is %s\n",s,accept(fa,s)?"accepted":"rejected");
            // Перевіряємо. Якщо рядок не прийнято, далі перевіряти сенсу немає
            if (!accept(fa,s)) { acpt = 0; break; }
        }

        if (acpt)
            printf("Yes, DFA accepted all %u-symbol strings\n",k);
        else
            printf("Sorry, DFA not accepted all %u-symbol strings\n",k);

        free(s);
    }
#endif
    killFA(fa);
}

