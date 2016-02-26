#include "BreakOutScene.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

Scene* BreakOut::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::createWithPhysics();
	//scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
	scene->getPhysicsWorld()->setGravity(Vec2(0, 0));
	scene->getPhysicsWorld()->setAutoStep(false);

	// 'layer' is an autorelease object
	auto layer = BreakOut::create();
	layer->setPhysicsWorld (scene->getPhysicsWorld());

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool BreakOut::init()
{
	if( !Layer::init() )
	{
		return false;
	}

	windowSize = Director::getInstance()->getWinSize();
	visibleSize = Director::getInstance()->getVisibleSize();
	origin = Director::getInstance()->getVisibleOrigin();

	gameInfoPaddingY = 100;

	{
		bgSpriteFileName = "sprites/bg2.jpg";	// http://eskipaper.com/images/starry-night-4.jpg
		ballSpriteFileName = "sprites/ball3.png";
		paddleSpriteFileName = "sprites/paddle2.png";
		firingAngleSpriteFileName = "sprites/firing-angle.png";
		powerUpSpriteFileName = "sprites/CrosshairPowerUp3.png";
		brickSpriteFileName = "sprites/brick4.png";

		pewSoundEffectFileName = "sound effects/pew.wav";
	}

	// "Constants"
	{
		BALL_BITMASK = 1;
		PADDLE_BITMASK = 2;
		BRICK_BITMASK = 3;
		POWER_UP_BITMASK = 4;
		BOUNDARY_BITMASK = 9;

		DEFAULT_FIRING_ANGLE = (float) CC_DEGREES_TO_RADIANS(90);
		MAX_VELOCITY = 600;
		MIN_VELOCITY = 400;
		VELOCITY_INC = 10;

		PseudoRandomDistribution prd;

		PROBABILITY = 0.12;
		INITIAL_PROBABILITY = prd.CfromP(PROBABILITY);
		PROBABILITY_INCREASE = INITIAL_PROBABILITY;
	}

	auto bgSprite = cocos2d::Sprite::create(bgSpriteFileName);
	bgSprite->setAnchorPoint(Vec2(0, 0));
	bgSprite->setScaleX(visibleSize.width / bgSprite->getContentSize().width);
	bgSprite->setScaleY(visibleSize.height / bgSprite->getContentSize().height);
	this->addChild(bgSprite, -1);

	gameTitleLabel = cocos2d::Label::create("Break Out", "fonts/arial.ttf", 72);
	gameTitleLabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 15);
	this->addChild(gameTitleLabel, 0);

	clickToPlayGameLabel = cocos2d::Label::create("Left Click To Play The Game.", "fonts/arial.ttf", 36);
	clickToPlayGameLabel->setPosition(visibleSize.width / 2, 
		gameTitleLabel->getPositionY() - gameTitleLabel->getContentSize().height / 2 - 20);
	this->addChild(clickToPlayGameLabel, 0);

	mainMenu = true;

	auto mouseListener = EventListenerMouse::create();
	mouseListener->onMouseDown = CC_CALLBACK_1(BreakOut::onMouseDown, this);
	_eventDispatcher->addEventListenerWithFixedPriority(mouseListener, 1);

}

void BreakOut::update(float dt)
{
	if (hasDied)
	{
		noOfLivesLabel->setString("No of Lives : " + std::to_string(noOfLives));
		if (noOfLives > 0)
		{
			hasInitiallyFired = false;
			ballSprite->setPosition(paddleSprite->getPositionX(), 
				paddleSprite->getPositionY() + paddleSprite->getContentSize().height / 2 + ballSprite->getContentSize().height / 2);
			ballSprite->getPhysicsBody()->setVelocity(Vec2(0,0));
			firingAngle = DEFAULT_FIRING_ANGLE;
			firingAngleSprite->setPosition(ballSprite->getPosition());
			firingAngleSprite->setRotation(-CC_RADIANS_TO_DEGREES(firingAngle));
			firingAngleSprite->setVisible(true);
			hasDied = false;
			noOfLives--;
		}
		else
		{
			hasLostLabel->setVisible(true);
			leftClickToPlayAgainLabel->setVisible(true);
		}
		return;
	}

	updatePaddlePosition(dt);
	updateFiringAngle(dt);
	updateBallPosition(dt);
}

