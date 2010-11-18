#ifndef _CCHELPER_H_
#define _CCHELPER_H_

#include <vector>
#include <deque>
#include <queue>
#include <map>
#include <cstring>
#include <string>
#include <algorithm>
#include <assert.h>
#include <istream>
#include <ostream>



//#define CC_UNICODE  1

#ifdef CC_UNICODE
#include <wchar.h>
#define cc_text(x) L##x
typedef wchar_t                         cc_char;
#else
typedef char                            cc_char;
#define cc_text(x) (x)
#endif

typedef std::basic_string<cc_char>  cc_string;

inline cc_string & operator << (cc_string & str, cc_string & s)
{
    return str += s;
}

inline cc_string & operator << (cc_string & str, const cc_string & s)
{
    return str += s;
}

inline cc_string & operator << (cc_string & str, const cc_char * s)
{
    return str += s;
}

inline cc_string & operator << (cc_string & str, cc_char * s)
{
    return str += s;
}

inline cc_string & operator << (cc_string & str, cc_char ch)
{
    return str += ch;
}

inline cc_string cc_ch2s(cc_char sz)
{
    cc_string str;
    return str += sz;
}

inline cc_string cc_u2s(unsigned int u)
{
    if(!u)
        return cc_text("0");
    cc_string result(cc_text("")),revresult(cc_text(""));
    int i = 0;
    while(u>0)
    {
        revresult +=  (char)(cc_text('0') + (u % 10));
        u/=10;
        i++;
    }
    while(i>0)
    {
        i--;
        result += revresult[i];
    }
    return result;
}

inline cc_string cc_i2s(int i)
{
    cc_string result(cc_text(""));
    if(i<0)
        result << cc_text('-');
    result << cc_u2s(abs(i)).c_str();
    return result;
}


inline void cc_lower(cc_string & str)
{
    std::transform(str.begin(),str.end(),str.begin(),::tolower);
}

inline bool cc_isupper(const cc_string & str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        if (!::isupper(str[i]))
            return false;
    }
    return true;
}

inline void cc_trimLeft(cc_string & str)
{
    size_t pos = str.find_first_not_of(cc_text(" \t\r\n"));
    if (pos != std::string::npos)
        str.erase(0,pos);
}
inline void cc_trimRight(cc_string & str)
{
    size_t pos = str.find_last_not_of(cc_text(" \t\r\n"));
    if (pos != std::string::npos)
        str.erase(str.begin()+pos+1,str.end());
}
inline void cc_trimAll(cc_string & str)
{
    cc_trimLeft(str);
    cc_trimRight(str);
}

class LoaderBase
{
public:
    virtual const cc_char* data() = 0;
    virtual unsigned int length() = 0;
    virtual const cc_char* fileName() = 0;
    virtual void Sync()
    {
    }
    virtual void Release()
    {
    }
};

class wxCriticalSection
{
public:
    void Enter()
    {
    }
    void Leave()
    {
    }
};

class wxCriticalSectionLocker
{
public:
    wxCriticalSectionLocker(wxCriticalSection & cs)
    {
    }
};

inline bool wxFileExists(const cc_string & filename)
{
    return true;
}

class wxFile
{
public:
    wxFile(const cc_string & fileName) : m_file(fileName)
    {
    }
    size_t length()
    {
        return 0;
    }

    cc_string m_file;
};

inline bool cbRead(wxFile & file,cc_string & str)
{
    return false;
}

class Parser
{
public:
    virtual cc_string GetFullFileName(const cc_string & Filename,const cc_string& filename,bool isGlobal)
    {
        return cc_text("");
    }
    virtual void OnParseFile(const cc_string & filename, int flag)
    {
    }
};

inline void SaveIntToFile(std::ostream & f, int i)
{
    // This used to be done as        f->Write(&i, sizeof(int));
    //which is incorrect because it assumes a consistant byte order
    //and a constant int size
    unsigned int const j = i; // rshifts aren't well-defined for negatives
    char c[4] = { (char) (j>>0&0xFF), (char) (j>>8&0xFF), (char) (j>>16&0xFF), (char) (j>>24&0xFF) };
    //f->Write( c, 4 );
    f.write(c,4);
}

inline bool LoadIntFromFile(std::istream & f, int* i)
{
//    See SaveIntToFile
//    return f->Read(i, sizeof(int)) == sizeof(int);

    char c[4];
    if (!f.read(c,4).good())
        return false;
    //if ( f->Read( c, 4 ).LastRead() != 4 ) return false;
    *i = ( c[0]<<0 | c[1]<<8 | c[2]<<16 | c[3]<<24 );
    return true;
}

template <typename _CharT>
inline void SaveStringToFile(std::ostream & f, const std::basic_string<_CharT> & str)
{
    //const wxWX2MBbuf psz = str.mb_str(wxConvUTF8);
    // TODO (MortenMacFly#5#): Can we safely use strlen here?
    int size = str.length()*(sizeof(_CharT)/sizeof(char));
    if (size >= 32767)
        size = 32767;
    SaveIntToFile(f, size);
    if(size)
        f.write((char*)(str.c_str()), size*(sizeof(_CharT)/sizeof(char)));
}

inline void cc_buf2str(char* buf, int size, std::basic_string<char> & str)
{
    str = buf;
}

inline void cc_buf2str(char* buf, int size, std::basic_string<wchar_t> & str)
{
    str = (wchar_t*)buf;
}

template <typename _CharT>
inline bool LoadStringFromFile(std::istream & f, std::basic_string<_CharT>& str)
{
    int size;
    if (!LoadIntFromFile(f, &size))
        return false;
    bool ok = true;
    if (size > 0 && size <= 32767)
    {
        char buf[32768];
        ok = f.read(buf,size).good();//f->Read(buf, size).LastRead() == (size_t)size;
        buf[size] = '\0';
        cc_buf2str(buf,size,str);
    }
    else // doesn't fit in our buffer, but still we have to skip it
    {
        str.clear();
        size = size & 0xFFFFFF; // Can't get any longer than that
        //f->SeekI(size, wxFromCurrent);
        f.seekg(size,std::ios_base::cur);
    }
    return ok;
}

#endif
