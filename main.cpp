#include <iostream>
/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#include <fstream>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <time.h>
#include<iomanip>

#include "cppparser/parserthread.h"



using namespace std;

class FileLoader : public LoaderBase
{
public:
    virtual const char* data()
    {
        return (const char*)buf;
    }
    virtual unsigned int length()
    {
        return buf_size;
    }
    virtual const char* fileName()
    {
        return name;
    }
public:
    FileLoader(const char* fileName) : name(fileName), buf(0), buf_size(0)
    {
        load();
    }
    bool isLoad()
    {
        return buf_size != 0;
    }
protected:
    void load()
    {
        ifstream in(name, ios::binary);
        if (!in.is_open())
            return;

//        int begin = in.tellg();
//        in.seekg (0, ios::end);
//        int end = in.tellg();
//        int filelength = end - begin;
//        printf ("d",filelength);

        in.seekg(0,std::ios::end);
        buf_size = in.tellg();
        in.seekg(0);
        buf = new char[buf_size+2];

        buf[0] = 0;
        in.read(buf+1,buf_size);
        buf[buf_size+1] = '\0';

//        for(int i = 0;i<=buf_size;i++)
//        {
//            cout<< i<< " " << int(buf[i]) <<" " << char (buf[i])<<endl;
//        }

    }
    const char *    name;
    char*           buf;
    int             buf_size;
};

class ParserClient : public Parser
{
public:
    ParserClient()
    {
        tree = new TokensTree();
    }
    ~ParserClient()
    {
        delete tree;
    }
    bool parser(const char * fileName)
    {
        FileLoader * loader = new FileLoader(fileName);
        if (!loader->isLoad())
        {
            delete loader;
            return false;
        }
        files.push_back(fileName);
        ParserThreadOptions opts;
        opts.wantPreprocessor = false;
        opts.useBuffer = false;
        opts.bufferSkipBlocks = false;
        opts.bufferSkipOuterBlocks = false;
        opts.followLocalIncludes = false;
        opts.followGlobalIncludes = false;
        opts.loader = loader;
        ParserThread * ph = new ParserThread(this,fileName,true,opts,tree);
        bool b = ph->Parse();
        delete ph;
        return b;
    }
    void clear()
    {
        files.clear();
        tree->clear();
    }

    virtual cc_string GetFullFileName(const cc_string & Filename,const cc_string& filename,bool isGlobal)
    {
        return cc_text("");
    }
    virtual void OnParseFile(const cc_string & filename, int flag)
    {
    }

    void printToken(std::ostream & f, Token* tk, const string & head = "")
    {
        f << head ;//<< tk->GetTokenKindString()
        if (!tk->m_Children.empty())
            f << "+";
        if (tk->m_TokenKind == tkFunction)
            f << tk->m_Type << tk->m_Name << tk->m_Args << "\t";
        else
            f << tk->DisplayName() << "\t";
        f << "[" << tk->m_Line << "," << tk->m_ImplLine << "]" << endl;

    }

    void printTokenTree(std::ostream & f, Token* tk, const string & head = "")
    {
        printToken(f,tk,head);
        TokenIdxSet & ids = tk->m_Children;
        for (TokenIdxSet::iterator it = ids.begin(); it != ids.end(); it++)
        {
            Token * tk = tree->at(*it);
            printTokenTree(f,tk,head+"|\t");
        }
    }

    void printTokenDetail(std::ostream & f, int n)
    {
        Token * tk = tree->at(n);
        f << tk->GetTokenKindString()<<endl;
        if (!tk->m_Children.empty())
            f << "+";
        if (tk->m_TokenKind == tkFunction)
            f << tk->m_Type << " "<< tk->m_Name << tk->m_Args << "\t";
        else
            f << tk->DisplayName() << "\t";
        f << "[" << tk->m_Line << "," << tk->m_ImplLine << "]" << endl;

//        if (!tk->m_TemplateArgument.empty())
//            f << "template argument " << tk->m_TemplateArgument << endl;
    }

    void printList(std::ostream & f)
    {
        f.setf(ios::left);
        TokenList & tokens = tree->m_Tokens;
        for (TokenList::iterator it = tokens.begin(); it != tokens.end(); it++)
        {
            Token * tk = *it;
//            f << std::setw(15) << tk->GetSelf()<< " "<< tk->GetTokenKindString() << "\t" << tk->DisplayName() << "\t[" << tk->m_Line << "," << tk->m_ImplLine << "]" <<tk->m_TemplateArgument<< endl;
        }
    }

    void printTree(std::ostream & f)
    {
        TokenList & tokens = tree->m_Tokens;
        for (TokenList::iterator it = tokens.begin(); it != tokens.end(); it++)
        {
            Token* tk = *it;
            if (tk->GetParentToken() == 0)
            {
                printTokenTree(f,tk);
            }
        }
    }