void BreakOut::updateBallPosition(float dt)
{
	if (ballSprite->getPositionY() <= gameInfoPaddingY + ballSprite->getContentSize().height / 2)
	{
		hasDied = true;
		log("died");
	}

	prevPosX = ballSprite->getPositionX();
	prevPosY = ballSprite->getPositionY();
	physicsWorld->step(dt);

	bool hasExceededPossibleXPosition = ballSprite->getPositionX() > visibleSize.width || 
		ballSprite->getPositionX() <= ballSprite->getContentSize().width / 2;
	bool hasExceededPossibleYPosition = ballSprite->getPositionY() > visibleSize.height;

	if (hasExceededPossibleXPosition || 
		hasExceededPossibleYPosition)
	{
		ballSprite->setPosition(prevPosX, prevPosY);
	}
}

void BreakOut::updatePaddlePosition(float dt)
{
	if (rightMouseButtonDown || hasWon || hasTriggeredPowerUp)
	{
		return;
	}

	int paddlePosX = paddleSprite->getPositionX();
	int mousePosX = (int) mousePosition.x;
	int spd = 25;

	if (paddlePosX < mousePosX)
	{
		paddlePosX = MIN(MIN(paddlePosX + spd, MAX_BOUNDS_X), mousePosX);
	}
	else if (paddlePosX > mousePosX)
	{
		paddlePosX = MAX(MAX(paddlePosX - spd, MIN_BOUNDS_X), mousePosX);
	}

	paddleSprite->setPositionX(paddlePosX);

	if (!hasInitiallyFired)
	{
		ballSprite->setPositionX(paddlePosX);
		firingAngleSprite->setPositionX(ballSprite->getPositionX());
	}
}

void BreakOut::updateFiringAngle(float dt)
{
	if (!rightMouseButtonDown || (hasInitiallyFired && !hasTriggeredPowerUp))
	{
		return;
	}

	cocos2d::PhysicsBody *physicsBody = ballSprite->getPhysicsBody();
	physicsBody->setVelocity(Vec2(0, 0));

	int ballPosX = ballSprite->getPositionX();
	int ballPosY = ballSprite->getPositionY();
	int mousePosX = mousePosition.x;
	int mousePosY = visibleSize.height - mousePosition.y;	// mouse y is top left instead of bottom left

	int dx = mousePosX - ballPosX;
	int dy = mousePosY - ballPosY;
	firingAngle = atan2(dy, dx);

	firingAngleSprite->setPosition(ballSprite->getPosition());
	firingAngleSprite->setVisible(true);
	firingAngleSprite->setRotation((float) -CC_RADIANS_TO_DEGREES(firingAngle));
}

void BreakOut::onMouseMove(cocos2d::Event *event)
{
	cocos2d::EventMouse *eventMouse = (EventMouse*) event;
	mousePosition = eventMouse->getLocation();
}

void BreakOut::onMouseDown(cocos2d::Event *event)
{
	EventMouse *eventMouse = (EventMouse*) event;
	switch (eventMouse->getMouseButton())
	{
	case MOUSE_BUTTON_RIGHT:
		if (!mainMenu && hasInitiallyFired && noOfPowerUp > 0)
		{
			hasTriggeredPowerUp = true;
		}
		rightMouseButtonDown = true;
		break;
	case MOUSE_BUTTON_LEFT:
		if ((hasDied && noOfLives == 0) || hasWon || mainMenu)
		{
			mainMenu = false;
			reset();
			return;
		}
		fireBall();
		break;
	}
}

