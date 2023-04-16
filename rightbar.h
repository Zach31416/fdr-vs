#ifndef RIGHTBAR_H
#define RIGHTBAR_H

#include <QGraphicsTextItem>

#define BARGRAPH_HEIGHT 400

class GameScene;

class RightBar {
private:
    QGraphicsTextItem* score;
    QGraphicsTextItem* streak;
    QGraphicsRectItem* backgroundRect;
    QGraphicsRectItem* filledRect;
    GameScene* scene;
public:
    RightBar(GameScene* scene);
    ~RightBar();
    void setScore(const int score);
    void setStreak(const int streak);
    void setFilledRect(int ledstate);
    void recolorFullBargraph();
    void recolorActivePowerup();
    void resetBargraph();
    void place();
};

#endif // RIGHTBAR_H
