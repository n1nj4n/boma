// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BMPlayer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRemoteTrigg);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLastMoveDone);
UCLASS()
class BOMA_API ABMPlayer : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USphereComponent*	Collision;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent*	Mesh;

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

	void	SetMain(class APlayfield* main){MainPawn=main;}


	// Sets default values for this pawn's properties
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
	UFUNCTION(BlueprintCallable)
	void Fire();

	// move left
	UFUNCTION(BlueprintCallable)
	void Up(float amount);

	// move right
	UFUNCTION(BlueprintCallable)
	void Right(float amount);

	UFUNCTION(BlueprintCallable)
	class APlayfield* GetPlayfield();

	// kill player, removes input
	UFUNCTION(BlueprintCallable)
	void KillYourself();

	// return used bomb to player
	UFUNCTION(BlueprintCallable)
	void AddAvailableBomb();

	// enable remote controlled bombs for a duration of time
	UFUNCTION(BlueprintCallable)
	void EnableRemote(float duration);

	// check if remote controlled mode
	UFUNCTION(BlueprintCallable)
	float GetRemoteTimer();

	UPROPERTY(BlueprintAssignable)
	FRemoteTrigg OnRemoteTrigg;

	UFUNCTION(BlueprintCallable)
	bool IsAlive();

	UPROPERTY(BlueprintAssignable)
	FRemoteTrigg OnLastMoveDone;

	UFUNCTION(BlueprintCallable)
	bool AnyDestructablesAround();

	UFUNCTION(BlueprintCallable)
	bool AnythingInfront();

	UFUNCTION(BlueprintCallable)
	TArray<FVector> ValidStepDirections();

	// steps towards pointing direction
	UFUNCTION(BlueprintCallable)
	void Step(FVector dir=FVector(0,0,0));

	// get closest actor of type
	UFUNCTION(BlueprintCallable)
	AActor* ClosestOfClass(UClass* type, float radius);

	// get distance of closest actor of type
	UFUNCTION(BlueprintCallable)
	float DistanceOfClosestOfClass(UClass* type, float radius);

	// move away from an actor
	UFUNCTION(BlueprintCallable)
	FVector MoveAwayFrom(AActor* object, const TArray<FVector>& directions);

	int32	GetAvailableBombs(){return Bombs-spawnedBombs;}
	void	kill(){alive=false;}
private:
	void	Move(float timeStep);
	float	up;
	float	right;
	uint8	bTriggBomb:1;
	uint8	alive:1;

	FRotator Rotation;

	class APlayfield* MainPawn;

	int32	spawnedBombs;

	FVector	moveFrom;
	FVector	moveTo;

	float remoteTimer;

};
