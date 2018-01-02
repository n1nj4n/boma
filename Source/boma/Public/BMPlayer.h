// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BMPlayer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRemoteTrigg);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMove);
UCLASS()
class BOMA_API ABMPlayer : public APawn
{
	GENERATED_BODY()

public:
	// collision mesh
	UPROPERTY(EditAnywhere,BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USphereComponent*	Collision;

	// rendered mesh
	UPROPERTY(EditAnywhere,BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent*	Mesh;

	// bomb template
	UPROPERTY(EditAnywhere,BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UClass*	BombTemplate;

	// initial bombs
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Setup")
	int32	Bombs;

	// initial bomb strength
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Setup")
	int32	BombStrength;

	// initial player speed scale
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Setup")
	float	SpeedFactor;

	// initial player speed/s in uu
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Setup")
	float	Speed;

	// sets the playfield
	void	SetMain(class APlayfield* pf){PlayField=pf;}


	ABMPlayer();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// release a bomb at current location
	UFUNCTION(BlueprintCallable,Category=Action)
	void Fire();

	// move up +1 , down -1
	UFUNCTION(BlueprintCallable,Category=Action)
	void Up(float amount);

	// move right +1, left -1
	UFUNCTION(BlueprintCallable,Category=Action)
	void Right(float amount);

	// disables player
	UFUNCTION(BlueprintCallable,Category=Action)
	void Disable();

	// add bomb to player arsenal
	UFUNCTION(BlueprintCallable,Category=Action)
	void AddAvailableBomb();

	// add time for remote controlled bombs.
	UFUNCTION(BlueprintCallable,Category=Action)
	void EnableRemote(float duration);

	// check if remote controlled mode
	UFUNCTION(BlueprintCallable,Category=Info)
	float GetRemoteTimer();

	// check if pawn has the possibility to be controlled
	UFUNCTION(BlueprintCallable,Category=Info)
	bool IsAlive();

	// event for a placed remote bomb.
	UPROPERTY(BlueprintAssignable,Category=Event)
	FRemoteTrigg OnRemoteTrigg;

	// event when move is initialized
	UPROPERTY(BlueprintAssignable,Category=Event)
	FOnMove OnMoveStart;

	// event when move is finished
	UPROPERTY(BlueprintAssignable,Category=Event)
	FOnMove OnMoveEnd;

	// check if anything in front of player
	UFUNCTION(BlueprintCallable,Category=Info)
	bool AnythingInfront();

	// steps towards dir
	// if dir vector has zero length, current direction is used to step pawn
	UFUNCTION(BlueprintCallable,Category=Action)
	void Step(FVector dir=FVector(0,0,0));

	// find best direction to move away from an actor using the directions array.
	UFUNCTION(BlueprintCallable,Category=Info)
	FVector MoveAwayFrom(AActor* object, const TArray<FVector>& directions);

	// Number of spawn-able bombs.
	int32	GetAvailableBombs(){return Bombs-spawnedBombs;}

	// get game play field
	UFUNCTION(BlueprintCallable,Category=Info)
	class APlayfield* GetPlayfield();

	// set myself as disabled
	void	disable(){bAlive=false;}
private:
	// move player amount of unreal units
	void	Move(float step);
	// current move.. distance left
	FVector	currentMove;

	// is still playing
	uint8	bAlive:1;

	// local rotation
	FRotator Rotation;

	// helper to reach play field
	class APlayfield* PlayField;

	// number of spawned bombs on play field
	int32	spawnedBombs;

	// spawn a bomb during a move
	FVector	moveStart;
	FVector	moveEnd;

	// time left to dropped remote controlled bomb.
	float remoteTimer;

};
