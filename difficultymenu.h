#ifndef DIFFICULTYMENU_H
#define DIFFICULTYMENU_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QMediaPlayer>

class Song;

class DifficultyMenu:public QGraphicsScene {
  Q_OBJECT
  public:
    explicit DifficultyMenu(QGraphicsView* view, Song* song, QObject* parent=nullptr);
    ~DifficultyMenu();
  private:
    QMediaPlayer* btnSound;
    QGraphicsView* view;
    int boutonActuel;
    QGraphicsTextItem* title;
    QGraphicsTextItem* boutons[5];
    Song* song;
    void nextElement();
    void prevElement();
    void select();
    void draw();
  protected:
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
  };

#endif // DIFFICULTYMENU_H
