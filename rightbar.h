#ifndef RIGHTBAR_H
#define RIGHTBAR_H

#include <QGraphicsTextItem>

#define BARGRAPH_HEIGHT 310
#define BARGRAPH_WIDTH 200
#define LED_HEIGHT 20
#define LED_WIDTH 180


class GameScene;

class RightBar {
private:
    QGraphicsTextItem* score;
    QGraphicsTextItem* streak;
    QGraphicsTextItem* multiplier;
    QGraphicsRectItem* backgroundRect;
    //QGraphicsRectItem* filledRect;

    QGraphicsRectItem* leds[10];

    GameScene* scene;
public:
    RightBar(GameScene* scene);
    ~RightBar();
    void setScore(const int score);
    void setStreak(const int streak);
    void setMultiplier(const int currentMultiplier, bool isPowerup, bool isDivineIntervention);
    //void setFilledRect(int ledstate);
    void fillRects(int ledState);
    void recolorFullBargraph();
    void recolorActivePowerup();
    void resetBargraph();
    void place();
};

#endif // RIGHTBAR_H