void BreakOut::onMouseUp(cocos2d::Event *event)
{
	EventMouse *eventMouse = (EventMouse*) event;
	switch (eventMouse->getMouseButton())
	{
	case MOUSE_BUTTON_RIGHT:
		rightMouseButtonDown = false;
		break;
	}
}

void BreakOut::onContactSeparate(cocos2d::PhysicsContact &contact)
{

	cocos2d::PhysicsBody *a = contact.getShapeA()->getBody();
	cocos2d::PhysicsBody *b = contact.getShapeB()->getBody();
	cocos2d::PhysicsBody *ball = NULL;

	if (hasInitiallyFired) 
	{
		if (a->getCollisionBitmask() == BALL_BITMASK)
		{
			ball = a;
		}
		else if (b->getCollisionBitmask() == BALL_BITMASK)
		{
			ball = b;
		}

		if (ball != NULL)
		{
			float prevVelocityX = ball->getVelocity().x;
			float prevVeolictyY = ball->getVelocity().y;
			float angle = atan2(prevVeolictyY, prevVelocityX);
			float velocityX = velocity * cos(angle);
			float velocityY = velocity * sin(angle);
			ball->setVelocity(Vec2(velocityX, velocityY));
			velocity = MIN(velocity + VELOCITY_INC, MAX_VELOCITY);
		}
	}

	//audio->playEffect(pewSoundEffectFileName.c_str());

}

bool BreakOut::onContactBegin(cocos2d::PhysicsContact &contact)
{

	cocos2d::PhysicsBody *a = contact.getShapeA()->getBody();
	cocos2d::PhysicsBody *b = contact.getShapeB()->getBody();
	int aBM = a->getCollisionBitmask();
	int bBM = b->getCollisionBitmask();
	cocos2d::PhysicsBody *brick = NULL;

	// ball to brick
	{

		if (aBM == BALL_BITMASK && bBM == BRICK_BITMASK)
		{
			brick = b;
		}
		else if (bBM == BALL_BITMASK && aBM == BRICK_BITMASK)
		{
			brick = a;
		}

		if (brick != NULL && brick->getNode() != NULL)
		{
			if (((double) rand() / (RAND_MAX)) / 2 < accumulatedProbability)
			{
				accumulatedProbability = INITIAL_PROBABILITY;
				float brickSpritePosX = brick->getOwner()->getPositionX();
				float brickSpritePosY = brick->getOwner()->getPositionY();
				createPowerUp(brickSpritePosX, brickSpritePosY);
			}
			else
			{
				accumulatedProbability += PROBABILITY_INCREASE;
			}
			brick->getNode()->removeFromParentAndCleanup(true);
			noOfBricks--;
			if (noOfBricks == 0)
			{
				hasWon = true;
				paddleSprite->setPositionX(visibleSize.width / 2);
				paddleSprite->setScaleX(50.0);
				hasWonLabel->setVisible(true);
				leftClickToPlayAgainLabel->setVisible(true);
			}
		}

	}

	// no collision with power up
	{
		if (aBM == POWER_UP_BITMASK)
		{
			if (a->getNode() != NULL && 
				bBM == PADDLE_BITMASK)
			{
				a->getNode()->removeFromParentAndCleanup(true);
				noOfPowerUp++;
				noOfPowerUpLabel->setString("No of Power Up : " + std::to_string(noOfPowerUp));
			}
			return false;
		}
		else if (bBM == POWER_UP_BITMASK)
		{
			if (b->getNode() != NULL && 
				aBM == PADDLE_BITMASK)
			{
				b->getNode()->removeFromParentAndCleanup(true);
				noOfPowerUp++;
				noOfPowerUpLabel->setString("No of Power Up : " + std::to_string(noOfPowerUp));
			}
			return false;
		}
	}

	return true;

}

