
#include <clang-c/Index.h>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

/*
* Run with:
* LIBCLANG_TIMING=1 ./complete file.cc line column [clang args...]
*/


const char *
clang_getCompletionChunkKindSpelling(enum CXCompletionChunkKind Kind) {
    switch (Kind) {
    case CXCompletionChunk_Optional:
        return "Optional";
    case CXCompletionChunk_TypedText:
        return "TypedText";
    case CXCompletionChunk_Text:
        return "Text";
    case CXCompletionChunk_Placeholder:
        return "Placeholder";
    case CXCompletionChunk_Informative:
        return "Informative";
    case CXCompletionChunk_CurrentParameter:
        return "CurrentParameter";
    case CXCompletionChunk_LeftParen:
        return "LeftParen";
    case CXCompletionChunk_RightParen:
        return "RightParen";
    case CXCompletionChunk_LeftBracket:
        return "LeftBracket";
    case CXCompletionChunk_RightBracket:
        return "RightBracket";
    case CXCompletionChunk_LeftBrace:
        return "LeftBrace";
    case CXCompletionChunk_RightBrace:
        return "RightBrace";
    case CXCompletionChunk_LeftAngle:
        return "LeftAngle";
    case CXCompletionChunk_RightAngle:
        return "RightAngle";
    case CXCompletionChunk_Comma:
        return "Comma";
    case CXCompletionChunk_ResultType:
        return "ResultType";
    case CXCompletionChunk_Colon:
        return "Colon";
    case CXCompletionChunk_SemiColon:
        return "SemiColon";
    case CXCompletionChunk_Equal:
        return "Equal";
    case CXCompletionChunk_HorizontalSpace:
        return "HorizontalSpace";
    case CXCompletionChunk_VerticalSpace:
        return "VerticalSpace";
    }

    return "Unknown";
}



void print_completion_string(CXCompletionString completion_string, FILE *file) {
    int I, N;

    N = clang_getNumCompletionChunks(completion_string);
    for (I = 0; I != N; ++I) {
        CXString text;
        const char *cstr;
        enum CXCompletionChunkKind Kind
        = clang_getCompletionChunkKind(completion_string, I);

        if (Kind == CXCompletionChunk_Optional) {
            fprintf(file, "{Optional ");
            print_completion_string(
                clang_getCompletionChunkCompletionString(completion_string, I),
                file);
            fprintf(file, "}");
            continue;
        }

        if (Kind == CXCompletionChunk_VerticalSpace) {
            fprintf(file, "{VerticalSpace  }");
            continue;
        }

        text = clang_getCompletionChunkText(completion_string, I);
        cstr = clang_getCString(text);
        fprintf(file, "{%s %s}",
                clang_getCompletionChunkKindSpelling(Kind),
                cstr ? cstr : "");
        clang_disposeString(text);
    }

}


void print_completion_result(CXCompletionResult *completion_result,
                             CXClientData client_data) {
    FILE *file = (FILE *)client_data;
    CXString ks = clang_getCursorKindSpelling(completion_result->CursorKind);

    fprintf(file, "%s:", clang_getCString(ks));
    clang_disposeString(ks);

    print_completion_string(completion_result->CompletionString, file);
    fprintf(file, " (%u)",
            clang_getCompletionPriority(completion_result->CompletionString));
    switch (clang_getCompletionAvailability(completion_result->CompletionString)) {
    case CXAvailability_Available:
        break;

    case CXAvailability_Deprecated:
        fprintf(file, " (deprecated)");
        break;

    case CXAvailability_NotAvailable:
        fprintf(file, " (unavailable)");
        break;
    }
    fprintf(file, "\n");
}





int main(int argc, char** argv) {

    printf("code completion demo project with lib clang support!!! asmwarrior \n");
    printf("ollydbg from codeblocks forum\n\n");

    printf("it will bydefault open the test.cpp file!\n");
    printf("* to show the code completion list, please enter the command\n");
    printf("cc line column [ENTER]\n");
    printf("* to exit, just enter");
    printf("exit [ENTER]\n");

    char test_file[] = "test.cpp";

    // hard coded include path
    const char *command_argv[]={
    "-ID:\\mingw_loaden\\lib\\gcc\\i686-mingw32\\4.4.5\\include\\c++",
    "-ID:\\mingw_loaden\\lib\\gcc\\i686-mingw32\\4.4.5\\include\\c++\\i686-mingw32",
    "-ID:\\mingw_loaden\\lib\\gcc\\i686-mingw32\\4.4.5\\include\\c++\\backward",
    "-ID:\\mingw_loaden\\include",
    "-ID:\\mingw_loaden\\lib\\gcc\\i686-mingw32\\4.4.5\\include",
    "-ID:\\mingw_loaden\\lib\\gcc\\i686-mingw32\\4.4.5\\include-fixed"
    };


    CXIndex idx = clang_createIndex(1, 0);
    if (!idx) {
        std::cerr << "createIndex failed" << std::endl;
        return 2;
    }

    CXTranslationUnit u = clang_parseTranslationUnit(idx, test_file, command_argv,
                          6, 0, 0,
                          CXTranslationUnit_PrecompiledPreamble
                          | CXTranslationUnit_CXXPrecompiledPreamble);

    if (!u) {
        std::cerr << "parseTranslationUnit failed" << std::endl;
        return 2;
    }


    //entering the command loop

    string cmd;
    while (true)
    {
        cin >> cmd;
        if (cmd=="cc")
        {
            int line = 0;
            int column = 0;
            cin >> line;
            cin >> column;

            // reparse the file
            clang_reparseTranslationUnit(u, 0, 0, 0);

            // do code completion
            CXCodeCompleteResults* res = clang_codeCompleteAt(u, test_file,
                                 line, column,
                                 0, 0, 0);
            if (!res) {
                std::cerr << "Could not complete" << std::endl;
                return 2;
            }

            unsigned i, n = res->NumResults;
            if(n==0)
                printf("no code completion!!!\n");

            /* Sort the code-completion results based on the typed text. */
            clang_sortCodeCompletionResults(res->Results, res->NumResults);

            for (i = 0; i != n; ++i)
                print_completion_result(res->Results + i, stdout);


            clang_disposeCodeCompleteResults(res);

        }
        else if(cmd == "exit")
        {
            break;
        }
    }

    //clean up

    clang_disposeTranslationUnit(u);
    clang_disposeIndex(idx);
    return 0;
}
