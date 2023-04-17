#ifndef MAINMENU_H
#define MAINMENU_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QMediaPlayer>


class MainMenu:public QGraphicsScene {
  Q_OBJECT
  public:
    explicit MainMenu(QGraphicsView* view, QObject* parent = nullptr);
    ~MainMenu();
  private:
    QGraphicsView* view;
    int boutonActuel;
    QGraphicsTextItem*  boutons[4];
    void nextElement();
    void prevElement();
    void select();
    QMediaPlayer* btnSound;
  protected:
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
};

#endif // MAINMENU_H
