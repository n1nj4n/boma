// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BMPlayer.generated.h"

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

	UFUNCTION(BlueprintCallable)
	virtual void Fire();
	UFUNCTION(BlueprintCallable)
	virtual void Up(float amount);
	UFUNCTION(BlueprintCallable)
	virtual void Right(float amount);
	UFUNCTION(BlueprintCallable)
	virtual class APlayfield* GetPlayfield();
	UFUNCTION(BlueprintCallable)
	virtual void KillYourself();

private:
	void	Move(float timeStep);
	float	up;
	float	right;
	uint8	bTriggBomb:1;

	FRotator Rotation;

	class APlayfield* MainPawn;

};
