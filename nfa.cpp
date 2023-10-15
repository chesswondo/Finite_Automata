#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <fstream>

using namespace std;

class NFA
{
    map<pair<int,int>,set<int>> t; // Таблиця переходів (пара стан+символ -> множина станів)
    set<int>                    f; // Таблиця кінцевих станів
    int                        al; // Довжина алфавіта
    int                        sc; // Кількість станів
    int                        s0; // Початковий стан
    set<int>                   cs; // Поточний стан
public:
    // Конструювання пустого автомату
    NFA(int al, int sc, int s0)
        :al(al),sc(sc),s0(s0)
    {}
        // Додавання правила ('-' для пустого символа)
        void addRule(int s, char c, int sp)
        {
            // Перевірка коректності правила
            if (s  < 0 || s  >= sc || sp < 0 || sp >= sc ||
                !((c < 'a' + al && c >= 'a') || (c == '-')))
            {
                cout << s << "  " << c << "  " << sp << endl;
                throw runtime_error("Wrong rule");
            }
            // Додавання правила (правил може бути багато)
            t[make_pair(s,c)].insert(sp);
        }

        // Додаємо позначки кінцевих станів
        void addFinite(int fs)
        {
            // Перевірка коректності (вихід за межі)
            if (fs < 0 || fs >= sc) throw runtime_error("Wrong finite state");
            f.insert(fs);
        }

        // Робота NFA
        bool accept(const string& s);

        // Розмір алфавиту
        unsigned int alphabetSize() { return al; }

    private:

        // Допоміжні функції
        set<int> closstep(const set<int>& state);     // Один крок переходу для пустого символа
        set<int> closure(const set<int>& state);      // Побудова всіх переходів при пустому символі
        set<int> move(const set<int>& state, char x); // Один крок переходу для символу x
};

// Створення автомату з файлу
NFA readNFA(istream& in)
{
    int A, S, s0, F, f;
    // Зчитування параметрів, перевірка, побудова пустого автомату
    if (!(in >> A >> S >> s0))     throw runtime_error("Wrong file format 1");
    if (A < 0 || S < 0 || s0 < 0)  throw runtime_error("Wrong file format 2");
    NFA fa(A,S,s0);
    // Кінцеві стани
    if (!(in >> F) || (F < 0))     throw runtime_error("Wrong file format 3");

    for(int j = 0; j < F; ++j)
    {
        if (!(in >> f) || (f < 0)) throw runtime_error("Wrong file format 4");
        fa.addFinite(f);
    }
    // Сброс до кінця рядка
    in.ignore(numeric_limits<streamsize>::max(), '\n');

    // Читаємо правила переходу до кінця файлу
    int s, sp; char c;
    while(in >> s >> c >> sp)
    {
        // додавання правила
        fa.addRule(s,c,sp);
    }
    // Якщо помилка читання ДО кінця файлу
    if (!in.eof())                 throw runtime_error("Wrong file format 5");
    return fa;
}

// Один крок переходу для пустого символа
set<int> NFA::closstep(const set<int>& state)
{
    set<int> res(state);
    for(int s: state)
    {
        // Для кожного стану з вихідної множини шукаемо всі "пусті" переходи
        // та додаємо нові стани в множину
        auto it = t.find(make_pair(s,'-'));
        if (it != t.end())
        {
            for(int v: it->second) res.insert(v);
        }
    }
    return res;
}

// Побудова всіх переходів при пустому символі
set<int> NFA::closure(const set<int>& state)
{
    set<int> r,p = state;
    for(;;)
    {
        // Виконуємо переходи доти, доки вони можливі
        r = closstep(p);
        // Якщо їх нема, множина станів не зміниться
        if (r == p) return r;
        p = r;
    };
}

// Один крок переходу для символу x
set<int> NFA::move(const set<int>& state, char x)
{
    set<int> res;
    for(int s: state)
    {
        // Для кожного стану з вихідної множини шукаемо переходи для x
        // та додаємо нові стани в множину
        auto it = t.find(make_pair(s,x));
        if (it != t.end())
        {
            for(int v: it->second) res.insert(v);
        }
    }
    return res;
}

//
bool NFA::accept(const string& s)
{
    cs.clear();
    // Поточний стан = початковому
    cs.insert(s0);
    // Всі "пусті" переходи
    cs = closure(cs);

    for(char c: s)    // Для всіх символів рядка
    {
        cs = closure(move(cs,c));      // Виконуємо переходи в новий стан
        if (cs.empty()) return false;  // Якщо їх немає — помилка
    }
    // По закінченню перевіряємо наявність кінцевого стану
    for(int i: cs) if (f.count(i)) return true;
    return false;
}


int main(int argc, char * argv[])
{
    try {
        string fileName; // Filename
        if (argc > 1)
            fileName = argv[1];
        else
        {
            printf("Enter the file name: ");
            getline(cin,fileName);
        }

        ifstream in(fileName.c_str());

        NFA fa = readNFA(in);

#if 0 // Простий тест

        string s;
        // Доки зчитуємо по рядку, доти виводимо, чи приймає його NFA
        while(cin>>s)
        {
            cout << (fa.accept(s) ? "accepted" : "rejected") << endl;
        }

#else
        // Завдання 9 — перевірка всіх k-символьних рядків
        for(;;)
        {
            cout << "Task 9. Please, enter k (0 for quit): ";
            unsigned int k;
            cin >> k;
            if (k == 0) break;

            // Генеруємо всі k-символьні рядки, як і для DFA
            unsigned int last = 1;
            for(unsigned int i = 0; i < k; ++i) last *= fa.alphabetSize();
            string s(k,' ');

            int acpt = 1;
            for(unsigned int i = 0; i < last; ++i)
            {
                unsigned int m = i;
                for(unsigned int j = 0; j < k; ++j)
                {
                    s[j] = (char)('a' + m%fa.alphabetSize());
                    m /= fa.alphabetSize();
                }
                s[k] = 0;

                // Перевіряємо. Якщо рядок не прийнято, далі перевіряти сенсу немає
                if (!fa.accept(s)) { acpt = 0; break; }
            }

            if (acpt)
                cout << "Yes, NFA accepted all " << k << "-symbol strings\n";
            else
                cout << "Sorry, NFA not accepted all " << k << "-symbol strings\n";

        }

#endif


    } catch(exception&e)  // Обробка помилок: вивід повідомлення і вихід
    {
        cerr << e.what();
        return 1;
    }
}

