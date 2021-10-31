#include "stdio.h"
#include "windows.h"


char* colors[] = {
    "BLACK", 
    "BLUE", 
    "GREEN", 
    "CYAN", 
    "RED", 
    "MAGENTA", 
    "BROWN", 
    "LIGHTGRAY", 
    "DARKGRAY", 
    "LIGHTBLUE", 
    "LIGHTGREEN", 
    "LIGHTCYAN", 
    "LIGHTRED", 
    "LIGHTMAGENTA", 
    "YELLOW", 
    "WHITE" 
};

typedef struct
{
    HANDLE hbuf;
    COORD size;
    COORD windowPos;
    COORD windowSize;
    COORD outToPos;
} Console;

Console* initConsole();
void destroyConsole();
void setWindow(Console* cons, int x, int y, int sX, int sY);
void writeCharacter(Console* cons, int x, int y, CHAR_INFO c);
void writeString(Console* cons, const char* str, WORD attr);
void writeNewLine(Console* cons);
void scrollWindowUp(Console* cons);

Console* initConsole()
{
    Console* cons = calloc(sizeof(Console), 1);

    cons->hbuf = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, 0, NULL, 
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (cons->hbuf == INVALID_HANDLE_VALUE)
    {
        destroyConsole(cons);
        return NULL;
    }
    
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(cons->hbuf, &info))
    {
        destroyConsole(cons);
        return NULL;
    }
    cons->size = info.dwSize;

    setWindow(cons, 0, 0, cons->size.X, cons->size.Y);

    return cons;
}

void destroyConsole(Console* cons)
{
    free(cons);
}

void setWindow(Console* cons, int x, int y, int sX, int sY)
{
    cons->windowPos.X = x;
    cons->windowPos.Y = y;
    cons->windowSize.X = sX;
    cons->windowSize.Y = sY;
    cons->outToPos.X = 0;
    cons->outToPos.Y = 0;
}

void writeCharacter(Console* cons, int x, int y, CHAR_INFO c)
{
    if (x < 0 || y < 0 || x >= cons->windowSize.X || y >= cons->windowSize.Y)
        return;

    COORD topLeft = { 0, 0 };
    COORD bottomRight = { 1, 1 };
    SMALL_RECT region;
    region.Left = x + cons->windowPos.X;
    region.Top = y + cons->windowPos.Y;
    region.Right = x + cons->windowPos.X;
    region.Bottom = y + cons->windowPos.Y;
    WriteConsoleOutput(cons->hbuf, &c, bottomRight, topLeft, &region);
}

void writeString(Console* cons, const char* str, WORD attr)
{
    for (const char* p = str; *p; ++p)
    {
        CHAR_INFO c;
        c.Char.AsciiChar = *p;
        c.Attributes = attr;
        writeCharacter(cons, cons->outToPos.X, cons->outToPos.Y, c);
        cons->outToPos.X++;
        if (cons->outToPos.X >= cons->windowSize.X)
        {
            cons->outToPos.X = 0;
            cons->outToPos.Y++;
            if (cons->outToPos.Y >= cons->windowSize.Y)
            {
                scrollWindowUp(cons);
                cons->outToPos.Y--;
            }
        }
    }
}

void writeNewLine(Console* cons)
{
    unsigned int size = cons->windowSize.X - cons->outToPos.X;
    char* buf = calloc(sizeof(char), size + 1);
    for (int i = 0; i < size; i++)
        buf[i] = ' ';
    
    buf[size] = '\0';

    WORD attr = 0;
    writeString(cons, buf, attr);
    free(buf);
}

void scrollWindowUp(Console* cons)
{
    CHAR_INFO* buf = calloc(sizeof(CHAR_INFO), cons->windowSize.X * cons->windowSize.Y);
    COORD topLeft = { 0, 0 };
    COORD bottomRight = { cons->windowSize.X, cons->windowSize.Y };
    SMALL_RECT region;
    region.Left = cons->windowPos.X;
    region.Top = cons->windowPos.Y + 1;
    region.Right = cons->windowPos.X + cons->windowSize.X - 1;
    region.Bottom = cons->windowPos.Y + cons->windowSize.Y - 1;
    ReadConsoleOutput(cons->hbuf, buf, bottomRight, topLeft, &region);

    region.Top--;
    WriteConsoleOutput(cons->hbuf, buf, bottomRight, topLeft, &region);

    for (int i = 0; i < cons->windowSize.X; i++)
    {
        CHAR_INFO c;
        c.Attributes = 0;
        c.Char.AsciiChar = ' ';
        writeCharacter(cons, i, cons->windowSize.Y - 1, c);
    }
    free(buf);
}

int main()
{
    Console* cons = initConsole();
    setWindow(cons, 25, 5, 55 - 25 + 1, 15 - 5 + 1);
    // setWindow(cons, 25, 5, 20, 15 - 5 + 1);
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
        {
            WORD attr = i * 16 + j;
            writeString(cons, colors[j], attr);
            writeString(cons, " on ", attr);
            writeString(cons, colors[i], attr);
            writeNewLine(cons);
            writeNewLine(cons);

            Sleep(600);
        }

    // writeString(cons, "qwertyuiopasdfghjkl", BACKGROUND_BLUE | FOREGROUND_RED);
    // Sleep(3000);
    // scrollWindowUp(cons);

    Sleep(60000);
}