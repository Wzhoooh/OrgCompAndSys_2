#include "stdio.h"
#include "windows.h"
#include "winuser.h"
#include "synchapi.h"

#define BACKGROUND_BLACK BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE 

typedef struct
{
    HANDLE hbuf;
    COORD size;
    CHAR_INFO* buf;
    CHAR_INFO* changesBuf;
} Console;


// static HANDLE hbuf;
// static COORD screenSize;
// static CHAR_INFO* screenBuffer;

Console* initConsole(COORD size);
void destroyConsole();
BOOL isCorrectPos(Console* cons, short x, short y);
CHAR_INFO* getCellFromBuf(Console* cons, short x, short y);
CHAR_INFO* getCellFromChanges(Console* cons, short x, short y);
void setCharToBuf(Console* cons, int x, int y, CHAR_INFO ch);
void setCharToChanges(Console* cons, int x, int y, CHAR_INFO ch);
void writeConsole(Console* cons);

Console* initConsole(COORD size)
{
    Console* cons = calloc(sizeof(Console), 1);
    cons->size.X = size.X;
    cons->size.Y = size.Y;
    cons->buf = calloc(sizeof(CHAR_INFO), cons->size.X * cons->size.Y);
    cons->changesBuf = calloc(sizeof(CHAR_INFO), cons->size.X * cons->size.Y);
    
    AllocConsole();

    cons->hbuf = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    if (cons->hbuf == INVALID_HANDLE_VALUE)
    {
        destroyConsole(cons);
        return NULL;
    }

    if (!SetConsoleActiveScreenBuffer(cons->hbuf))
    {
        destroyConsole(cons);
        return NULL;
    }

    // CONSOLE_CURSOR_INFO cInfo = { 1, FALSE };
    // if (!SetConsoleCursorInfo(cons->hbuf, &cInfo))
    // {
    //     destroyConsole(cons);
    //     return NULL;
    // }
    if (!MoveWindow(GetConsoleWindow(), 0, 0, 0, 0, TRUE))
    {
        destroyConsole(cons);
        return NULL;
    }

    if (!SetConsoleScreenBufferSize(cons->hbuf, cons->size))
    {
        destroyConsole(cons);
        return NULL;
    }

    SMALL_RECT pos;
    pos.Left = 0;
    pos.Top = 0;
    pos.Right = cons->size.X - 1;
    pos.Bottom = cons->size.Y - 1;
    if (!SetConsoleWindowInfo(cons->hbuf, TRUE, &pos))
    {
        destroyConsole(cons);
        return NULL;
    }

    CHAR_INFO nullCh;
    nullCh.Attributes = BACKGROUND_BLACK;
    nullCh.Char.AsciiChar = 0;
    for (int i = 0; i < cons->size.Y; i++)
        for (int j = 0; j < cons->size.X; j++)
            setCharToChanges(cons, j, i, nullCh);

    writeConsole(cons);
    return cons;
}

void destroyConsole(Console* cons)
{
    free(cons->buf);
    free(cons->changesBuf);
    free(cons);
}

BOOL isCorrectPos(Console* cons, short x, short y)
{
    return x >= 0 && x < cons->size.X && y >= 0 && y < cons->size.Y;
}

CHAR_INFO* getCellFromBuf(Console* cons, short x, short y)
{
    if (isCorrectPos(cons, x, y))
        return cons->buf + (cons->size.X * y + x);

    return NULL;
}

CHAR_INFO* getCellFromChanges(Console* cons, short x, short y)
{
    if (isCorrectPos(cons, x, y))
        return cons->changesBuf + (cons->size.X * y + x);

    return NULL;
}

void setCharToBuf(Console* cons, int x, int y, CHAR_INFO ch)
{
    cons->buf[x + y * cons->size.X] = ch;
}

void setCharToChanges(Console* cons, int x, int y, CHAR_INFO ch)
{
    cons->changesBuf[x + y * cons->size.X] = ch;
}

void writeConsole(Console* cons)
{
    for (int i = 0; i < cons->size.Y; i++)
        for (int j = 0; j < cons->size.X; j++)
        {
            CHAR_INFO cell = *getCellFromChanges(cons, j, i);
            if (cell.Char.AsciiChar != cons->buf[j + i*cons->size.X].Char.AsciiChar)
            {
                int index = i * cons->size.X + j;
                COORD topLeft = { 0, 0 };
                COORD bottomRight = { 1, 1 };
                SMALL_RECT region;
                region.Left = j;
                region.Top = i;
                region.Right = j;
                region.Bottom = i;
                WriteConsoleOutput(cons->hbuf, cons->changesBuf+index, bottomRight,
                                   topLeft, &region);
                cons->buf[j + i * cons->size.X] = cell;
            }
        }
}

int main()
{
    COORD coords = {10, 10};
    Console* cons = initConsole(coords);
    if (cons == NULL)
        printf("null, %d", GetLastError());
    else
        printf("ok");

    Sleep(6000);
}