void BreakOut::createPowerUp(float x, float y)
{
	auto powerUpSprite = cocos2d::Sprite::create(powerUpSpriteFileName);
	auto physicsBody = PhysicsBody::createBox(powerUpSprite->getContentSize(), PhysicsMaterial(1.0f, 1.0f, 0.0f));
	physicsBody->setCollisionBitmask(POWER_UP_BITMASK);
	physicsBody->setContactTestBitmask(true);
	physicsBody->setDynamic(true);
	powerUpSprite->setPhysicsBody(physicsBody);
	powerUpSprite->setAnchorPoint(Vec2(0.5,0.5));
	physicsBody->setVelocity(Vec2(0, -200));
	powerUpSprite->setPosition(x, y);
	listOfPowerUpSprites.push_back(powerUpSprite);
	this->addChild(powerUpSprite, 0);
	physicsWorld->step(0);
}

void BreakOut::fireBall()
{
	if ((hasInitiallyFired && !hasTriggeredPowerUp) || 
		rightMouseButtonDown)
	{
		return;
	}

	if (hasTriggeredPowerUp)
	{
		noOfPowerUp--;
		noOfPowerUpLabel->setString("No of Power Up : " + std::to_string(noOfPowerUp));
	}

	if (!hasInitiallyFired)
	{
		tipLabel->setString("Left Click To Use Power Up.");
	}

	hasInitiallyFired = true;
	hasTriggeredPowerUp = false;
	firingAngleSprite->setVisible(false);
	int velocity_x = velocity * cos(firingAngle);
	int velocity_y = velocity * sin(firingAngle);
	auto physicsBody = ballSprite->getPhysicsBody();
	physicsBody->setVelocity(Vec2(velocity_x, velocity_y));
	//audio->playEffect(pewSoundEffectFileName.c_str());

}

