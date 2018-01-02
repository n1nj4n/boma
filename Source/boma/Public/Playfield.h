// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#define tileSize (100.f)

#include "Playfield.generated.h"



UCLASS()
class BOMA_API APlayfield : public APawn
{
	GENERATED_BODY()
	static const uint32 numPlayers=4;
public:
	// for debugging purposes
	// disables walls.
	// players are moved closer to each other.
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=Debug)
	uint8	NoLevelBuilding:1;

	// our root component
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class USceneComponent* Root;

	// game camera
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UCameraComponent* Camera;

	// walls that are unbreakable
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UInstancedStaticMeshComponent* Unbreakable;

	// walls that are breakable
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UInstancedStaticMeshComponent* Breakable;

	// X size of playfield in number of tiles
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=GameSetup)
	int	XSize;

	// Y size of playfield in number of tiles
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=GameSetup)
	int	YSize;

	// percent soft walls spawned
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=GameSetup)
	float	SoftWallDistribution;

	// initial game time
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=GameSetup)
	float	GameTime;

	// speed in uu/s
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=GameSetup)
	float	CameraSpeed;

	// template player 1
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=GameSetupPlayer)
	class UClass*	TemplatePlayer1;

	// template player 2
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=GameSetupPlayer)
	class UClass*	TemplatePlayer2;

	// template player 3
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=GameSetupPlayer)
	class UClass*	TemplatePlayer3;

	// template player 4
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=GameSetupPlayer)
	class UClass*	TemplatePlayer4;

	// a percentage chance a random pickup will spawn when destoying a breakable wall.
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=GameSetupPickups)
	float	PickupSpawnPercetage;

	// pickups classes ..
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=GameSetupPickups)
	TArray<class UClass*>	Pickups;

	// Sets default values for this pawn's properties
	APlayfield();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category=Action)
	virtual void SpawnPickup(const FVector& pos);

	// global spawn
	UFUNCTION(BlueprintCallable, Category=Action)
	virtual void SpawnTemplate(class UClass* tempClass, const FVector& pos, AActor* SpawnOwner=0);

	// check if player alive
	UFUNCTION(BlueprintCallable, Category=Info)
	bool	IsPlayerAlive(int32 player);

	// get maximum of bombs
	UFUNCTION(BlueprintCallable, Category=Info)
	int32	GetPlayerBombs(int32 player);

	// get available bombs
	UFUNCTION(BlueprintCallable, Category=Info)
	int32	GetPlayerAvailableBombs(int32 player);

	// get time left of current round
	UFUNCTION(BlueprintCallable, Category=Info)
	FString	GetGameTime();

	// is game over state. (not playing)
	UFUNCTION(BlueprintCallable, Category=Info)
	bool	IsGameOver();

	// last winner
	UFUNCTION(BlueprintCallable,Category=Info)
	FString	GetLastWinner();

	// check if we hit an unbreakable wall at a location (world coords)
	UFUNCTION(BlueprintCallable, Category=Info)
	bool	CheckUnbreakable(const FVector& location);

	// get idx of breakable wall at a location
	// negative return if no breakable
	UFUNCTION(BlueprintCallable, Category=Info)
	int32	CheckBreakable(const FVector& location);

	// break wall at a location
	UFUNCTION(BlueprintCallable, Category=Action)
	bool	BreakBreakable(const FVector& location);

	// check any breakable walls around an actor.
	UFUNCTION(BlueprintCallable, Category=Info)
	bool AnyBreakableAround(AActor* obj);

	// get closest actor of a type within radius
	UFUNCTION(BlueprintCallable, Category=Info)
	AActor* ClosestOfClass(AActor* obj, UClass* type, float radius);

	// get array of sorted actors of specific classes within radius.
	UFUNCTION(BlueprintCallable, Category=Info)
	TArray<AActor*> GetSortedOfClasses(AActor* obj,const TArray<UClass*> &types, float radius);

	// get distance of closest actor of type
	UFUNCTION(BlueprintCallable, Category=Info)
	float DistanceOfClosestOfClass(AActor* obj,UClass* type, float radius);

	// get valid directions for an actor
	// return values are sorted.
	// front, left, right, back .. 
	UFUNCTION(BlueprintCallable, Category=Info)
	TArray<FVector> ValidStepDirections(AActor* actor);

	// Tracking how many bombs are active
	// When game-time is up, all bombs need to detonate to get the final score.
	void	AddBomb(){placedBombs++;}
	void	SubBomb(){placedBombs--;}

	// disable player from current game.
	void	DisablePlayer(class ABMPlayer* player);

	// any wall at location
	bool	AnyWall(const FVector& location);

private:
	// human 1 control
	void	Fire1();
	void	Up1(float amount);
	void	Right1(float amount);

	// human 2 control
	void	Fire2();
	void	Up2(float amount);
	void	Right2(float amount);

	// remove all players
	void	RemovePlayers();
	// spawn new players
	void	SpawnPlayers();
	// creates the game area
	void	CreateMap();

	// calculate winnner
	void	CalcWinner();
	// initialize a new game
	void	InitializeGame();

	// camera update .. try to fit players into screen
	void	CameraUpdate(float DeltaTime);

	// internal spawner
	class ABMPlayer*	SpawnPlayer(class UClass* tempclass, const FTransform &transform);

	// internal player definition
	struct sPlayer
	{
		sPlayer():player(nullptr),lifeTime(0){}
		class ABMPlayer* player;
		// used in the end to figure out who stayed alive longest at en screen
		float lifeTime;
	};

	// internal player handling
	sPlayer	players[numPlayers];

	TArray<class ABMPlayer*> spawned;

	// seconds left of round
	float gameTimer;

	// last round winner
	FString	lastWinner;

	// game over state
	uint32	gameOver:1;

	// actual tile count of current play field.
	int32	xS;
	int32	yS;

	// active bombs on playfield
	uint32 placedBombs;
};
