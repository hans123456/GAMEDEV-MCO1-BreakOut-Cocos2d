#ifndef __BREAKOUT_SCENE_H__
#define __BREAKOUT_SCENE_H__

#include "cocos2d.h"
#include "Direction.h"
#include "PseudoRandomDistribution.h"
#include "SimpleAudioEngine.h"

class BreakOut : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC(BreakOut);
	
private:
	
	cocos2d::Size windowSize;
	cocos2d::Size visibleSize;
	cocos2d::Vec2 origin;

	cocos2d::PhysicsWorld *physicsWorld;
	void setPhysicsWorld (cocos2d::PhysicsWorld *physicsWorld) { this->physicsWorld = physicsWorld; };
	
	int BRICK_BITMASK, BOUNDARY_BITMASK, BALL_BITMASK, PADDLE_BITMASK, POWER_UP_BITMASK;

	int gameInfoPaddingY;
	int velocity;
	int MAX_VELOCITY, MIN_VELOCITY, VELOCITY_INC;
	int MAX_BOUNDS_X, MIN_BOUNDS_X;
	int prevPosX, prevPosY;

	float DEFAULT_FIRING_ANGLE, MIN_FIRING_ANGLE, MAX_FIRING_ANGLE;
	float firingAngle;
	
	bool mainMenu;
	cocos2d::Label *gameTitleLabel;
	cocos2d::Label *clickToPlayGameLabel;

	CocosDenshion::SimpleAudioEngine *audio;
	std::string pewSoundEffectFileName;

	std::string brickSpriteFileName;
	std::string ballSpriteFileName;
	std::string paddleSpriteFileName;
	std::string firingAngleSpriteFileName;
	std::string bgSpriteFileName;
	std::string powerUpSpriteFileName;

	cocos2d::Sprite *firingAngleSprite;
	cocos2d::Sprite *ballSprite;
	cocos2d::Sprite *paddleSprite;
	cocos2d::Vec2 mousePosition;
	
	bool hasTriggeredNoticeFire;
	cocos2d::Label noticeFireLabel;

	int noOfLives;
	cocos2d::Label *noOfLivesLabel;
	
	cocos2d::Label *theTipLabel;
	cocos2d::Label *tipLabel;

	int noOfPowerUp;
	bool hasTriggeredPowerUp;
	cocos2d::Label *noOfPowerUpLabel;
	std::vector<cocos2d::Sprite*> listOfPowerUpSprites;

	float accumulatedProbability;
	float PROBABILITY;
	float INITIAL_PROBABILITY, PROBABILITY_INCREASE;

	int noOfBricks;
	cocos2d::Label *hasWonLabel;
	cocos2d::Label *hasLostLabel;
	cocos2d::Label *leftClickToPlayAgainLabel;

	bool hasInitiallyFired;
	bool hasFired, hasDied, hasWon;
	void fireBall();
	
	std::map<cocos2d::PhysicsBody*, float> brickMapPhysicsBodyToSpritePosX;
	std::map<cocos2d::PhysicsBody*, float> brickMapPhysicsBodyToSpritePosY;

	void update(float) override;
	void updatePaddlePosition(float);
	void updateFiringAngle(float);
	void updateBallPosition(float);
	float getAngleOfTwoVectors(cocos2d::Point, cocos2d::Point);

	bool rightMouseButtonDown;
	void onMouseDown(cocos2d::Event *);
	void onMouseUp(cocos2d::Event *);
	void onMouseMove(cocos2d::Event *);
	
	void onContactSeparate(cocos2d::PhysicsContact &);
	bool onContactBegin(cocos2d::PhysicsContact &);

	void createPowerUp(float, float);
	void reset();

};

#endif // __BREAKOUT_SCENE_H__