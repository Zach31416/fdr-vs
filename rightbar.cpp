#include "rightbar.h"

#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QFont>
#include <QDebug>

#include "ui.h"
#include "common.h"
#include "gamescene.h"

RightBar::RightBar(GameScene* scene) {
    this->scene = scene;
    qDebug() << "THIS IS THE POSITION OF THE LAST FRET:";
    qDebug() << scene->getFret(4)->scenePos();
    score = new QGraphicsTextItem("Score: 0");
    score->setDefaultTextColor(TEXT_COLOR_MAIN);
    score->setFont(QFont("Arial", 32));
    streak = new QGraphicsTextItem("Notes de suite: 0");
    streak->setDefaultTextColor(TEXT_COLOR_MAIN);
    streak->setFont(QFont("Arial", 32));
    multiplier = new QGraphicsTextItem("x1");
    multiplier->setDefaultTextColor(TEXT_COLOR_MAIN);
    multiplier->setFont(QFont("Arial", 32));

    backgroundRect = new QGraphicsRectItem();
    backgroundRect->setBrush(TEXT_COLOR_MAIN);
    filledRect = new QGraphicsRectItem();
    filledRect->setBrush(QColor(139, 0, 0, 255)); //Dark red
    //backgroundRect->setBrush(QColor(255, 255, 255, 255));
    place();
}

RightBar::~RightBar() {}

void RightBar::setScore(const int score) {
    this->score->setPlainText("Score: " + QString::number(score));
}

void RightBar::setStreak(const int streak) {
    this->streak->setPlainText("Notes de suite: " + QString::number(streak));
}

void RightBar::setFilledRect(int ledstate) {
    const int xPos = scene->getFret(4)->scenePos().x() +
        scene->getFret(4)->rect().width() +
        TEXT_SIDE_PADDING;
    filledRect->setRect(xPos, multiplier->pos().y() + multiplier->boundingRect().height() + TEXT_MARGIN_Y, 200, (BARGRAPH_HEIGHT / 10) * ledstate);
}

void RightBar::recolorFullBargraph() {
    filledRect->setBrush(QColor(0, 100, 0, 255)); //Dark Green
}

void RightBar::recolorActivePowerup() {
    filledRect->setBrush(QColor(0, 0, 205, 255)); //Dark Blue
}

void RightBar::resetBargraph() {
    filledRect->setBrush(QColor(139, 0, 0, 255)); //Dark Red
    bargraphState = 0;
    setFilledRect(0);
}

void RightBar::setMultiplier(const int currentMultiplier, bool isPowerup, bool isDivineIntervention) {
    QString multiplierDesc = "";
    if (isPowerup && isDivineIntervention) {
        multiplierDesc = "POWERUP ET INTERVENTION DIVINE !!!";
    }
    else if (isPowerup)
    {
        multiplierDesc = "POWERUP !!!";
    }
    else if (isDivineIntervention)
    {
        multiplierDesc = "INTERVENTION DIVINE !!!";
    }
    this->multiplier->setPlainText("x" + QString::number(currentMultiplier) + " : " + multiplierDesc);
}

void RightBar::place() {
    const int xPos = scene->getFret(4)->scenePos().x() +
        scene->getFret(4)->rect().width() +
        TEXT_SIDE_PADDING;
    score->setPos(xPos, TEXT_TOP_PADDING);
    scene->addItem(score);

    streak->setPos(xPos, score->pos().y() + score->boundingRect().height() + TEXT_MARGIN_Y);
    scene->addItem(streak);

    multiplier->setPos(xPos, streak->pos().y() +
        streak->boundingRect().height() + TEXT_MARGIN_Y);
    scene->addItem(multiplier);

    backgroundRect->setRect(xPos, multiplier->pos().y() + multiplier->boundingRect().height() + TEXT_MARGIN_Y, 200, BARGRAPH_HEIGHT);
    scene->addItem(backgroundRect);
    filledRect->setRect(xPos, multiplier->pos().y() + multiplier->boundingRect().height() + TEXT_MARGIN_Y, 200, 1);
    scene->addItem(filledRect);
}
