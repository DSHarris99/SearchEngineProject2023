%{
#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <vector>

#include "hashtable.cpp" 
using namespace std;

// Create an instance of the HashTable
HashTable documentTable(1000);
int totalTokens = 0;
int uniqueTokens = 0;

// Declare yylex and yyin with C++ linkage
int yylex();

int readCumulativeTotal() {
    int cumulativeTotal = 0;
    ifstream file("cumulative_total.txt");
    if (file.is_open()) {
        file >> cumulativeTotal;
        file.close();
    }
    return cumulativeTotal;
}

// Function to write the cumulative total to a file
void writeCumulativeTotal(int cumulativeTotal) {
    ofstream file("cumulative_total.txt");
    if (file.is_open()) {
        file << cumulativeTotal;
        file.close();
    }
}

void DowncaseAndPrint(const char Str[]) {
    char Ch;
	string downcasedStr;
    for (int i = 0; i < (int)strlen(Str); i++) {
        if ('A' <= Str[i] && Str[i] <= 'Z') {
            Ch = (char)('a' + Str[i] - 'A');
			downcasedStr += Ch;
            //fprintf(yyout, "%c", Ch);
        } else {
			downcasedStr += Str[i];
            //fprintf(yyout, "%c", Str[i]);
        }
    }
	string tokenString = downcasedStr; // Get the matched token string
	bool alreadyExists = documentTable.tokenExists(tokenString);

	if (!alreadyExists)
	{
		uniqueTokens++;
	}
	
    documentTable.InsertOrUpdate(downcasedStr, 1);
	totalTokens++;


}

void HandleHyphenatedWordsAndPrint(const char Str[]) {
	string word;
	for (int i = 0; Str[i] != '\0'; i++) {
		if (Str[i] == '-') {
			if (!word.empty()) {
				DowncaseAndPrint(word.c_str());
				word.clear();
			}
		} else {
			word += Str[i];
		}
	}
	if (!word.empty()) {
		DowncaseAndPrint(word.c_str());
	}
}

void RemoveWordPunctuationAndPrint(const char Str[]) {
	string modifiedStr;
	for (int i = 0; Str[i] != '\0'; i++) {
		if (!ispunct(Str[i])) {  // Check if the character is not punctuation
			modifiedStr += Str[i];
		}
	}
	DowncaseAndPrint(modifiedStr.c_str());
}

%}

%option noyywrap
%s HTML_TAG

ATTRIBUTE [ \n\t]+(([A-Za-z\-_]+)?[ \n\t]*=?[ \n\t]*((\"[^\"]*\")|([A-Za-z0-9]+)|({URL}))[ \n\t]*)+[ \n\t]*

URL ((http)|(ftp))s?:\/\/[A-Za-z0-9]+([\-\.]{1}[A-Za-z0-9]+)*\.[A-Za-z0-9]{2,}(:[0-9]{1,})?(\/[A-Za-z0-9_~\.\-]*)*

%%
((http)|(ftp))s?:\/\/[A-Za-z0-9]+([\-\.]{1}[A-Za-z0-9]+)*\.[A-Za-z0-9]{2,}(:[0-9]{1,})?(\/[A-Za-z0-9_~\.\-]*)* {
    // reading URLs
    DowncaseAndPrint(yytext);
    //fprintf(yyout, "\n");

}

[A-Za-z0-9_\-\.]+@([A-Za-z0-9_\-]+\.)+[A-Za-z0-9_\-]{2,4} {
    // Handle email tokens
    DowncaseAndPrint(yytext);
    //fprintf(yyout, "\n");

}

[0-9]{3}-[0-9]{3}-[0-9]{4} {
    // Handle phone number tokens
	DowncaseAndPrint(yytext);
    //fprintf(yyout, "%s\n", yytext);
	
}

[0-9]*\.[0-9]+ {
    // Remove floating point numbers entirely
}

[0-9]+(,[0-9]+)* {
    // Do nothing for numbers??
}

[0-9]{1,2}(:[0-9]{2})+ {
    // Ignore time
}

[0-9]+\.[0-9]+(\.[0-9]+)+ {
    // Handle version tokens
	DowncaseAndPrint(yytext);
    //fprintf(yyout, "%s\n", yytext);

}

"<"[^>]+">" {
    // Erase global HTML
}

"<"!?[A-Za-z0-9]+{ATTRIBUTE}*[\/]?">" {
    // Do nothing for start tag
}

"<"[\/][A-Za-z0-9]+">" {
    // Do nothing for end tag
}

[A-Z0-9][A-Z0-9]* {
    // Handle uppercase word tokens
    DowncaseAndPrint(yytext);
    //fprintf(yyout, "\n");

}

[A-Za-z]+\.([A-Za-z]+\.)+ {
    // Handle abbreviation tokens
    DowncaseAndPrint(yytext);
    //fprintf(yyout, "\n");

}

. {
    // Ignore other characters
}

[A-Za-z]+(\-[A-Za-z]+)+ {
    // Handle hyphenated word tokens by replacing hyphens with spaces
    HandleHyphenatedWordsAndPrint(yytext);
}

[A-Za-z]+('[A-Za-z]+)? {
    // Handle words with punctuation
    RemoveWordPunctuationAndPrint(yytext);
}
[A-Za-z][a-z]* {
    // Handle lowercase word tokens
    DowncaseAndPrint(yytext);
    //fprintf(yyout, "\n");
}

[\n\t ] {
    // Ignore whitespace
}

%%


int main(int argc, char **argv) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " input_file output_file" << endl;
        return 1;
    }

    const string inputFileName = argv[1];
    const string outputFileName = argv[2];

    // Read the cumulative total from the file
    int cumulativeTotal = readCumulativeTotal();

    // Open the input file for processing
    ifstream inputFile(inputFileName);
    if (inputFile.is_open()) {
        string line;
        while (getline(inputFile, line)) {
            // Process each line in the file
            char *str = const_cast<char *>(line.c_str());
            yy_scan_string(str); // Set the input string for Flex
            yylex();
        }
        //cout << "Total Tokens: " << totalTokens << endl;
        //cout << "Unique Tokens: " << uniqueTokens << endl;

		//Sort and print the hashtable's contents to the specified output file
		documentTable.Print(outputFileName.c_str());
		//documentTable.PrintDocumentTable();
		documentTable.Clear();
        inputFile.close();
    } else {
        cerr << "Error opening file: " << inputFileName << endl;
        return 1;
    }

    // Add the totalTokens from this run to the cumulative total
    cumulativeTotal += totalTokens;

    // Update the cumulative total in the file
    writeCumulativeTotal(cumulativeTotal);

    //cout << "Total number of tokens in collection: " << cumulativeTotal << endl << endl;
    return 0;
}

