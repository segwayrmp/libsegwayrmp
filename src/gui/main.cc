#include <QtGui/QApplication>
#include "segwayrmp/gui/segwayrmp_gui.h"

#include <SDL/SDL.h>

// SDL defines main?
#ifdef main
#undef main
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
