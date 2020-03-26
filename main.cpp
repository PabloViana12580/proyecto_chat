#include "mainwindow.h"
#include <QApplication>
#include <QStackedWidget>
#include "connectview.h"
#include "chatview.h"
#include <messagegenerator.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    User* selfUser = new User(0, "", 0);
    MessageGenerator* messageGenerator = new MessageGenerator();

    ClientData* data = new ClientData();
    data = new ClientData();
    data->addUser(User(666, "General", 0));

    QStackedWidget* windowStack = w.centralWidget()->findChild<QStackedWidget*>("stackedWidget");

    UIClient* client = new UIClient(windowStack, selfUser, messageGenerator, data);
    ConnectView* connectView = new ConnectView(nullptr, windowStack, client, selfUser, messageGenerator);
    ChatView* chatView = new ChatView(nullptr, windowStack, client, selfUser, messageGenerator, data);

    // Por alguna razon se empieza con 2 paginas, asi que se eliminan
    windowStack->removeWidget(windowStack->findChild<QWidget*>("page"));
    windowStack->removeWidget(windowStack->findChild<QWidget*>("page_2"));

    windowStack->addWidget(connectView);
    windowStack->addWidget(chatView);
    windowStack->setCurrentIndex(0);

    w.show();

    return a.exec();
}