void BreakOut::reset()
{
	this->removeAllChildrenWithCleanup(true);
	this->unscheduleUpdate();
	_eventDispatcher->removeAllEventListeners();

	audio = CocosDenshion::SimpleAudioEngine::getInstance();
	audio->preloadEffect(pewSoundEffectFileName.c_str());

	auto bgSprite = cocos2d::Sprite::create(bgSpriteFileName);
	bgSprite->setAnchorPoint(Vec2(0, 0));
	bgSprite->setScaleX(visibleSize.width / bgSprite->getContentSize().width);
	bgSprite->setScaleY(visibleSize.height / bgSprite->getContentSize().height);
	bgSprite->setPositionY(gameInfoPaddingY);
	this->addChild(bgSprite, -1);

	{
		velocity = MIN_VELOCITY;
		firingAngle = DEFAULT_FIRING_ANGLE;
		accumulatedProbability = INITIAL_PROBABILITY;

		rightMouseButtonDown = false;
		hasFired = false;
		hasTriggeredNoticeFire = false;
		hasInitiallyFired = false;
		hasTriggeredPowerUp = false;
		hasDied = false;
		hasWon = false;

		noOfLives = 2;
		noOfPowerUp = 0;
	}

	// top left right boundary
	{ 
		auto edgeBodyLeft = PhysicsBody::createEdgeBox(Size(1, visibleSize.height), PhysicsMaterial(1.0f, 1.0f, 0.0f), 3);
		edgeBodyLeft->setCollisionBitmask(BOUNDARY_BITMASK);
		edgeBodyLeft->setContactTestBitmask(true);
		auto edgeNodeLeft = Node::create();
		edgeNodeLeft->setPosition(Vec2(origin.x, gameInfoPaddingY + visibleSize.height / 2));
		edgeNodeLeft->setPhysicsBody(edgeBodyLeft);
		this->addChild(edgeNodeLeft);

		auto edgeBodyRight = PhysicsBody::createEdgeBox(Size(1, visibleSize.height), PhysicsMaterial(1.0f, 1.0f, 0.0f), 3);
		edgeBodyRight->setCollisionBitmask(BOUNDARY_BITMASK);
		edgeBodyRight->setContactTestBitmask(true);
		auto edgeNodeRight = Node::create();
		edgeNodeRight->setPosition(Vec2(visibleSize.width, gameInfoPaddingY + visibleSize.height / 2));
		edgeNodeRight->setPhysicsBody(edgeBodyRight);
		this->addChild(edgeNodeRight);

		auto edgeBodyTop = PhysicsBody::createEdgeBox(Size(visibleSize.width, 1), PhysicsMaterial(1.0f, 1.0f, 0.0f), 3);
		edgeBodyTop->setCollisionBitmask(BOUNDARY_BITMASK);
		edgeBodyTop->setContactTestBitmask(true);
		auto edgeNodeTop = Node::create();
		edgeNodeTop->setPosition(Vec2(visibleSize.width / 2, visibleSize.height));
		edgeNodeTop->setPhysicsBody(edgeBodyTop);
		this->addChild(edgeNodeTop);
	}

	noOfLivesLabel = cocos2d::Label::create("No of Lives : 3", "fonts/arial.ttf", 24);
	noOfLivesLabel->setPosition(origin.x + 20, origin.y + 20);
	noOfLivesLabel->setAnchorPoint(Vec2(0, 0));
	this->addChild(noOfLivesLabel, 0);

	theTipLabel = cocos2d::Label::create("Tip:", "fonts/arial.ttf", 24);
	theTipLabel->setPosition(visibleSize.width / 2 - 50, gameInfoPaddingY - 10);
	theTipLabel->setAnchorPoint(Vec2(0, 1));
	this->addChild(theTipLabel, 0);

	tipLabel = cocos2d::Label::create("Right Click To Change Firing Angle.\nLeft Click To Fire Ball.", "fonts/arial.ttf", 24);
	tipLabel->setPosition(theTipLabel->getPositionX() + theTipLabel->getContentSize().width + 5, gameInfoPaddingY - 10);
	tipLabel->setAnchorPoint(Vec2(0, 1));
	this->addChild(tipLabel, 0);

	noOfPowerUpLabel = cocos2d::Label::create("No of Power Up : 0", "fonts/arial.ttf", 24);
	noOfPowerUpLabel->setPosition(origin.x + 20, 
		noOfLivesLabel->getPositionY() + noOfLivesLabel->getContentSize().height + 10);
	noOfPowerUpLabel->setAnchorPoint(Vec2(0, 0));
	this->addChild(noOfPowerUpLabel, 0);

	hasWonLabel = cocos2d::Label::create("You Won!", "fonts/arial.ttf", 64);
	hasWonLabel->setPosition(visibleSize.width / 2, gameInfoPaddingY + visibleSize.height / 2);
	hasWonLabel->setVisible(false);
	this->addChild(hasWonLabel, 9);

	hasLostLabel = cocos2d::Label::create("You Lost!", "fonts/arial.ttf", 64);
	hasLostLabel->setPosition(visibleSize.width / 2, gameInfoPaddingY + visibleSize.height / 2);
	hasLostLabel->setVisible(false);
	this->addChild(hasLostLabel, 9);

	leftClickToPlayAgainLabel = cocos2d::Label::create("Left Click To Play Again.", "fonts/arial.ttf", 48);
	leftClickToPlayAgainLabel->setPosition(visibleSize.width / 2, 
		hasLostLabel->getPositionY() - hasLostLabel->getContentSize().height / 2 - leftClickToPlayAgainLabel->getContentSize().height / 2 - 10);
	leftClickToPlayAgainLabel->setVisible(false);
	this->addChild(leftClickToPlayAgainLabel, 9);

	paddleSprite = Sprite::create(paddleSpriteFileName);
	paddleSprite->setPosition(visibleSize.width / 2, gameInfoPaddingY + origin.y + 100);
	auto paddleSpritePhysicsBody = PhysicsBody::createBox(paddleSprite->getContentSize(), 
		PhysicsMaterial(1.0f, 1.0f, 0.0f));
	paddleSpritePhysicsBody->setCollisionBitmask(PADDLE_BITMASK);
	paddleSpritePhysicsBody->setContactTestBitmask(true);
	paddleSprite->setPhysicsBody(paddleSpritePhysicsBody);
	paddleSpritePhysicsBody->setDynamic(false);
	this->addChild(paddleSprite);

	MIN_BOUNDS_X = origin.x + paddleSprite->getContentSize().width / 2;
	MAX_BOUNDS_X = visibleSize.width - paddleSprite->getContentSize().width / 2;

	ballSprite = Sprite::create(ballSpriteFileName);
	ballSprite->setPosition(paddleSprite->getPositionX(), 
		paddleSprite->getPositionY() + paddleSprite->getContentSize().height / 2 + ballSprite->getContentSize().height / 2);
	auto ballSpritePhysicsBody = PhysicsBody::createCircle(ballSprite->getContentSize().height / 2, 
		PhysicsMaterial(1.0f, 1.0f, 0.0f));
	ballSpritePhysicsBody->setRotationEnable(false);
	ballSpritePhysicsBody->setCollisionBitmask(BALL_BITMASK);
	ballSpritePhysicsBody->setContactTestBitmask(true);
	ballSpritePhysicsBody->setVelocityLimit(800);
	ballSprite->setPhysicsBody(ballSpritePhysicsBody);
	this->addChild(ballSprite);

	firingAngleSprite = Sprite::create(firingAngleSpriteFileName);
	firingAngleSprite->setAnchorPoint(Vec2(0, 0.5));
	firingAngleSprite->setPosition(ballSprite->getPositionX(), ballSprite->getPositionY());
	firingAngleSprite->setRotation(-CC_RADIANS_TO_DEGREES(firingAngle));
	this->addChild(firingAngleSprite, 5);

	auto mouseListener = EventListenerMouse::create();
	mouseListener->onMouseMove = CC_CALLBACK_1(BreakOut::onMouseMove, this);
	mouseListener->onMouseDown = CC_CALLBACK_1(BreakOut::onMouseDown, this);
	mouseListener->onMouseUp = CC_CALLBACK_1(BreakOut::onMouseUp, this);
	_eventDispatcher->addEventListenerWithFixedPriority(mouseListener, 1);

	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(BreakOut::onContactBegin, this);
	contactListener->onContactSeparate = CC_CALLBACK_1(BreakOut::onContactSeparate, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);

	cocos2d::Sprite *brickSprite = cocos2d::Sprite::create(brickSpriteFileName);	

	int width = brickSprite->getContentSize().width;
	int height = brickSprite->getContentSize().height;
	int startX = ((((int) (visibleSize.width) % (width) + width) - (floor((visibleSize.width / width) - 1))) / 2);

	noOfBricks = 0;
	for (int i=(visibleSize.height - (height * 3.5)), j=0, k=0, w_2=width/2, h_2=height/2; k<5; i-=height + 1, k++)
	{
		for (j=startX; j+width+1<visibleSize.width; j+=width + 1)
		{
			brickSprite->setAnchorPoint(Vec2(0.5, 0.5));
			brickSprite->setPosition(j+w_2, i-h_2);
			cocos2d::PhysicsBody *physicsBody = PhysicsBody::createEdgeBox(brickSprite->getContentSize(), PhysicsMaterial(1.0f, 1.0f, 0.0f), 3);
			physicsBody->setDynamic(false);
			physicsBody->setCollisionBitmask(BRICK_BITMASK);
			physicsBody->setContactTestBitmask(true);
			brickSprite->setPhysicsBody(physicsBody);
			brickMapPhysicsBodyToSpritePosX[physicsBody] = j;
			brickMapPhysicsBodyToSpritePosY[physicsBody] = i;
			this->addChild(brickSprite);
			brickSprite = cocos2d::Sprite::create(brickSpriteFileName);
			noOfBricks++;
		}
	}

	this->scheduleUpdateWithPriority(0);

}