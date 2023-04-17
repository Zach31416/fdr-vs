#ifndef INFOMENU_H
#define INFOMENU_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QMediaPlayer>

class InfoMenu:public QGraphicsScene {
  Q_OBJECT
  public:
    explicit InfoMenu(QGraphicsView* view, QObject* parent=nullptr);
    ~InfoMenu();
  private:
    QGraphicsView* view;
    QGraphicsTextItem* title;
    QGraphicsTextItem* paragraph;
    QGraphicsTextItem* backButton;
    void select();
    QMediaPlayer* btnSound;
  protected:
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
  };

#endif // INFOMENU_H
