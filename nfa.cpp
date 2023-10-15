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
    map<pair<int,int>,set<int>> t; // ������� �������� (���� ����+������ -> ������� �����)
    set<int>                    f; // ������� ������� �����
    int                        al; // ������� �������
    int                        sc; // ʳ������ �����
    int                        s0; // ���������� ����
    set<int>                   cs; // �������� ����
public:
    // ������������� ������� ��������
    NFA(int al, int sc, int s0)
        :al(al),sc(sc),s0(s0)
    {}
        // ��������� ������� ('-' ��� ������� �������)
        void addRule(int s, char c, int sp)
        {
            // �������� ���������� �������
            if (s  < 0 || s  >= sc || sp < 0 || sp >= sc ||
                !((c < 'a' + al && c >= 'a') || (c == '-')))
            {
                cout << s << "  " << c << "  " << sp << endl;
                throw runtime_error("Wrong rule");
            }
            // ��������� ������� (������ ���� ���� ������)
            t[make_pair(s,c)].insert(sp);
        }

        // ������ �������� ������� �����
        void addFinite(int fs)
        {
            // �������� ���������� (����� �� ���)
            if (fs < 0 || fs >= sc) throw runtime_error("Wrong finite state");
            f.insert(fs);
        }

        // ������ NFA
        bool accept(const string& s);

        // ����� ��������
        unsigned int alphabetSize() { return al; }

    private:

        // ������� �������
        set<int> closstep(const set<int>& state);     // ���� ���� �������� ��� ������� �������
        set<int> closure(const set<int>& state);      // �������� ��� �������� ��� ������� ������
        set<int> move(const set<int>& state, char x); // ���� ���� �������� ��� ������� x
};

// ��������� �������� � �����
NFA readNFA(istream& in)
{
    int A, S, s0, F, f;
    // ���������� ���������, ��������, �������� ������� ��������
    if (!(in >> A >> S >> s0))     throw runtime_error("Wrong file format 1");
    if (A < 0 || S < 0 || s0 < 0)  throw runtime_error("Wrong file format 2");
    NFA fa(A,S,s0);
    // ʳ���� �����
    if (!(in >> F) || (F < 0))     throw runtime_error("Wrong file format 3");

    for(int j = 0; j < F; ++j)
    {
        if (!(in >> f) || (f < 0)) throw runtime_error("Wrong file format 4");
        fa.addFinite(f);
    }
    // ����� �� ���� �����
    in.ignore(numeric_limits<streamsize>::max(), '\n');

    // ������ ������� �������� �� ���� �����
    int s, sp; char c;
    while(in >> s >> c >> sp)
    {
        // ��������� �������
        fa.addRule(s,c,sp);
    }
    // ���� ������� ������� �� ���� �����
    if (!in.eof())                 throw runtime_error("Wrong file format 5");
    return fa;
}

// ���� ���� �������� ��� ������� �������
set<int> NFA::closstep(const set<int>& state)
{
    set<int> res(state);
    for(int s: state)
    {
        // ��� ������� ����� � ������� ������� ������� �� "����" ��������
        // �� ������ ��� ����� � �������
        auto it = t.find(make_pair(s,'-'));
        if (it != t.end())
        {
            for(int v: it->second) res.insert(v);
        }
    }
    return res;
}

// �������� ��� �������� ��� ������� ������
set<int> NFA::closure(const set<int>& state)
{
    set<int> r,p = state;
    for(;;)
    {
        // �������� �������� ����, ���� ���� ������
        r = closstep(p);
        // ���� �� ����, ������� ����� �� ��������
        if (r == p) return r;
        p = r;
    };
}

// ���� ���� �������� ��� ������� x
set<int> NFA::move(const set<int>& state, char x)
{
    set<int> res;
    for(int s: state)
    {
        // ��� ������� ����� � ������� ������� ������� �������� ��� x
        // �� ������ ��� ����� � �������
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
    // �������� ���� = �����������
    cs.insert(s0);
    // �� "����" ��������
    cs = closure(cs);

    for(char c: s)    // ��� ��� ������� �����
    {
        cs = closure(move(cs,c));      // �������� �������� � ����� ����
        if (cs.empty()) return false;  // ���� �� ���� � �������
    }
    // �� ��������� ���������� �������� �������� �����
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

#if 0 // ������� ����

        string s;
        // ���� ������� �� �����, ���� ��������, �� ������ ���� NFA
        while(cin>>s)
        {
            cout << (fa.accept(s) ? "accepted" : "rejected") << endl;
        }

#else
        // �������� 9 � �������� ��� k-���������� �����
        for(;;)
        {
            cout << "Task 9. Please, enter k (0 for quit): ";
            unsigned int k;
            cin >> k;
            if (k == 0) break;

            // �������� �� k-�������� �����, �� � ��� DFA
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

                // ����������. ���� ����� �� ��������, ��� ��������� ����� ����
                if (!fa.accept(s)) { acpt = 0; break; }
            }

            if (acpt)
                cout << "Yes, NFA accepted all " << k << "-symbol strings\n";
            else
                cout << "Sorry, NFA not accepted all " << k << "-symbol strings\n";

        }

#endif


    } catch(exception&e)  // ������� �������: ���� ����������� � �����
    {
        cerr << e.what();
        return 1;
    }
}