    void printFiles(std::ostream & f)
    {
        for (int i = 0; i < files.size(); i++)
        {
            f << i << ". " << files[i] << endl;
        }
    }
    void printFileToken(std::ostream & f, int n)
    {
        f << "show file:" << files[n] << endl;
        TokenIdxSet result;
        tree->FindTokensInFile(files[n], result, 0xFFFF);
        for (TokenIdxSet::iterator it = result.begin(); it != result.end(); it++)
        {
            Token* tk = tree->at(*it);
            if (tk->GetParentToken() == 0)
            {
                printTokenTree(f,tk);
            }
        }

    }

public:
    TokensTree  * tree;
    vector<string> files;
};

void showHelp(std::string & logname)
{
    cout.setf(ios::left);
    cout << "-> cmd: (load,files,clear)  (list,tree,show,dump) (log,logoff) (help,exit) log=" << logname << endl;
    cout << setw(30) << "\tload  filename" << "load c++ file path" << endl
         << setw(30) << "\tfiles" << "show load files" << endl
         << setw(30) << "\tprint  TokenIndex" << "print token information" << endl
         << setw(30) << "\tclear"<< "clear all files" << endl
         << setw(30) << "\tlist" << "show token by list" << endl
         << setw(30) << "\ttree" << "show token by tree" << endl
         << setw(30) << "\tshow  n" << "show signal file token  by tree" << endl
         << setw(30) << "\tdump" << "dump token tree" << endl
         << setw(30) << "\tlog  filename" << "setup log filepath" << endl
         << setw(30) << "\tlogoff" << "setup log = std::cout" << endl
         << setw(30) << "\thelp" << "show this help" << endl
         << setw(30) << "\texit" << "exit application" << endl;
}

int main()
{
    cout << "Hello world! C++ Parser Programing!" << endl;
    ParserClient client;

    std::string     logname = "off";
    std::ofstream   fout;
    std::ostream *  log = 0;

    showHelp(logname);

    while(true)
    {
        cout << "-> ";
        string cmd, opts;
        cin >> cmd;

        if (log)
            *log << "\ncommand:" <<cmd << endl;

        if (cmd == "load")
        {
            cin >> opts;
            if (!opts.empty() && client.parser(opts.c_str()))
            {
                cout << "load file ok! " << opts << endl;
                if (log)
                    *log << "load file:" << opts << endl;
            }
            else
            {
                cout << "error load filename! " << opts << endl;
            }
        }
        else if ( cmd == "print")
        {
            int n = -1;
            cin >> n;
            if (n >= 0 && n < client.tree->size())
            {
                client.printTokenDetail(cout,n);
                if (log)
                    client.printTokenDetail(*log,n);
            }
            else
            {
                cout << "-> error input token index " << n << endl;
            }

        }
        else if (cmd == "files")
        {
            client.printFiles(cout);
            if (log)
                client.printFiles(*log);
        }
        else if (cmd == "clear")
        {
            client.clear();
            if (log)
                *log << "clear all files" << endl;
        }
        else if (cmd == "list")
        {
            client.printList(std::cout);
            if (log)
                client.printList(*log);
        }
        else if (cmd == "tree")
        {
            client.printTree(cout);
            if (log)
                client.printTree(*log);
        }
        else if (cmd == "show")
        {
            int n = -1;
            cin >> n;
            if (n >= 0 && n < client.files.size())
            {
                client.printFileToken(cout,n);
                if (log)
                    client.printFileToken(*log,n);
            }
            else
            {
                cout << "-> error input file index " << n << endl;
            }
        }
        else if (cmd == "dump")
        {
            cout << client.tree->m_Tree.dump();
            if (log)
                *log << client.tree->m_Tree.dump();
        }
        else if (cmd == "logoff")
        {
            if (fout.is_open())
                fout.close();
            log = 0;
            logname = "off";
            cout << "log=" << logname << endl;
        }
        else if (cmd == "log")
        {
            cin >> opts;
            if (fout.is_open())
                fout.close();
            fout.open(opts.c_str(), ios::out | ios::app);
            if (fout.is_open())
            {
                log = &fout;
                logname = opts;
                time_t tt = time(0);
                tm * t = localtime(&tt);

                *log << "log time: "<< asctime(t) << endl;
            }
            else
            {
                log = 0;
                logname = "off";
            }
            cout << "log=" << logname << endl;
        }
        else if (cmd == "help")
        {
            showHelp(logname);
        }
        else if (cmd == "exit")
        {
            break;
        }
        else
        {
            cout << "-> error input command" << endl;
        }
    }

    if (fout.is_open())
        fout.close();

    return 0;
}
