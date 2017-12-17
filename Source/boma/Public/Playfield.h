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
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	uint8	NoLevelBuilding:1;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class USceneComponent* Root;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UCameraComponent* Camera;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UInstancedStaticMeshComponent* Unbreakable;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UInstancedStaticMeshComponent* Breakable;

	// 1 is 100 uu
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int	XSize;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int	YSize;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float	gameTime;



	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UClass*	TemplatePlayer1;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UClass*	TemplatePlayer2;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UClass*	TemplatePlayer3;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UClass*	TemplatePlayer4;

	// chance for pickup to spawn after wall destruction .. in percent
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float	PickupSpawnPercetage;

	// pickups classes
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
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

	UFUNCTION(BlueprintCallable)
	virtual void SpawnPickup(const FVector& pos);

	// global spawn
	UFUNCTION(BlueprintCallable)
	virtual void SpawnTemplate(class UClass* tempClass, const FVector& pos, AActor* SpawnOwner=0);

	// check if we hit an unbreakable wall
	UFUNCTION(BlueprintCallable)
	bool	CheckUnbreakable(const FVector& center);

	// get idx of breakable instance
	UFUNCTION(BlueprintCallable)
	int32	CheckBreakable(const FVector& center);

	// when destoying a wall piece there is a "PickupSpawnPercetage" chance a random pickup will spawn.
	UFUNCTION(BlueprintCallable)
	bool	BreakBreakable(const FVector& center);

	void	RemovePlayer(class ABMPlayer* player);

	UFUNCTION(BlueprintCallable)
	bool	IsPlayerAlive(int32 player);

	UFUNCTION(BlueprintCallable)
	int32	GetPlayerBombs(int32 player);

	UFUNCTION(BlueprintCallable)
	int32	GetPlayerAvailableBombs(int32 player);

	UFUNCTION(BlueprintCallable)
	FString	GetGameTime();

	UFUNCTION(BlueprintCallable)
	bool	IsGameOver();

	UFUNCTION(BlueprintCallable)
	FString	GetLastWinner();

private:
	void	Fire1();
	void	Up1(float amount);
	void	Right1(float amount);

	void	Fire2();
	void	Up2(float amount);
	void	Right2(float amount);

	void	KillPlayers();
	void	SpawnPlayers();
	void	CreateMap();

	void	CalcWinner();
	void	InitializeGame();

	class ABMPlayer*	SpawnPlayer(class UClass* tempclass, const FTransform &transform);

	class ABMPlayer* players[numPlayers];
	float playerTimers[numPlayers];
	TArray<class ABMPlayer*> spawned;

	// game clock .. seconds left
	float gameTimer;
	FString	lastWinner;

	uint32	gameOver:1;

	// actual size of playfield. 
	int32	xS;
	int32	yS;
};
