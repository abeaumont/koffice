#ifndef WINWORDDOC_H
#define WINWORDDOC_H

#include <string.h>
#include <qstring.h>
#include <kdebug.h>
#include "klaola.h"
#include "myfile.h"
#include "fib.h"


class WinWordDoc {

public:
	WinWordDoc(myFile in);
	~WinWordDoc();
	
private:
    myFile doc;     // raw data
    FIB *fib;
};
#endif // WINWORDDOC_H
