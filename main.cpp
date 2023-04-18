#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGuiApplication>
#include <QPainter>
#include <QThread>
#include <QUrl>

#include "qscreen.h"
#include "mainmenu.h"
// #include "remote.h"
#include "gamescene.h"


int main(int argc, char *argv[]) {
  // Create the application.
  QApplication a(argc, argv);
  // Get a list of the connected screens
  QList<QScreen*> screens = QGuiApplication::screens();
  // Create a view to put the scene inside
  QGraphicsView* view = new QGraphicsView();
  // Disable scrollbars
  view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  // Lock its dimensions (using the first screen from the list)
  view->setFixedSize(screens.first()->availableSize());
  // Make the game fullscreen
  view->showFullScreen();
  // Enable antialiasing
  view->setRenderHint(QPainter::Antialiasing);
  // Mount a remote to this view
  // Remote* remote = new Remote(view);
  // Create the main menu (where the game starts)
  QGraphicsScene* mainMenu = new MainMenu(view);
  view->setScene(mainMenu);
  // Display the viewport
  view->show();
  return a.exec();
}
